#pragma once

#include <chrono>
#include <stdint.h>

namespace mcutl::types
{

template<typename Duration, auto>
struct duration;

template<typename Rep, typename Period, Rep Interval>
struct duration<std::chrono::duration<Rep, Period>, Interval>
{
	using duration_type = std::chrono::duration<Rep, Period>;
	static constexpr auto interval = Interval;
};

template<uint16_t Interval>
using nanoseconds = duration<std::chrono::duration<uint16_t, std::nano>, Interval>;
template<uint16_t Interval>
using microseconds = duration<std::chrono::duration<uint16_t, std::micro>, Interval>;
template<uint16_t Interval>
using milliseconds = duration<std::chrono::duration<uint16_t, std::milli>, Interval>;
template<uint16_t Interval>
using seconds = duration<std::chrono::duration<uint16_t>, Interval>;

} //namespace mcutl::types
