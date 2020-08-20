#pragma once

#include <stdint.h>

#include "mcutl/backup/backup.h"
#include "mcutl/device/device.h"
#include "mcutl/exti/exti.h"
#include "mcutl/interrupt/interrupt.h"
#include "mcutl/memory/volatile_memory.h"
#include "mcutl/periph/periph.h"
#include "mcutl/rtc/rtc_defs.h"
#include "mcutl/utils/class_limitations.h"
#include "mcutl/utils/datetime.h"
#include "mcutl/utils/definitions.h"
#include "mcutl/utils/type_helpers.h"

namespace mcutl::device::rtc
{

using peripheral_type = types::list<mcutl::periph::pwr, mcutl::periph::bkp>;

[[maybe_unused]] constexpr bool supports_prescalers = true;
[[maybe_unused]] constexpr bool supports_alarm = true;
[[maybe_unused]] constexpr bool supports_second_interrupt = true;
[[maybe_unused]] constexpr bool supports_overflow_interrupt = true;
[[maybe_unused]] constexpr bool supports_internal_clock_source = true;
[[maybe_unused]] constexpr bool supports_atomic_clear_pending_flags = false;
[[maybe_unused]] constexpr bool supports_clock_source_reconfiguration = false;

struct options : mcutl::rtc::detail::options
{
	mcutl::interrupt::detail::interrupt_info external_alarm {};
	uint32_t external_alarm_set_count = 0;
};

[[maybe_unused]] constexpr uint32_t min_prescaler = 1u;
[[maybe_unused]] constexpr uint32_t max_prescaler = 0xfffffu;

template<uint32_t PrescalerValue>
[[maybe_unused]] constexpr bool prescaler_supported
	= PrescalerValue >= min_prescaler && PrescalerValue <= max_prescaler;

} //namespace mcutl::device::rtc

namespace mcutl::rtc::clock
{

using lse_crystal = external_crystal;
using lsi = internal;
struct lse_bypass {};
struct hse_div_128 {};

} //namespace mcutl::rtc::clock

namespace mcutl::rtc::interrupt
{

struct external_alarm {};

} //namespace mcutl::rtc::interrupt

namespace mcutl::rtc::detail
{

struct clock_ex
{
	enum value { lse_bypass = clock::end, hse_div_128 };
};

template<>
struct options_parser<rtc::clock::lse_bypass>
	: opts::base_option_parser<static_cast<clock::value>(clock_ex::lse_bypass),
		&options::clock_value, &options::clock_set_count> {};

template<>
struct options_parser<rtc::clock::hse_div_128>
	: opts::base_option_parser<static_cast<clock::value>(clock_ex::hse_div_128),
		&options::clock_value, &options::clock_set_count> {};

template<> struct interrupt_map<interrupt::external_alarm>
	: mcutl::interrupt::detail::map_base<&device::rtc::options::external_alarm_set_count,
		&device::rtc::options::external_alarm> {};

template<>
struct options_parser<interrupt::external_alarm>
	: mcutl::interrupt::detail::interrupt_parser<interrupt::external_alarm, interrupt_map> {};

template<> struct interrupt_type_helper<interrupt::alarm>
	: types::identity<mcutl::interrupt::type::rtc> {};
template<> struct interrupt_type_helper<interrupt::second>
	: types::identity<mcutl::interrupt::type::rtc> {};
template<> struct interrupt_type_helper<interrupt::overflow>
	: types::identity<mcutl::interrupt::type::rtc> {};
template<> struct interrupt_type_helper<interrupt::external_alarm>
	: types::identity<mcutl::interrupt::type::rtc_alarm> {};

} //namespace mcutl::rtc::detail

namespace mcutl::device::rtc
{

using rtc_exti_line_options = mcutl::exti::line_options<mcutl::exti::rtc_alarm_line,
	mcutl::exti::line_mode::interrupt, mcutl::exti::line_trigger::rising>;
	
struct rtc_config
{
	uint32_t prescaler = 0u;
	uint32_t bdcr_bits = 0u;
	uint32_t bdcr_mask = 0u;
	uint32_t crh_bits = 0u;
	uint32_t crh_mask = 0u;
	mcutl::interrupt::detail::interrupt_info global_info;
	bool has_global = false;
	bool priority_conflict = false;
};

struct controller_interrupt_map
{
	template<typename Interrupt>
	using type = mcutl::rtc::interrupt_type<Interrupt>;
};

template<typename ConfigLambda>
constexpr rtc_config get_rtc_config(ConfigLambda config_lambda) noexcept
{
	constexpr options opts = config_lambda();
	rtc_config result {};
	
	if constexpr (opts.enable_set_count != 0)
	{
		result.bdcr_mask |= RCC_BDCR_RTCEN_Msk;
		if constexpr (opts.enable)
			result.bdcr_bits |= RCC_BDCR_RTCEN;
	}
	
	if constexpr (opts.clock_set_count != 0)
	{
		result.bdcr_mask |= RCC_BDCR_RTCSEL_Msk | RCC_BDCR_LSEBYP_Msk;
		switch (static_cast<uint32_t>(opts.clock_value))
		{
		case mcutl::rtc::detail::clock::internal: //lsi
			result.bdcr_bits |= RCC_BDCR_RTCSEL_LSI;
			break;
		
		case mcutl::rtc::detail::clock::external_crystal: //lse
			result.bdcr_bits |= RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEON;
			break;
		
		case mcutl::rtc::detail::clock_ex::lse_bypass:
			result.bdcr_bits |= RCC_BDCR_RTCSEL_LSE | RCC_BDCR_LSEBYP | RCC_BDCR_LSEON;
			break;
		
		case mcutl::rtc::detail::clock_ex::hse_div_128:
			result.bdcr_bits |= RCC_BDCR_RTCSEL_HSE;
			break;
			
		default:
			break;
		}
	}
	
	if constexpr (opts.prescaler_set_count != 0)
	{
		if constexpr (opts.one_second_prescaler)
		{
			static_assert(opts.clock_set_count != 0,
				"Unable to determine the one-second prescaler when the RTC clock source is not specified");
			static_assert(opts.clock_set_count == 0
				|| static_cast<uint32_t>(opts.clock_value) != mcutl::rtc::detail::clock_ex::hse_div_128,
				"Unable to determine the one-second prescaler for the selected RTC clock source");
			
			if constexpr (static_cast<uint32_t>(opts.clock_value)
				== mcutl::rtc::detail::clock::internal)
			{
				result.prescaler = 40'000u;
			}
			else
			{
				result.prescaler = 32'767u;
			}
		}
		else
		{
			result.prescaler = opts.prescaler;
		}
	}
	
	if constexpr (opts.alarm_set_count != 0
		|| opts.external_alarm_set_count != 0)
	{
		if constexpr (opts.alarm_set_count != 0)
		{
			result.has_global = true;
			result.global_info = opts.alarm;
			if constexpr (!opts.alarm.disable)
				result.crh_bits |= RTC_CRH_ALRIE;
		}
		
		if constexpr (opts.external_alarm_set_count != 0 && !opts.external_alarm.disable)
			result.crh_bits |= RTC_CRH_ALRIE;
		
		result.crh_mask |= RTC_CRH_ALRIE_Msk;
	}
	
	if constexpr (opts.second_set_count != 0)
	{
		result.has_global = true;
		result.global_info = opts.second;
		result.crh_mask |= RTC_CRH_SECIE_Msk;
		if constexpr (!opts.second.disable)
			result.crh_bits |= RTC_CRH_SECIE;
	}
	
	if constexpr (opts.overflow_set_count != 0)
	{
		result.has_global = true;
		result.global_info = opts.overflow;
		result.crh_mask |= RTC_CRH_OWIE_Msk;
		if constexpr (!opts.overflow.disable)
			result.crh_bits |= RTC_CRH_OWIE;
	}
	
	result.priority_conflict
		= !mcutl::interrupt::detail::interrupt_priority_conflict_checker<
			mcutl::rtc::detail::interrupt_map,
			controller_interrupt_map,
			mcutl::rtc::interrupt::alarm,
			mcutl::rtc::interrupt::external_alarm,
			mcutl::rtc::interrupt::overflow,
			mcutl::rtc::interrupt::second
		>::check(opts);
	
	return result;
}

template<bool ConfigModeRequired = true>
inline void start_rtc_config() MCUTL_NOEXCEPT
{
	while (mcutl::memory::get_register_bits<RTC_CRL_RTOFF, &RTC_TypeDef::CRL, RTC_BASE>() == 0)
	{
	}
	
	//Clear RSF bit
	mcutl::memory::set_register_value<RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF,
		&RTC_TypeDef::CRL, RTC_BASE>();
	
	//Wait RSF to be set
	while (mcutl::memory::get_register_bits<RTC_CRL_RSF, &RTC_TypeDef::CRL, RTC_BASE>()
		!= RTC_CRL_RSF)
	{
	}
	
	if constexpr (ConfigModeRequired)
	{
		//Set the CNF bit and keep other bits as is
		mcutl::memory::set_register_value<
			RTC_CRL_CNF | RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF,
			&RTC_TypeDef::CRL, RTC_BASE>();
	}
}

inline void wait_rtc_sync() MCUTL_NOEXCEPT
{
	start_rtc_config<false>();
}

template<bool ConfigModeRequired = true>
inline void stop_rtc_config() MCUTL_NOEXCEPT
{
	if constexpr (ConfigModeRequired)
	{
		//Reset CNF bit to commit operation
		mcutl::memory::set_register_value<RTC_CRL_ALRF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF,
			&RTC_TypeDef::CRL, RTC_BASE>();
	}
	
	while (mcutl::memory::get_register_bits<RTC_CRL_RTOFF, &RTC_TypeDef::CRL, RTC_BASE>() == 0)
	{
	}
}

template<bool ConfigModeRequired = true>
struct rtc_config_enabler : types::noncopymovable
{
	rtc_config_enabler() MCUTL_NOEXCEPT
	{
		start_rtc_config<ConfigModeRequired>();
	}
	
#ifdef MCUTL_TEST
	~rtc_config_enabler() MCUTL_NOEXCEPT
	{
		try
		{
			stop_rtc_config<ConfigModeRequired>();
		}
		catch (...)
		{
		}
	}
#else //MCUTL_TEST
	~rtc_config_enabler() MCUTL_NOEXCEPT
	{
		stop_rtc_config<ConfigModeRequired>();
	}
#endif //MCUTL_TEST
	
private:
	mcutl::backup::backup_write_enabler<> backup_enabler;
};

inline void enable_lsi() MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_bits<RCC_CSR_LSION_Msk, RCC_CSR_LSION,
		&RCC_TypeDef::CSR, RCC_BASE>();
	while (!mcutl::memory::get_register_bits<
		RCC_CSR_LSIRDY, &RCC_TypeDef::CSR, RCC_BASE>())
	{
	}
}

inline void reset_backup_domain(uint32_t bdcr) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<&RCC_TypeDef::BDCR, RCC_BASE>(
		bdcr | RCC_BDCR_BDRST);
	while (mcutl::memory::get_register_bits<
		RCC_BDCR_BDRST_Msk, &RCC_TypeDef::BDCR, RCC_BASE>())
	{
	}
}

inline void disable_lse(uint32_t bdcr) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<&RCC_TypeDef::BDCR, RCC_BASE>(bdcr);
	while (mcutl::memory::get_register_bits<
		RCC_BDCR_LSERDY, &RCC_TypeDef::BDCR, RCC_BASE>())
	{
	}
}

inline void enable_lse(uint32_t bdcr) MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<&RCC_TypeDef::BDCR, RCC_BASE>(bdcr);
	while (!mcutl::memory::get_register_bits<
		RCC_BDCR_LSERDY, &RCC_TypeDef::BDCR, RCC_BASE>())
	{
	}
}

template<uint32_t Bdcr>
inline void enable_lse() MCUTL_NOEXCEPT
{
	mcutl::memory::set_register_value<Bdcr, &RCC_TypeDef::BDCR, RCC_BASE>();
	while (!mcutl::memory::get_register_bits<
		RCC_BDCR_LSERDY, &RCC_TypeDef::BDCR, RCC_BASE>())
	{
	}
}

template<typename OptionsLambda>
void configure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr options opts = options_lambda();
	constexpr rtc_config config = get_rtc_config(options_lambda);
	static_assert(!config.priority_conflict, "Conflicting RTC interrupt priorities");
	static_assert(opts.clock_set_count != 0, "RTC clock source is not selected");
	static_assert(opts.prescaler_set_count != 0, "RTC prescaler is not selected");

	constexpr auto clocksel = config.bdcr_bits & RCC_BDCR_RTCSEL_Msk;
	
	mcutl::backup::backup_write_enabler<> backup_writes_enabled;
	
	if constexpr (!opts.base_configuration_set_count)
	{
		auto bdcr_value = mcutl::memory::get_register_bits<&RCC_TypeDef::BDCR, RCC_BASE>();
		uint32_t current_clock_sel = bdcr_value & (RCC_BDCR_RTCSEL_Msk | RCC_BDCR_LSEBYP_Msk);
		if (current_clock_sel != (config.bdcr_bits & (RCC_BDCR_RTCSEL_Msk | RCC_BDCR_LSEBYP_Msk)))
		{
			if (bdcr_value & RCC_BDCR_RTCSEL_Msk)
			{
				reset_backup_domain(bdcr_value);
				bdcr_value = 0;
			}
			else
			{
				bdcr_value &= ~RCC_BDCR_RTCEN;
			
				if (clocksel == RCC_BDCR_RTCSEL_HSE || clocksel == RCC_BDCR_RTCSEL_LSI
					|| (bdcr_value & RCC_BDCR_LSEBYP) != (config.bdcr_bits & RCC_BDCR_LSEBYP))
				{
					bdcr_value &= ~(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
					disable_lse(bdcr_value);
				}
			}
			
			if constexpr (clocksel == RCC_BDCR_RTCSEL_LSI)
			{
				enable_lsi();
			}
			else if constexpr (clocksel == RCC_BDCR_RTCSEL_LSE)
			{
				bdcr_value |= RCC_BDCR_LSEON | (config.bdcr_bits & RCC_BDCR_LSEBYP);
				enable_lse(bdcr_value);
			}
		}
	}
	else
	{
		if constexpr (clocksel == RCC_BDCR_RTCSEL_LSI)
			enable_lsi();
		else if constexpr (clocksel == RCC_BDCR_RTCSEL_LSE)
			enable_lse<config.bdcr_bits & (RCC_BDCR_LSEON | RCC_BDCR_LSEBYP)>();
	}
		
	mcutl::memory::set_register_value<config.bdcr_bits & ~RCC_BDCR_RTCEN,
		&RCC_TypeDef::BDCR, RCC_BASE>();
	
	start_rtc_config();
	
	if constexpr (!opts.base_configuration_set_count || config.crh_bits)
		mcutl::memory::set_register_value<config.crh_bits, &RTC_TypeDef::CRH, RTC_BASE>();
	
	mcutl::memory::set_register_value<config.prescaler & RTC_PRLL_PRL_Msk, &RTC_TypeDef::PRLL, RTC_BASE>();
	mcutl::memory::set_register_value<(config.prescaler >> 16u) & RTC_PRLH_PRL_Msk, &RTC_TypeDef::PRLH, RTC_BASE>();
	
	stop_rtc_config();
	
	if constexpr (opts.disable_controller_interrupts_set_count != 0)
	{
		if constexpr (!config.crh_bits || (config.crh_bits == RTC_CRH_ALRIE
			&& (!opts.alarm_set_count || opts.alarm.disable)))
		{
			mcutl::interrupt::disable<mcutl::interrupt::type::rtc>();
		}
		
		if constexpr (!opts.external_alarm_set_count || opts.external_alarm.disable)
		{
			mcutl::exti::disable_lines<rtc_exti_line_options>();
			mcutl::interrupt::disable<mcutl::interrupt::type::rtc_alarm>();
		}
	}
	
	if constexpr (opts.enable_controller_interrupts_set_count != 0)
	{
		if constexpr (config.crh_bits != 0
			&& !(config.crh_bits == RTC_CRH_ALRIE
				&& (!opts.alarm_set_count || opts.alarm.disable)))
		{
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::interrupt::type::rtc,
					config.global_info.priority,
					config.global_info.subpriority
				>,
				opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
		
		if constexpr (opts.external_alarm_set_count && !opts.external_alarm.disable)
		{
			mcutl::exti::enable_lines<rtc_exti_line_options>();
			mcutl::interrupt::enable<
				mcutl::interrupt::interrupt<
					mcutl::interrupt::type::rtc_alarm,
					opts.external_alarm.priority,
					opts.external_alarm.subpriority
				>,
				opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
			>();
		}
	}

	if constexpr (opts.enable)
		mcutl::memory::set_register_value<config.bdcr_bits, &RCC_TypeDef::BDCR, RCC_BASE>();
}

template<typename OptionsLambda>
void reconfigure(OptionsLambda options_lambda) MCUTL_NOEXCEPT
{
	constexpr options opts = options_lambda();
	constexpr rtc_config config = get_rtc_config(options_lambda);
	static_assert(!config.priority_conflict, "Conflicting RTC interrupt priorities");
	static_assert(!opts.base_configuration_set_count,
		"base_configuration_is_currently_present can not be used with reconfigure()");
	static_assert((config.bdcr_mask & (RCC_BDCR_RTCSEL_Msk | RCC_BDCR_LSEBYP_Msk)) == 0,
		"Reconfiguring RTC clock source is not supported, use configure() instead");
	
	[[maybe_unused]] bool backup_writes_enabled = false;
	if constexpr (config.bdcr_mask || config.crh_mask || config.prescaler)
		backup_writes_enabled = mcutl::backup::enable_backup_writes();
	
	if constexpr (config.crh_mask || config.prescaler)
		start_rtc_config();
	
	[[maybe_unused]] bool had_alarm_interrupt = false;
	if constexpr (config.crh_mask != 0)
	{
		auto current_crh = mcutl::memory::get_register_bits<&RTC_TypeDef::CRH, RTC_BASE>();
		had_alarm_interrupt = (current_crh & RTC_CRH_ALRIE) != 0;
		
		if constexpr ((opts.alarm_set_count && opts.external_alarm_set_count)
			|| (config.crh_bits & RTC_CRH_ALRIE) || !(config.crh_mask & RTC_CRH_ALRIE_Msk))
		{
			current_crh &= ~config.crh_mask;
			current_crh |= config.crh_bits;
			mcutl::memory::set_register_value<&RTC_TypeDef::CRH, RTC_BASE>(current_crh);
		}
		else
		{
			//Either alarm or external_alarm was disabled
			//(config.crh_bits & RTC_CRH_ALRIE) == 0 in this branch
			auto crh_bits = config.crh_bits;
			if ((current_crh & RTC_CRH_ALRIE) != 0)
			{
				if constexpr (opts.alarm_set_count != 0)
				{
					if (mcutl::interrupt::is_enabled<mcutl::interrupt::type::rtc_alarm>())
						crh_bits |= RTC_CRH_ALRIE;
				}
				else //external_alarm_set_count != 0
				{
					if (mcutl::interrupt::is_enabled<mcutl::interrupt::type::rtc>())
						crh_bits |= RTC_CRH_ALRIE;
				}
			}
			
			current_crh &= ~config.crh_mask;
			current_crh |= crh_bits;
			
			mcutl::memory::set_register_value<&RTC_TypeDef::CRH, RTC_BASE>(current_crh);
		}
	}
	
	if constexpr (config.prescaler != 0)
	{
		mcutl::memory::set_register_value<config.prescaler & RTC_PRLL_PRL_Msk,
			&RTC_TypeDef::PRLL, RTC_BASE>();
		mcutl::memory::set_register_value<(config.prescaler >> 16u) & RTC_PRLH_PRL_Msk,
			&RTC_TypeDef::PRLH, RTC_BASE>();
	}
	
	if constexpr (config.crh_mask || config.prescaler)
		stop_rtc_config();
	
	if constexpr (opts.disable_controller_interrupts_set_count != 0
		|| opts.enable_controller_interrupts_set_count != 0)
	{
		auto crh = mcutl::memory::get_register_bits<&RTC_TypeDef::CRH, RTC_BASE>();
		if constexpr (!config.crh_mask)
			had_alarm_interrupt = (crh & RTC_CRH_ALRIE) != 0;
		
		if constexpr (opts.disable_controller_interrupts_set_count != 0)
		{
			if (!crh)
				mcutl::interrupt::disable<mcutl::interrupt::type::rtc>();
			
			if (opts.external_alarm.disable || !crh)
			{
				mcutl::exti::disable_lines<rtc_exti_line_options>();
				mcutl::interrupt::disable<mcutl::interrupt::type::rtc_alarm>();
			}
		}
		
		if constexpr (opts.enable_controller_interrupts_set_count != 0)
		{
			if (crh && (crh != RTC_CRH_ALRIE || had_alarm_interrupt
				|| (opts.alarm_set_count && !opts.alarm.disable)))
			{
				mcutl::interrupt::enable<
					mcutl::interrupt::interrupt<
						mcutl::interrupt::type::rtc,
						config.global_info.priority,
						config.global_info.subpriority
					>,
					opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
				>();
			}
			
			if constexpr (opts.external_alarm_set_count && !opts.external_alarm.disable)
			{
				mcutl::exti::enable_lines<rtc_exti_line_options>();
				mcutl::interrupt::enable<
					mcutl::interrupt::interrupt<
						mcutl::interrupt::type::rtc_alarm,
						opts.external_alarm.priority,
						opts.external_alarm.subpriority
					>,
					opts.priority_count_set_count ? opts.priority_count : mcutl::interrupt::maximum_priorities
				>();
			}
		}
	}
	
	mcutl::memory::set_register_bits<config.bdcr_mask, config.bdcr_bits, &RCC_TypeDef::BDCR, RCC_BASE>();
	
	if constexpr (config.bdcr_mask || config.crh_mask || config.prescaler)
	{
		if (backup_writes_enabled)
			mcutl::backup::disable_backup_writes();
	}
}

template<typename Interrupt>
struct interrupt_info
{
	static_assert(types::always_false<Interrupt>::value, "Unknown RTC interrupt type");
};

template<> struct interrupt_info<mcutl::rtc::interrupt::alarm>
	: std::integral_constant<uint32_t, RTC_CRL_ALRF> {};
template<> struct interrupt_info<mcutl::rtc::interrupt::external_alarm>
	: std::integral_constant<uint32_t, RTC_CRL_ALRF> {};
template<> struct interrupt_info<mcutl::rtc::interrupt::overflow>
	: std::integral_constant<uint32_t, RTC_CRL_OWF> {};
template<> struct interrupt_info<mcutl::rtc::interrupt::second>
	: std::integral_constant<uint32_t, RTC_CRL_SECF> {};
template<typename Interrupt,
	mcutl::interrupt::priority_t Priority, mcutl::interrupt::priority_t SubPriority>
struct interrupt_info<mcutl::interrupt::interrupt<Interrupt, Priority, SubPriority>>
	: interrupt_info<Interrupt> {};

template<typename Interrupt>
struct exti_pending_alarm
{
	static constexpr void clear() noexcept
	{
	}
};

template<>
struct exti_pending_alarm<mcutl::rtc::interrupt::external_alarm>
{
	static void clear() MCUTL_NOEXCEPT
	{
		mcutl::exti::clear_pending_line_bits<rtc_exti_line_options>();
	}
};

template<typename... Interrupts>
[[maybe_unused]] constexpr auto pending_flags_v = (... | interrupt_info<Interrupts>::value);

template<typename... Interrupts>
void clear_pending_flags() MCUTL_NOEXCEPT
{
	constexpr uint32_t flags_to_set
		= (RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF | RTC_CRL_ALRF)
		& ~pending_flags_v<Interrupts...>;
	(exti_pending_alarm<Interrupts>::clear(), ...);
	rtc_config_enabler<false> enabler;
	mcutl::memory::set_register_value<flags_to_set, &RTC_TypeDef::CRL, RTC_BASE>();
}

template<typename... Interrupts>
constexpr void clear_pending_flags_atomic() noexcept
{
	static_assert(types::always_false<Interrupts...>::value,
		"clear_pending_flags_atomic is not supported by the RTC");
}

inline bool is_enabled() MCUTL_NOEXCEPT
{
	auto bdcr_value = mcutl::memory::get_register_bits<&RCC_TypeDef::BDCR, RCC_BASE>();
	return (bdcr_value & RCC_BDCR_RTCEN) && (bdcr_value & RCC_BDCR_RTCSEL);
}

template<typename... Interrupts>
auto get_pending_flags() MCUTL_NOEXCEPT
{
	wait_rtc_sync();
	return pending_flags_v<Interrupts...>
		& mcutl::memory::get_register_bits<&RTC_TypeDef::CRL, RTC_BASE>();
}

template<typename T>
[[nodiscard]] inline bool has_alarmed() MCUTL_NOEXCEPT
{
	return get_pending_flags<mcutl::rtc::interrupt::alarm>() != 0u;
}

inline uint32_t get_rtc_counter() MCUTL_NOEXCEPT
{
	wait_rtc_sync();
	uint16_t high1 = static_cast<uint16_t>(mcutl::memory::get_register_bits<
		RTC_CNTH_RTC_CNT, &RTC_TypeDef::CNTH, RTC_BASE>());
	uint16_t low = static_cast<uint16_t>(mcutl::memory::get_register_bits<
		RTC_CNTL_RTC_CNT, &RTC_TypeDef::CNTL, RTC_BASE>());
	uint16_t high2 = static_cast<uint16_t>(mcutl::memory::get_register_bits<
		RTC_CNTH_RTC_CNT, &RTC_TypeDef::CNTH, RTC_BASE>());

	if (high1 != high2)
	{
		return (static_cast<uint32_t>(high2) << 16u)
			| static_cast<uint16_t>(mcutl::memory::get_register_bits<
				RTC_CNTL_RTC_CNT, &RTC_TypeDef::CNTL, RTC_BASE>());
	}
	
	return (static_cast<uint32_t>(high1) << 16u) | low;
}

inline void set_rtc_counter(uint32_t timestamp) MCUTL_NOEXCEPT
{
	rtc_config_enabler<true> enabler;
	mcutl::memory::set_register_value<0, &RTC_TypeDef::CNTL, RTC_BASE>();
	mcutl::memory::set_register_value<&RTC_TypeDef::CNTH, RTC_BASE>(timestamp >> 16u);
	mcutl::memory::set_register_value<&RTC_TypeDef::CNTL, RTC_BASE>(timestamp & RTC_CNTL_RTC_CNT);
}

template<typename DateTime>
inline void get_date_time(DateTime& date_time) MCUTL_NOEXCEPT
{
	if constexpr (std::is_same_v<DateTime, datetime::date_time>)
	{
		date_time = datetime::date_time_from_timestamp(get_rtc_counter());
	}
	else
	{
		constexpr bool has_time = datetime::traits::has_second_v<DateTime>
			|| datetime::traits::has_minute_v<DateTime>
			|| datetime::traits::has_hour_v<DateTime>;
		constexpr bool has_date = datetime::traits::has_day_v<DateTime>
			|| datetime::traits::has_month_v<DateTime>
			|| datetime::traits::has_year_v<DateTime>;
		constexpr bool has_timestamp = datetime::traits::has_timestamp_v<DateTime>;
		
		static_assert(has_time || has_date || has_timestamp,
			"No applicable date or time fields found in DateTime type");
		
		datetime::date_time value;
		auto timestamp = get_rtc_counter();
		if constexpr (has_timestamp)
			date_time.timestamp = timestamp;
		
		if constexpr (has_date)
			static_cast<datetime::date_value&>(value) = datetime::date_from_timestamp(timestamp);
		if constexpr (has_time)
			static_cast<datetime::time_value&>(value) = datetime::time_from_timestamp(timestamp);
	
		if constexpr (datetime::traits::has_second_v<DateTime>)
			date_time.second = value.second;
		if constexpr (datetime::traits::has_minute_v<DateTime>)
			date_time.minute = value.minute;
		if constexpr (datetime::traits::has_hour_v<DateTime>)
			date_time.hour = value.hour;
		if constexpr (datetime::traits::has_day_v<DateTime>)
			date_time.day = value.day;
		if constexpr (datetime::traits::has_month_v<DateTime>)
			date_time.month = value.month;
		if constexpr (datetime::traits::has_year_v<DateTime>)
			date_time.year = value.year;
	}
}

template<typename DateTime>
constexpr datetime::date_time copy_date_time(const DateTime& value) noexcept
{
	datetime::date_time copy {};
	copy.day = value.day;
	copy.month = value.month;
	copy.year = value.year;
	copy.second = value.second;
	copy.minute = value.minute;
	copy.hour = value.hour;
	return copy;
}

template<typename DateTimeOrTimeStamp>
inline void set_date_time(const DateTimeOrTimeStamp& value) MCUTL_NOEXCEPT
{
	if constexpr (std::is_same_v<DateTimeOrTimeStamp, datetime::date_time>)
	{
		set_rtc_counter(datetime::timestamp_from_date_time(value));
	}
	else if constexpr (std::is_integral_v<DateTimeOrTimeStamp>)
	{
		set_rtc_counter(value);
	}
	else
	{
		set_rtc_counter(datetime::timestamp_from_date_time(copy_date_time(value)));
	}
}

template<typename T>
inline void disable_alarm() MCUTL_NOEXCEPT
{
	{
		rtc_config_enabler<false> enabler;
		mcutl::memory::set_register_bits<RTC_CRH_ALRIE_Msk, ~RTC_CRH_ALRIE,
			&RTC_TypeDef::CRH, RTC_BASE>();
		mcutl::memory::set_register_value<
			RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF,
			&RTC_TypeDef::CRL, RTC_BASE>();
	}
	
	exti_pending_alarm<mcutl::rtc::interrupt::external_alarm>::clear();
}

template<typename DateTimeOrTimeStamp>
void set_alarm(const DateTimeOrTimeStamp& value) MCUTL_NOEXCEPT
{
	if constexpr (std::is_integral_v<DateTimeOrTimeStamp>)
	{
		exti_pending_alarm<mcutl::rtc::interrupt::external_alarm>::clear();
		
		{
			rtc_config_enabler<true> enabler;
			
			mcutl::memory::set_register_value<
				&RTC_TypeDef::ALRH, RTC_BASE>((value >> 16u) & RTC_ALRH_RTC_ALR);
			mcutl::memory::set_register_value<
				&RTC_TypeDef::ALRL, RTC_BASE>(value & RTC_ALRL_RTC_ALR);
			
			//Clear ALRF bit
			mcutl::memory::set_register_value<RTC_CRL_CNF | RTC_CRL_SECF | RTC_CRL_OWF | RTC_CRL_RSF,
				&RTC_TypeDef::CRL, RTC_BASE>();
		}
	}
	else if constexpr (std::is_same_v<DateTimeOrTimeStamp, datetime::date_time>)
	{
		set_alarm(datetime::timestamp_from_date_time(value));
	}
	else
	{
		set_alarm(datetime::timestamp_from_date_time(copy_date_time(value)));
	}
}

} //namespace mcutl::device::rtc
