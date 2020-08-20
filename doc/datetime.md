# Date-time and timestamp helper definitions and functions
The library provides several helper definitions and functions to work with dates and times, as well as timestamps. They are defined in the `mcutl/utils/datetime.h` file in the `mcutl::datetime` namespace. These definitions are used by the [RTC support layer](rtc.md).

## Timestamp
```cpp
using timestamp_t = uint32_t;
```
The timestamp of the library is defined as an unsigned 32-bit integer.

## Date and time types
The following types are used to represent the date and time:
```cpp
using year_type = uint16_t;
using month_type = uint8_t;
using day_type = uint8_t;
using hour_type = uint8_t;
using minute_type = uint8_t;
using second_type = uint8_t;

enum class weekday : uint8_t
{
	sunday,
	monday,
	tuesday,
	wednesday,
	thursday,
	friday,
	saturday
};

enum class month : month_type
{
	january = 1,
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
	december
};

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
```

## Valid ranges for types
```cpp
constexpr hour_type max_hour = 23;
constexpr minute_type max_minute = 59;
constexpr second_type max_second = 59;
constexpr hour_type min_hour = 0;
constexpr minute_type min_minute = 0;
constexpr second_type min_second = 0;
constexpr month_type min_month = static_cast<month_type>(month::january);
constexpr month_type max_month = static_cast<month_type>(month::december);
constexpr day_type min_day = 1;
constexpr day_type max_day = 31;
constexpr year_type min_year = 2020;
constexpr auto max_date = static_cast<date_value>(
	date_time_from_timestamp((std::numeric_limits<timestamp_t>::max)()));
constexpr year_type max_year = max_date.year;
constexpr date_value min_date = { min_year, min_month, min_day };
```

## Epoch
The beginning of the epoch for the library is `03/01/2000` (`mm/dd/yyyy`). The maximum possible date is `04/07/2136` (`mm/dd/yy`).

## civil_from_days, days_from_civil
```cpp
constexpr date_value civil_from_days(uint32_t days_from_epoch) noexcept;
constexpr uint32_t days_from_civil(date_value value) noexcept;
```
These two functions are based on the [https://howardhinnant.github.io/date_algorithms.html](https://howardhinnant.github.io/date_algorithms.html) page. They are used to convert the number of days from the beginning of the epoch to the date value and back.

## weekday_from_days
```cpp
constexpr weekday weekday_from_days(uint32_t days_from_epoch) noexcept;
```
Returns the `weekday` of the day with number `days_from_epoch` from the beginning of the epoch.

## time_from_timestamp, date_from_timestamp, date_time_from_timestamp
```cpp
constexpr time_value time_from_timestamp(timestamp_t timestamp) noexcept;
constexpr date_value date_from_timestamp(timestamp_t timestamp) noexcept;
constexpr date_time date_time_from_timestamp(timestamp_t timestamp) noexcept;
```
These functions convert the timestamp value to the `time_value`, `date_value` or `date_time`.

## timestamp_from_date_time
```cpp
constexpr timestamp_t timestamp_from_date_time(const date_time& value) noexcept;
```
This function converts `date_time` to the timestamp value.

## is_leap
```cpp
constexpr bool is_leap(year_type year) noexcept;
```
This function returns `true` if the `year` is leap.

## get_number_of_days
```cpp
day_type get_number_of_days(year_type year, month_type month) noexcept;
```
Returns number of days in a month of a year. Takes leap years into consideration.

## operator>, operator<, operator>=, operator<=
There are constexpr comparison operators available for `time_value`, `date_value` and `date_time` types.
