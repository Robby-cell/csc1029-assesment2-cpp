#ifndef CSC_DATE_HPP
#define CSC_DATE_HPP

#include <chrono>
#include <cstdint>
#include <ostream>
#include <ratio>

namespace csc::date {
using Date = std::chrono::year_month_day;

template <typename Type, typename PrintMe>
concept Streamable = requires(Type& os, PrintMe p) {
  os << p;
  { os << p } -> std::same_as<Type&>;
};

class Time {
 public:
  // NOLINTBEGIN
  using millisecond = std::ratio<1>;

  using milliseconds_per_second =
      std::ratio_multiply<std::ratio<1000, 1>, millisecond>;

  using seconds_per_minute = std::ratio<60, 1>;
  using milliseconds_per_minute =
      std::ratio_multiply<seconds_per_minute, milliseconds_per_second>;

  using minutes_per_hour = std::ratio<60, 1>;
  using milliseconds_per_hour =
      std::ratio_multiply<minutes_per_hour, milliseconds_per_minute>;

  using hours_per_day = std::ratio<24, 1>;
  using milliseconds_per_day =
      std::ratio_multiply<hours_per_day, milliseconds_per_hour>;
  // NOLINTEND

  static_assert(millisecond::num == 1);
  static_assert(milliseconds_per_second::num == 1000);
  static_assert(milliseconds_per_minute::num == 60000);
  static_assert(milliseconds_per_hour::num == 3600000);
  static_assert(milliseconds_per_day::num == 86400000);

  explicit constexpr Time(uint32_t ms = 0U) : ms_{ms} {
    if (ms >= milliseconds_per_day::num) {
      throw std::out_of_range{"Time out of range"};
    }
  }

 private:
  using u8 = std::uint8_t;  // NOLINT
  struct TimeSplit {
   public:
    constexpr explicit TimeSplit(uint32_t ms) {
      days_ = ms / milliseconds_per_day::num;
      ms %= milliseconds_per_day::num;
      hours_ = ms / milliseconds_per_hour::num;
      ms %= milliseconds_per_hour::num;
      minutes_ = ms / milliseconds_per_minute::num;
      ms %= milliseconds_per_minute::num;
      seconds_ = ms / milliseconds_per_second::num;
      ms %= milliseconds_per_second::num;
      milliseconds_ = ms;
    }

    constexpr inline auto get_days() const noexcept -> u8 { return days_; }
    constexpr inline auto get_hours() const noexcept -> u8 { return hours_; }
    constexpr inline auto get_minutes() const noexcept -> u8 {
      return minutes_;
    }
    constexpr inline auto get_seconds() const noexcept -> u8 {
      return seconds_;
    }
    constexpr inline auto get_milliseconds() const noexcept -> u8 {
      return milliseconds_;
    }

    template <typename Stream>
      requires(Streamable<Stream, u8> and Streamable<Stream, const char*>)
    auto print(Stream& os) const -> Stream& {
      return os << static_cast<std::uint32_t>(days_) << "d "
                << static_cast<std::uint32_t>(hours_) << "h "
                << static_cast<std::uint32_t>(minutes_) << "m "
                << static_cast<std::uint32_t>(seconds_) << "s "
                << static_cast<std::uint32_t>(milliseconds_) << "ms";
    }

    friend inline auto operator<<(std::ostream& os,
                                  const TimeSplit& time) -> std::ostream& {
      return time.print(os);
    }
    friend inline auto operator<<(std::wostream& os,
                                  const TimeSplit& time) -> std::wostream& {
      return time.print(os);
    }

   private:
    u8 days_;
    u8 hours_;
    u8 minutes_;
    u8 seconds_;
    u8 milliseconds_;
  };
  friend inline auto operator<<(std::ostream& os,
                                const Time& time) -> std::ostream& {
    const TimeSplit my_time{time.ms_};
    return os << my_time.get_days() << "d " << my_time.get_hours() << "h "
              << my_time.get_minutes() << "m " << my_time.get_seconds() << "s "
              << my_time.get_milliseconds() << "ms";
  }

  uint32_t ms_{0};
};

class DateTime {
 public:
 private:
  std::chrono::year_month_day date_;
  Time time_;
};

// NOLINTBEGIN
struct Milli {
  std::uint32_t value_;
  constexpr operator Time() const noexcept { return Time{value_}; }
};
struct Second {
  std::uint32_t value_;
  constexpr operator Time() const noexcept {
    return Time{value_ *
                static_cast<std::uint32_t>(Time::milliseconds_per_second::num)};
  }
};
struct Minute {
  std::uint32_t value_;
  constexpr operator Time() const noexcept {
    return Time{value_ *
                static_cast<std::uint32_t>(Time::milliseconds_per_minute::num)};
  }
};
struct Hour {
  std::uint32_t value_;
  constexpr operator Time() const noexcept {
    return Time{value_ *
                static_cast<std::uint32_t>(Time::milliseconds_per_hour::num)};
  }
};
struct Day {
  std::uint32_t value_;
  consteval operator Time() const noexcept {
    return Time{value_ *
                static_cast<std::uint32_t>(Time::milliseconds_per_day::num)};
  }
};
// NOLINTEND

namespace literals {
consteval auto operator"" _ms(unsigned long long ms) noexcept -> Milli {
  return Milli(static_cast<std::int32_t>(ms));
}
consteval auto operator"" _s(unsigned long long s) noexcept -> Second {
  return Second(static_cast<std::uint32_t>(s) *
                Time::milliseconds_per_second::num);
}
consteval auto operator"" _m(unsigned long long mins) noexcept -> Minute {
  return Minute(static_cast<std::uint32_t>(mins) *
                Time::milliseconds_per_minute::num);
}
consteval auto operator"" _h(unsigned long long hours) noexcept -> Hour {
  return Hour(static_cast<std::uint32_t>(hours) *
              Time::milliseconds_per_hour::num);
}
consteval auto operator"" _d(unsigned long long days) noexcept -> Day {
  return Day(static_cast<std::uint32_t>(days) *
             Time::milliseconds_per_day::num);
}
}  // namespace literals

consteval auto operator/(Day&& day, Hour&& hour) noexcept -> Hour {
  return Hour{
      (day.value_ * static_cast<std::uint32_t>(Time::minutes_per_hour::num)) +
      hour.value_};
}
consteval auto operator/(Hour&& hour, Minute&& minute) noexcept -> Minute {
  return Minute{(hour.value_ *
                 static_cast<std::uint32_t>(Time::seconds_per_minute::num)) +
                minute.value_};
}
consteval auto operator/(Minute&& minute, Second&& second) noexcept -> Second {
  return Second{(minute.value_ * static_cast<std::uint32_t>(
                                     Time::milliseconds_per_minute::num)) +
                second.value_};
}
consteval auto operator/(Second&& second, Milli&& milli) noexcept -> Milli {
  return Milli{(second.value_ * static_cast<std::uint32_t>(
                                    Time::milliseconds_per_second::num)) +
               milli.value_};
}

}  // namespace csc::date

#endif  // CSC_DATE_HPP
