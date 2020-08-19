#pragma once

#include <stdint.h>
#include <limits>

#include "mcutl/utils/member_checker.h"

namespace mcutl::datetime
{

using timestamp_t = uint32_t;

[[maybe_unused]] constexpr uint8_t max_hour = 23;
[[maybe_unused]] constexpr uint8_t max_minute = 59;
[[maybe_unused]] constexpr uint8_t max_second = 59;
[[maybe_unused]] constexpr uint8_t min_hour = 0;
[[maybe_unused]] constexpr uint8_t min_minute = 0;
[[maybe_unused]] constexpr uint8_t min_second = 0;

enum class weekday : uint8_t
{
	sunday,
	monday,
	tuesday,
	wednesday,
	thursday,
	friday,
	saturday,
	
	max_value
};

enum class month : uint8_t
{
	january   = 1,
	february,
	march,
	april,
	may,
	june,
	july,
	august,
	september,
	october,
	november,
	december,
	
	max_value
};

[[maybe_unused]] constexpr uint8_t min_month = static_cast<uint8_t>(month::january);
[[maybe_unused]] constexpr uint8_t max_month = static_cast<uint8_t>(month::december);
[[maybe_unused]] constexpr uint8_t min_day = 1;
[[maybe_unused]] constexpr uint8_t max_day = 31;

struct date_value
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
};

struct time_value
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

struct date_time : date_value, time_value
{
};

struct date_time_with_timestamp : date_time
{
	timestamp_t timestamp;
};

//Based on https://howardhinnant.github.io/date_algorithms.html
[[nodiscard]] constexpr date_value civil_from_days(uint32_t days_from_epoch) noexcept
{
	const uint32_t doe = days_from_epoch;								// [0, 146096]
	const uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;		// [0, 399]
	date_value result {};
	const uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);  		// [0, 365]
	const uint32_t mp = (5 * doy + 2) / 153;  							// [0, 11]
	result.day = static_cast<uint8_t>(doy - (153 * mp + 2) / 5 + 1);	// [1, 31]
	result.month = static_cast<uint8_t>(mp + (mp < 10 ? 3 : -9));		// [1, 12]
	result.year = static_cast<uint16_t>(yoe + 2000u + (result.month <= 2));
	return result;
}

[[nodiscard]] constexpr uint32_t days_from_civil(date_value value) noexcept
{
	value.year -= value.month <= 2;
	const uint32_t yoe = static_cast<uint32_t>(value.year - 2000u); 	// [0, 399]
	const uint32_t doy = (153 * (value.month + (value.month > 2 ? -3 : 9)) + 2) / 5 + value.day - 1;  // [0, 365]
	const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; 			// [0, 146096]
	return doe;
}

[[nodiscard]] constexpr weekday weekday_from_days(uint32_t days_from_epoch) noexcept
{
	return static_cast<weekday>((days_from_epoch + 3) % 7);
}

[[nodiscard]] constexpr time_value time_from_timestamp(timestamp_t timestamp) noexcept
{
	time_value result {};
	result.second = timestamp % 60;
	timestamp /= 60;
	result.minute = timestamp % 60;
	timestamp /= 60;
	result.hour = timestamp % 24;
	return result;
}

[[nodiscard]] constexpr date_value date_from_timestamp(timestamp_t timestamp) noexcept
{
	return civil_from_days(timestamp / (60 * 60 * 24));
}

[[nodiscard]] constexpr date_time date_time_from_timestamp(timestamp_t timestamp) noexcept
{
	date_time result {};
	static_cast<time_value&>(result) = time_from_timestamp(timestamp);
	static_cast<date_value&>(result) = date_from_timestamp(timestamp);
	return result;
}

[[nodiscard]] constexpr timestamp_t timestamp_from_date_time(const date_time& value) noexcept
{
	timestamp_t timestamp = days_from_civil(value) * (60 * 60 * 24);
	timestamp += value.hour * (60 * 60);
	timestamp += value.minute * 60;
	timestamp += value.second;
	return timestamp;
}

[[nodiscard]] constexpr bool is_leap(uint32_t year) noexcept
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

[[nodiscard]] uint32_t get_number_of_days(uint32_t year, uint32_t month) noexcept
{
	static constexpr const uint8_t days_in_month[] { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	return month != 2 || !is_leap(year) ? days_in_month[month] : 29u;
}

[[maybe_unused]] constexpr uint16_t min_year = 2020;
[[maybe_unused]] constexpr auto max_date = static_cast<date_value>(
	date_time_from_timestamp((std::numeric_limits<timestamp_t>::max)()));
[[maybe_unused]] constexpr uint16_t max_year = max_date.year;
[[maybe_unused]] constexpr date_value min_date = { min_year, min_month, min_day };

constexpr bool operator<(date_value left, date_value right) noexcept
{
	if (left.year < right.year)
		return true;
	if (left.year > right.year)
		return false;
	if (left.month < right.month)
		return true;
	if (left.month > right.month)
		return false;
	
	return left.day < right.day;
}

constexpr bool operator>(date_value left, date_value right) noexcept
{
	return right < left;
}

namespace traits
{

MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(second)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(minute)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(hour)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(day)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(month)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(year)
MCUTL_DEFINE_INTEGRAL_MEMBER_CHECKER(timestamp)
	
} //namespace traits

} //namespace mcutl::datetime
