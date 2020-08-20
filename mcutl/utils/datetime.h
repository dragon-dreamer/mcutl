#pragma once

#include <stdint.h>
#include <limits>

#include "mcutl/utils/member_checker.h"

namespace mcutl::datetime
{

using timestamp_t = uint32_t;

using year_type = uint16_t;
using month_type = uint8_t;
using day_type = uint8_t;
using hour_type = uint8_t;
using minute_type = uint8_t;
using second_type = uint8_t;

[[maybe_unused]] constexpr hour_type max_hour = 23;
[[maybe_unused]] constexpr minute_type max_minute = 59;
[[maybe_unused]] constexpr second_type max_second = 59;
[[maybe_unused]] constexpr hour_type min_hour = 0;
[[maybe_unused]] constexpr minute_type min_minute = 0;
[[maybe_unused]] constexpr second_type min_second = 0;

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

enum class month : month_type
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

[[maybe_unused]] constexpr month_type min_month = static_cast<month_type>(month::january);
[[maybe_unused]] constexpr month_type max_month = static_cast<month_type>(month::december);
[[maybe_unused]] constexpr day_type min_day = 1;
[[maybe_unused]] constexpr day_type max_day = 31;

struct date_value
{
	year_type year;
	month_type month;
	day_type day;
};

struct time_value
{
	hour_type hour;
	minute_type minute;
	second_type second;
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

[[nodiscard]] constexpr bool is_leap(year_type year) noexcept
{
	return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

[[nodiscard]] day_type get_number_of_days(year_type year, month_type month) noexcept
{
	static constexpr const uint8_t days_in_month[] { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	return month != 2 || !is_leap(year) ? days_in_month[month] : 29u;
}

[[maybe_unused]] constexpr year_type min_year = 2020;
[[maybe_unused]] constexpr auto max_date = static_cast<date_value>(
	date_time_from_timestamp((std::numeric_limits<timestamp_t>::max)()));
[[maybe_unused]] constexpr year_type max_year = max_date.year;
[[maybe_unused]] constexpr date_value min_date = { min_year, min_month, min_day };

[[nodiscard]] constexpr bool operator<(date_value left, date_value right) noexcept
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

[[nodiscard]] constexpr bool operator>(date_value left, date_value right) noexcept
{
	return right < left;
}

[[nodiscard]] constexpr bool operator<=(date_value left, date_value right) noexcept
{
	return !(left > right);
}

[[nodiscard]] constexpr bool operator>=(date_value left, date_value right) noexcept
{
	return !(left < right);
}

[[nodiscard]] constexpr bool operator<(time_value left, time_value right) noexcept
{
	if (left.hour < right.hour)
		return true;
	if (left.hour > right.hour)
		return false;
	if (left.minute < right.minute)
		return true;
	if (left.minute > right.minute)
		return false;
	
	return left.second < right.second;
}

[[nodiscard]] constexpr bool operator>(time_value left, time_value right) noexcept
{
	return right < left;
}

[[nodiscard]] constexpr bool operator<=(time_value left, time_value right) noexcept
{
	return !(left > right);
}

[[nodiscard]] constexpr bool operator>=(time_value left, time_value right) noexcept
{
	return !(left < right);
}

[[nodiscard]] constexpr bool operator<(const date_time& left, const date_time& right) noexcept
{
	if (static_cast<const date_value&>(left) < static_cast<const date_value&>(right))
		return true;
	
	if (left.day > right.day)
		return false;
	
	return static_cast<const time_value&>(left) < static_cast<const time_value&>(right);
}

[[nodiscard]] constexpr bool operator>(const date_time& left, const date_time& right) noexcept
{
	return right < left;
}

[[nodiscard]] constexpr bool operator<=(const date_time& left, const date_time& right) noexcept
{
	return !(left > right);
}

[[nodiscard]] constexpr bool operator>=(const date_time& left, const date_time& right) noexcept
{
	return !(left < right);
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
