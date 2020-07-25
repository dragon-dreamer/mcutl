#pragma once

#include <array>
#include <limits>
#include <stddef.h>
#include <stdint.h>

#include "mcutl/clock/clock_defs.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::clock::detail
{

enum class prescaler_type
{
	multiplier,
	divider
};

enum class memoized_frequency : uint8_t
{
	unknown  = 0,
	accepted,
	rejected
};

constexpr size_t max_clock_sources = 30;
constexpr size_t max_clock_prescalers = 20;
constexpr size_t max_parent_sources = 10;
constexpr size_t max_child_sources = 10;

class frequency_description_base
{
public:
	explicit constexpr frequency_description_base(uint64_t frequency) noexcept
		: frequency(frequency)
	{
	}

	constexpr uint64_t get_frequency() const noexcept
	{
		return frequency;
	}

private:
	uint64_t frequency;
};

class min_frequency : public frequency_description_base { using frequency_description_base::frequency_description_base; };
class max_frequency : public frequency_description_base { using frequency_description_base::frequency_description_base; };
class exact_frequency : public frequency_description_base { using frequency_description_base::frequency_description_base; };

class prescaler
{
public:
	using prescaler_list_t = std::array<uint64_t, max_clock_prescalers>;

public:
	constexpr prescaler(prescaler_type type,
		const prescaler_list_t& prescaler_list,
		uint64_t prescaler_divider = 1) noexcept
		: type_(type)
		, prescaler_list_(prescaler_list)
		, prescaler_divider_(prescaler_divider)
	{
	}

	constexpr prescaler(prescaler_type type, uint64_t value) noexcept
		: type_(type)
		, prescaler_list_{ {value} }
		, prescaler_divider_(1)
	{
	}

	constexpr prescaler() noexcept
		: prescaler(prescaler_type::multiplier, 1)
	{
	}

	constexpr prescaler_type get_type() const noexcept
	{
		return type_;
	}

	constexpr const prescaler_list_t& get_prescaler_list() const noexcept
	{
		return prescaler_list_;
	}

	constexpr prescaler_list_t& get_prescaler_list() noexcept
	{
		return prescaler_list_;
	}

	constexpr uint64_t get_prescaler_divider() const noexcept
	{
		return prescaler_divider_;
	}

	constexpr uint64_t calc_child_frequency(uint64_t frequency, size_t prescaler_index) const noexcept
	{
		if (type_ == prescaler_type::multiplier)
			return (frequency / prescaler_divider_) * prescaler_list_[prescaler_index];

		return (frequency * prescaler_divider_) / prescaler_list_[prescaler_index];
	}

	constexpr size_t count() const noexcept
	{
		size_t i = 0;
		for (; i != prescaler_list_.size() && prescaler_list_[i]; ++i)
		{
		}
		return i;
	}

	constexpr bool is_valid() const noexcept
	{
		return !!prescaler_list_[0];
	}

private:
	prescaler_type type_;
	prescaler_list_t prescaler_list_;
	uint64_t prescaler_divider_;
};

constexpr prescaler multiplier(const prescaler::prescaler_list_t& prescaler_list,
	uint64_t prescaler_divider = 1) noexcept
{
	return prescaler(prescaler_type::multiplier, prescaler_list, prescaler_divider);
}

constexpr prescaler multiplier(uint64_t value) noexcept
{
	return prescaler(prescaler_type::multiplier, value);
}

constexpr prescaler divider(const prescaler::prescaler_list_t& prescaler_list,
	uint64_t prescaler_divider = 1) noexcept
{
	return prescaler(prescaler_type::divider, prescaler_list, prescaler_divider);
}

constexpr prescaler divider(uint64_t value) noexcept
{
	return prescaler(prescaler_type::divider, value);
}

class clock_source_config
{
public:
	static constexpr size_t max_offered_frequencies = 100u;

public:
	constexpr clock_source_config() noexcept {}

	constexpr void reset_ranges(const frequency_limits& limits) noexcept
	{
		ranges_ = {};
		ranges_.exact_frequency = limits.exact_frequency;
		offered_frequencies_ = { {} };
	}

	constexpr bool is_used() const noexcept
	{
		return used_;
	}

	constexpr void set_used(bool used) noexcept
	{
		used_ = used;
	}

	constexpr const frequency_limits& get_ranges() const noexcept
	{
		return ranges_;
	}

	constexpr void set_exact_frequency(uint64_t frequency) noexcept
	{
		ranges_.exact_frequency = frequency;
		offered_frequencies_ = { {} };
	}

	constexpr uint64_t get_exact_frequency() const noexcept
	{
		return ranges_.exact_frequency;
	}

	constexpr bool is_frequency_ok(uint64_t frequency) const noexcept
	{
		if (ranges_.exact_frequency)
			return ranges_.exact_frequency == frequency;

		return true;
	}

	constexpr void extend_limits(const frequency_limits& source_limits, uint64_t frequency) noexcept
	{
		if (ranges_.exact_frequency)
			return;

		if (!source_limits.max_frequency || frequency <= source_limits.max_frequency)
		{
			if (!ranges_.max_frequency || ranges_.max_frequency < frequency)
				ranges_.max_frequency = frequency;
		}

		if (!source_limits.min_frequency || frequency >= source_limits.min_frequency)
		{
			if (!ranges_.min_frequency || ranges_.min_frequency > frequency)
				ranges_.min_frequency = frequency;
		}
	}

	constexpr void mark_prescaler_valid(size_t prescaler_index) noexcept
	{
		valid_prescalers_[prescaler_index] = true;
	}

	constexpr memoized_frequency offer_frequency(uint64_t frequency) const noexcept
	{
		auto freq_rem = frequency % 1'000'000;
		auto freq_quot = static_cast<size_t>(frequency / 1'000'000);
		if (!freq_rem && freq_quot < max_offered_frequencies)
			return offered_frequencies_[freq_quot];

		return memoized_frequency::unknown;
	}

	constexpr void memoize_frequency(uint64_t frequency, bool accept) noexcept
	{
		auto freq_rem = frequency % 1'000'000;
		auto freq_quot = static_cast<size_t>(frequency / 1'000'000);
		if (!freq_rem && freq_quot < max_offered_frequencies)
			offered_frequencies_[freq_quot] = accept ? memoized_frequency::accepted : memoized_frequency::rejected;
	}

	constexpr void set_prescaler(const prescaler& value) noexcept
	{
		prescaler_ = value;
		valid_prescalers_ = { {} };
	}

	constexpr const prescaler& get_prescaler() const noexcept
	{
		return prescaler_;
	}

	constexpr void fill_valid_prescalers() noexcept
	{
		size_t valid = 0;
		size_t prescaler_count = prescaler_.count();
		auto& prescaler_list = prescaler_.get_prescaler_list();
		for (size_t i = 0; i != prescaler_count; ++i)
		{
			if (is_prescaler_valid(i))
			{
				prescaler_list[valid++] = prescaler_list[i];
				valid_prescalers_[i] = false;
			}
		}

		if (valid < prescaler_list.size())
			prescaler_list[valid] = 0;
	}

	constexpr bool is_final() const noexcept
	{
		const auto& prescalers = prescaler_.get_prescaler_list();
		return prescalers[0] && !prescalers[1];
	}

private:
	constexpr bool is_prescaler_valid(size_t prescaler_index) const noexcept
	{
		return valid_prescalers_[prescaler_index];
	}

private:
	frequency_limits ranges_;
	prescaler prescaler_;
	std::array<bool, max_clock_prescalers> valid_prescalers_{ {} };
	std::array<memoized_frequency, max_offered_frequencies> offered_frequencies_{ {} };
	bool used_ = false;
};

class final_clock_source_config
{
public:
	constexpr final_clock_source_config() noexcept {}

	constexpr final_clock_source_config(const clock_source_config& config) noexcept
		: exact_frequency_(config.get_exact_frequency())
		, prescaler_type_(config.get_prescaler().get_type())
		, prescaler_value_(config.get_prescaler().get_prescaler_list()[0])
		, prescaler_divider_(config.get_prescaler().get_prescaler_divider())
		, used_(config.is_used())
	{
	}

	constexpr uint64_t get_exact_frequency() const noexcept
	{
		return exact_frequency_;
	}

	constexpr prescaler_type get_prescaler_type() const noexcept
	{
		return prescaler_type_;
	}

	constexpr uint64_t get_prescaler_value() const noexcept
	{
		return prescaler_value_;
	}

	constexpr uint64_t get_prescaler_divider() const noexcept
	{
		return prescaler_divider_;
	}

	constexpr bool is_used() const noexcept
	{
		return used_;
	}

private:
	uint64_t exact_frequency_ = 0u;
	prescaler_type prescaler_type_ = prescaler_type::multiplier;
	uint64_t prescaler_value_ = 0u;
	uint64_t prescaler_divider_ = 0u;
	bool used_ = false;
};

class clock_source;
using prescaler_validator_t = bool(*)(uint64_t parent_prescaler_value, uint64_t child_prescaler_value);

class clock_source
{
public:
	static constexpr size_t invalid_tree_index = (std::numeric_limits<size_t>::max)();

public:
	template<typename Id>
	constexpr clock_source(Id id) noexcept
		: id_(static_cast<uint32_t>(id))
	{
	}

	template<auto Min, auto Max>
	constexpr clock_source& operator<<(types::limits<Min, Max>) noexcept
	{
		limits_.min_frequency = Min;
		limits_.max_frequency = Max;
		return *this;
	}

	constexpr clock_source& operator<<(min_frequency frequency) noexcept
	{
		limits_.min_frequency = frequency.get_frequency();
		return *this;
	}

	constexpr clock_source& operator<<(max_frequency frequency) noexcept
	{
		limits_.max_frequency = frequency.get_frequency();
		return *this;
	}

	constexpr clock_source& operator<<(exact_frequency frequency) noexcept
	{
		limits_.exact_frequency = frequency.get_frequency();
		return *this;
	}

	constexpr clock_source& operator<<(const frequency_limits& limits) noexcept
	{
		if (limits.exact_frequency)
			limits_.exact_frequency = limits.exact_frequency;

		if (limits.max_frequency)
		{
			if (!limits_.max_frequency || limits_.max_frequency > limits.max_frequency)
				limits_.max_frequency = limits.max_frequency;
		}

		if (limits.min_frequency)
		{
			if (!limits_.min_frequency || limits_.min_frequency < limits.min_frequency)
				limits_.min_frequency = limits.min_frequency;
		}

		return *this;
	}

	constexpr clock_source& operator<<(const prescaler& value) noexcept
	{
		prescaler_ = value;
		return *this;
	}

	constexpr clock_source& operator<<(prescaler_validator_t value) noexcept
	{
		prescaler_validator_ = value;
		return *this;
	}

	constexpr void set_tree_index(size_t index) noexcept
	{
		tree_index_ = index;
	}

	constexpr size_t get_tree_index() const noexcept
	{
		return tree_index_;
	}

	constexpr const prescaler& get_prescaler() const noexcept
	{
		return prescaler_;
	}

	constexpr uint64_t get_exact_frequency() const noexcept
	{
		return limits_.exact_frequency;
	}

	constexpr const frequency_limits& get_limits() const noexcept
	{
		return limits_;
	}

	constexpr uint32_t get_id() const noexcept
	{
		return id_;
	}
	
	constexpr bool is_prescaler_valid(uint64_t parent_prescaler_value, uint64_t child_prescaler_value) const noexcept
	{
		return prescaler_validator_ ? prescaler_validator_(parent_prescaler_value, child_prescaler_value) : true;
	}
	
	constexpr bool has_prescaler_validator() const noexcept
	{
		return prescaler_validator_ != nullptr;
	}

	constexpr bool is_frequency_ok(uint64_t frequency) const noexcept
	{
		if (limits_.exact_frequency)
			return limits_.exact_frequency == frequency;

		if (limits_.max_frequency && frequency > limits_.max_frequency)
			return false;
		if (limits_.min_frequency && frequency < limits_.min_frequency)
			return false;

		return true;
	}

private:
	uint32_t id_;
	frequency_limits limits_;
	prescaler prescaler_;
	size_t tree_index_ = invalid_tree_index;
	clock_source_config config_;
	prescaler_validator_t prescaler_validator_ = nullptr;
};

class clock_relationships
{
public:
	constexpr clock_relationships() noexcept {}

	constexpr void add_parent(size_t parent_index) noexcept
	{
		parents_.at(parent_count_++) = parent_index;
	}

	constexpr size_t get_parent_count() const noexcept
	{
		return parent_count_;
	}

	constexpr const auto& get_parents() const noexcept
	{
		return parents_;
	}

	constexpr void add_child(size_t child_index) noexcept
	{
		children_.at(child_count_++) = child_index;
	}

	constexpr void reset_children() noexcept
	{
		child_count_ = 0;
	}

	constexpr size_t get_child_count() const noexcept
	{
		return child_count_;
	}

	constexpr size_t get_child(size_t index) const noexcept
	{
		return children_[index];
	}

	constexpr void reset_selected_parent() noexcept
	{
		selected_parent_ = 0u;
	}

	constexpr bool set_next_selected_parent() noexcept
	{
		if (++selected_parent_ >= parent_count_)
		{
			selected_parent_ = 0;
			return false;
		}
		return true;
	}

	constexpr size_t get_selected_parent() const noexcept
	{
		return parents_[selected_parent_];
	}

private:
	std::array<size_t, max_parent_sources> parents_{ {} };
	size_t parent_count_ = 0u;
	std::array<size_t, max_child_sources> children_{ {} };
	size_t child_count_ = 0u;
	size_t selected_parent_ = 0u;
};

class tree_parent
{
public:
	explicit constexpr tree_parent(const clock_source& source) noexcept
		: source_(source)
	{
	}

	constexpr size_t get_tree_index() const noexcept
	{
		return source_.get_tree_index();
	}

private:
	const clock_source& source_;
};

using source_config_list_t = std::array<clock_source_config, max_clock_sources>;
using final_source_config_list_t = std::array<final_clock_source_config, max_clock_sources>;
using source_relationship_list_t = std::array<clock_relationships, max_clock_sources>;
constexpr size_t unused_source_id = std::numeric_limits<size_t>::max();

class clock_tree_base;

template<typename SourceId>
class clock_tree_result
{
public:
	constexpr clock_tree_result() noexcept {}
	
	constexpr const final_clock_source_config get_config_by_id(SourceId id) const noexcept
	{
		const auto index = source_id_to_index_[static_cast<size_t>(id)];
		if (index == unused_source_id)
			return {};
		
		return best_source_configs_[index];
	}

	constexpr SourceId get_node_parent(SourceId id, SourceId default_value = {}) const noexcept
	{
		const auto index = source_id_to_index_[static_cast<size_t>(id)];
		if (index == unused_source_id)
			return default_value;

		if (!best_source_relationships_[index].get_parent_count() || !best_source_configs_[index].is_used())
			return default_value;

		const auto parent_index = best_source_relationships_[index].get_selected_parent();
		return source_index_to_id_[parent_index];
	}
	
	constexpr bool is_valid() const noexcept
	{
		return is_valid_;
	}

private:
	friend class clock_tree_base;
	bool is_valid_ = false;
	std::array<SourceId, max_clock_sources> source_index_to_id_{ {} };
	std::array<size_t, max_clock_sources> source_id_to_index_{ {} };
	final_source_config_list_t best_source_configs_{ {} };
	source_relationship_list_t best_source_relationships_{ {} };
};

class clock_tree_base
{
public:
	constexpr clock_tree_base() noexcept {}
	clock_tree_base(clock_tree_base&) = delete;
	clock_tree_base& operator=(clock_tree_base&) = delete;

	template<typename SourceId>
	constexpr clock_tree_result<SourceId> choose_best(frequency_requirements reqs) noexcept
	{
		clock_tree_result<SourceId> result;
		bool first_tree = true;

		set_first_tree_state();
		do
		{
			if (process(reqs))
			{
				if (first_tree)
				{
					save_best_config(result);
					first_tree = false;
				}
				else if (is_tree_better(result.best_source_configs_, reqs))
				{
					save_best_config(result);
				}
			}
		} while (set_next_tree_state());

		result.is_valid_ = !first_tree;
		if (result.is_valid_)
		{
			for (size_t i = 0; i != result.source_id_to_index_.size(); ++i)
				result.source_id_to_index_[i] = unused_source_id;

			for (size_t i = 0; i != source_count_; ++i)
			{
				result.source_index_to_id_[i] = static_cast<SourceId>(sources_[i]->get_id());
				result.source_id_to_index_[sources_[i]->get_id()] = i;
			}
		}

		return result;
	}

private:
	class clock_source_tree_wrapper
	{
	public:
		constexpr clock_source_tree_wrapper(clock_tree_base& tree, size_t child_index) noexcept
			: tree_(tree)
			, child_index_(child_index)
		{
		}

		constexpr clock_source_tree_wrapper& operator<<(tree_parent parent) noexcept
		{
			auto parent_index = parent.get_tree_index();
			if (parent_index != clock_source::invalid_tree_index)
				tree_.add_parent(child_index_, parent_index);
			return *this;
		}

	private:
		clock_tree_base& tree_;
		size_t child_index_;
	};

protected:
	constexpr clock_source_tree_wrapper add_source(clock_source& source) noexcept
	{
		sources_.at(source_count_) = &source;
		source.set_tree_index(source_count_);
		return clock_source_tree_wrapper(*this, source_count_++);
	}

	constexpr clock_source_tree_wrapper add_root(clock_source& source) noexcept
	{
		roots_.at(root_count_++) = &source;
		return add_source(source);
	}

	constexpr clock_source_tree_wrapper add_leaf(clock_source& source) noexcept
	{
		leaves_.at(leaf_count_++) = &source;
		return add_source(source);
	}

	template<typename... Sources>
	constexpr void set_frequency_priority(const Sources&... sources) noexcept
	{
		(..., set_source_frequency_priority(sources));
	}

private:
	template<typename SourceId>
	constexpr void save_best_config(clock_tree_result<SourceId>& best_config) const noexcept
	{
		for (size_t i = 0; i != source_count_; ++i)
			best_config.best_source_configs_[i] = source_configs_[i];

		best_config.best_source_relationships_ = source_relationships_;
	}

	constexpr bool is_tree_better(const final_source_config_list_t& old_configs, frequency_requirements reqs) noexcept
	{
		for (size_t i = 0; i != priority_count_; ++i)
		{
			auto source_index = frequency_priority_[i];
			const auto new_frequency = source_configs_[source_index].get_ranges().exact_frequency;
			const auto old_frequency = old_configs[source_index].get_exact_frequency();
			if (new_frequency > old_frequency)
				return reqs == frequency_requirements::highest;

			if (new_frequency < old_frequency)
				return reqs == frequency_requirements::lowest;
		}
		return false;
	}

	constexpr void set_first_tree_state() noexcept
	{
		for (size_t i = 0; i != source_count_; ++i)
			source_relationships_[i].reset_selected_parent();

		update_children_links();
	}

	constexpr bool set_next_tree_state() noexcept
	{
		size_t i = 0;
		for (; i != source_count_; ++i)
		{
			if (!source_relationships_[i].set_next_selected_parent())
				continue;

			break;
		}
		update_children_links();
		return i < source_count_;
	}

	constexpr bool process(frequency_requirements reqs) noexcept
	{
		reset_tree_cache();

		if (!process_step())
			return false;

		while (!is_final_tree())
		{
			if (!select_frequency(reqs))
				return false;
			if (!process_step())
				return false;
		}

		select_remaining_frequencies(reqs);
		mark_used_sources();
		return true;
	}

	constexpr void reset_config(size_t source_index) noexcept
	{
		auto& config = source_configs_[source_index];
		config.reset_ranges(sources_[source_index]->get_limits());
		config.set_prescaler(sources_[source_index]->get_prescaler());
		config.set_used(false);
	}

	constexpr void reset_tree_cache() noexcept
	{
		priority_index_ = 0;
		for (size_t i = 0; i != source_count_; ++i)
			reset_config(i);
	}

	constexpr bool is_final_tree() const noexcept
	{
		bool is_final_subtree = true;
		for (size_t i = 0; i != leaf_count_ && is_final_subtree; ++i)
			is_final_subtree = is_final_tree(leaves_[i]->get_tree_index());

		return is_final_subtree;
	}

	constexpr void mark_used_sources() noexcept
	{
		for (size_t i = 0; i != leaf_count_; ++i)
			mark_used_sources(leaves_[i]->get_tree_index());
	}

	constexpr void mark_used_sources(size_t source_index) noexcept
	{
		if (source_configs_[source_index].is_used())
			return;
		source_configs_[source_index].set_used(true);
		if (!source_relationships_[source_index].get_parent_count())
			return;
		mark_used_sources(source_relationships_[source_index].get_selected_parent());
	}

	constexpr void select_remaining_frequencies(frequency_requirements reqs) noexcept
	{
		while (select_frequency(reqs))
		{
		}
	}

	constexpr bool select_frequency(frequency_requirements reqs) noexcept
	{
		while (priority_index_ < priority_count_)
		{
			auto& config = source_configs_[frequency_priority_[priority_index_++]];
			auto ranges = config.get_ranges();
			if (ranges.exact_frequency)
				continue;
			if (!ranges.max_frequency || !ranges.min_frequency)
				continue;

			auto exact_frequency = reqs == frequency_requirements::highest
				? ranges.max_frequency : ranges.min_frequency;
			config.set_exact_frequency(exact_frequency);
			return true;
		}

		return false;
	}

	constexpr bool process_step() noexcept
	{
		clear_limits();
		for (size_t i = 0; i != root_count_; ++i)
		{
			if (!apply_frequency(roots_[i]->get_tree_index(), roots_[i]->get_exact_frequency()))
				return false;
		}

		return fill_valid_prescalers();
	}

	constexpr bool fill_valid_prescalers() noexcept
	{
		for (size_t i = 0; i != source_count_; ++i)
			source_configs_[i].fill_valid_prescalers();

		for (size_t i = 0; i != leaf_count_; ++i)
		{
			if (!source_configs_[leaves_[i]->get_tree_index()].get_prescaler().is_valid())
				return false;
		}

		return true;
	}

	constexpr void clear_limits() noexcept
	{
		for (size_t i = 0; i != source_count_; ++i)
		{
			auto exact_frequency = source_configs_[i].get_ranges().exact_frequency;
			source_configs_[i].reset_ranges(sources_[i]->get_limits());
			source_configs_[i].set_exact_frequency(exact_frequency);
		}
	}

	constexpr bool offer_frequency(uint64_t parent_prescaler, size_t source_index, uint64_t frequency) noexcept
	{
		auto& config = source_configs_[source_index];
		const auto& source = *sources_[source_index];

		if (!source.has_prescaler_validator())
		{
			auto mem_result = config.offer_frequency(frequency);
			if (mem_result != memoized_frequency::unknown)
				return mem_result == memoized_frequency::accepted;
		}

		const auto& relationships = source_relationships_[source_index];
		bool any_prescaler_valid = false;
		const auto& prescaler_value = config.get_prescaler();
		const auto& prescalers = prescaler_value.get_prescaler_list();
		for (size_t i = 0; i != prescalers.size() && prescalers[i] && !any_prescaler_valid; ++i)
		{
			if (!source.is_prescaler_valid(parent_prescaler, prescalers[i]))
				continue;
			
			auto scaled_frequency = prescaler_value.calc_child_frequency(frequency, i);
			if (!config.is_frequency_ok(scaled_frequency) || !source.is_frequency_ok(scaled_frequency))
				continue;

			bool all_children_accept_frequency = true;
			for (size_t ch = 0; ch != relationships.get_child_count() && all_children_accept_frequency; ++ch)
				all_children_accept_frequency = offer_frequency(prescalers[i], relationships.get_child(ch), scaled_frequency);

			if (all_children_accept_frequency)
				any_prescaler_valid = true;
		}

		if (!source.has_prescaler_validator())
			config.memoize_frequency(frequency, any_prescaler_valid);
		return any_prescaler_valid;
	}

	constexpr bool apply_frequency(size_t source_index, uint64_t frequency) noexcept
	{
		const auto source = *sources_[source_index];
		const auto& source_limits = source.get_limits();
		auto& config = source_configs_[source_index];
		const auto& relationships = source_relationships_[source_index];

		bool any_prescaler_valid = false;
		const auto& prescaler_value = config.get_prescaler();
		const auto& prescalers = prescaler_value.get_prescaler_list();
		for (size_t i = 0; i != prescalers.size() && prescalers[i]; ++i)
		{
			auto scaled_frequency = prescaler_value.calc_child_frequency(frequency, i);
			if (!config.is_frequency_ok(scaled_frequency) || !source.is_frequency_ok(scaled_frequency))
				continue;

			bool all_children_accept_frequency = true;
			for (size_t ch = 0; ch != relationships.get_child_count() && all_children_accept_frequency; ++ch)
				all_children_accept_frequency = offer_frequency(prescalers[i], relationships.get_child(ch), scaled_frequency);

			if (!all_children_accept_frequency)
				continue;

			config.mark_prescaler_valid(i);
			config.extend_limits(source_limits, scaled_frequency);
			any_prescaler_valid = true;
			for (size_t ch = 0; ch != relationships.get_child_count(); ++ch)
				apply_frequency(relationships.get_child(ch), scaled_frequency);
		}

		return any_prescaler_valid;
	}

	constexpr bool is_final_tree(size_t source_index) const noexcept
	{
		if (!source_configs_[source_index].is_final())
			return false;
		if (!source_relationships_[source_index].get_parent_count())
			return true;
		return is_final_tree(source_relationships_[source_index].get_selected_parent());
	}

	constexpr void add_parent(size_t child_index, size_t parent_index) noexcept
	{
		source_relationships_[child_index].add_parent(parent_index);
	}

	constexpr void set_source_frequency_priority(const clock_source& source) noexcept
	{
		auto index = source.get_tree_index();
		if (index != clock_source::invalid_tree_index)
			frequency_priority_[priority_count_++] = index;
	}

	constexpr void update_children_links() noexcept
	{
		for (size_t i = 0; i != source_count_; ++i)
			source_relationships_[i].reset_children();

		for (size_t i = 0; i != source_count_; ++i)
		{
			if (!source_relationships_[i].get_parent_count())
				continue;

			source_relationships_[source_relationships_[i].get_selected_parent()].add_child(i);
		}
	}

private:
	using tree_nodes_t = std::array<clock_source*, max_clock_sources>;
	tree_nodes_t roots_{ {} }, leaves_{ {} }, sources_{ {} };
	size_t root_count_ = 0u, leaf_count_ = 0u, source_count_ = 0u, priority_count_ = 0u;
	source_relationship_list_t source_relationships_{ {} };
	source_config_list_t source_configs_{ {} };
	std::array<size_t, max_clock_sources> frequency_priority_{ {} };
	std::size_t priority_index_ = 0u;
};

} //namespace mcutl::clock::detail
