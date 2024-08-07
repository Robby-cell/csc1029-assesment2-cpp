#ifndef CSC_DATE_HPP
#define CSC_DATE_HPP

#include <chrono>
#include <cstdint>
#include <format>
#include <ostream>
#include <ratio>

#include "csc/core.h"

namespace csc::date {
using Date = std::chrono::year_month_day;

template <typename Type, typename PrintMe>
concept Streamable = requires(Type& os, PrintMe p) {
  os << p;
  { os << p } -> std::same_as<Type&>;
};

class Time {
 public:
  MAYBE_CONSTEXPR inline Time(std::uint32_t ms = 0U) : ms_{ms} {  // NOLINT
    if (ms >= milliseconds_per_day::num) {
      throw std::out_of_range{"Time out of range"};
    }
  }

  MAYBE_CONSTEXPR inline Time(std::size_t hour, std::size_t minute,
                              std::size_t second, std::size_t milli) noexcept {
    ms_ = (hour * milliseconds_per_hour::num) +
          (minute * milliseconds_per_minute::num) +
          (second * milliseconds_per_second::num) + milli;
  }

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

  inline auto to_string() const noexcept -> std::string {
    return TimeSplit{ms_}.to_string();
  }

 private:
  using u8 = std::uint8_t;  // NOLINT
  struct TimeSplit {
   public:
    MAYBE_CONSTEXPR explicit TimeSplit(uint32_t ms) {
      hours_ = ms / milliseconds_per_hour::num;
      ms %= milliseconds_per_hour::num;
      minutes_ = ms / milliseconds_per_minute::num;
      ms %= milliseconds_per_minute::num;
      seconds_ = ms / milliseconds_per_second::num;
      ms %= milliseconds_per_second::num;
      milliseconds_ = ms;
    }

    MAYBE_CONSTEXPR inline auto get_hours() const noexcept -> u8 {
      return hours_;
    }
    MAYBE_CONSTEXPR inline auto get_minutes() const noexcept -> u8 {
      return minutes_;
    }
    MAYBE_CONSTEXPR inline auto get_seconds() const noexcept -> u8 {
      return seconds_;
    }
    MAYBE_CONSTEXPR inline auto get_milliseconds() const noexcept -> u8 {
      return milliseconds_;
    }

    template <typename Stream>
      requires(Streamable<Stream, u8> and Streamable<Stream, const char*>)
    auto print(Stream& os) const -> Stream& {
      return os << static_cast<std::uint32_t>(hours_) << "h "
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

    MAYBE_CONSTEXPR inline auto total_time_ms() const noexcept
        -> std::uint32_t {
      return (hours_ * milliseconds_per_hour::num) +
             (minutes_ * milliseconds_per_minute::num) +
             (seconds_ * milliseconds_per_second::num) + milliseconds_;
    }

    inline auto to_string() const noexcept -> std::string {
      return std::format("{}h {}min {}s {}ms", hours_, minutes_, seconds_,
                         milliseconds_);
    }

   private:
    u8 hours_;
    u8 minutes_;
    u8 seconds_;
    u8 milliseconds_;
  };

 public:
  friend inline auto operator<(const Time& self,
                               const Time& other) noexcept -> bool {
    return self.ms_ < other.ms_;
  }
  friend inline auto operator<=(const Time& self,
                                const Time& other) noexcept -> bool {
    return self.ms_ <= other.ms_;
  }
  friend inline auto operator>(const Time& self,
                               const Time& other) noexcept -> bool {
    return self.ms_ > other.ms_;
  }
  friend inline auto operator>=(const Time& self,
                                const Time& other) noexcept -> bool {
    return self.ms_ >= other.ms_;
  }
  friend inline auto operator==(const Time& self,
                                const Time& other) noexcept -> bool {
    return self.ms_ == other.ms_;
  }

 private:
  friend inline auto operator<<(std::ostream& os,
                                const Time& time) -> std::ostream& {
    const TimeSplit my_time{time.ms_};
    return os << my_time.get_hours() << "h " << my_time.get_minutes() << "m "
              << my_time.get_seconds() << "s " << my_time.get_milliseconds()
              << "ms";
  }

  uint32_t ms_{0};
};

class DateTime {
 public:
  MAYBE_CONSTEXPR inline DateTime(std::chrono::year_month_day date, Time time)
      : date_{date}, time_{time} {}

  friend inline auto operator<<(std::ostream& os,
                                const DateTime& date_time) -> std::ostream& {
    return os << date_time.date_ << ' ' << date_time.time_;
  }

  friend auto operator<(const DateTime& self,
                        const DateTime& other) noexcept -> bool {
    return (self.date_ < other.date_) and (self.time_ < other.time_);
  }
  friend auto operator<=(const DateTime& self,
                         const DateTime& other) noexcept -> bool {
    return (self.date_ <= other.date_) and (self.time_ <= other.time_);
  }
  friend auto operator>(const DateTime& self,
                        const DateTime& other) noexcept -> bool {
    return (self.date_ > other.date_) and (self.time_ > other.time_);
  }
  friend auto operator>=(const DateTime& self,
                         const DateTime& other) noexcept -> bool {
    return (self.date_ >= other.date_) and (self.time_ >= other.time_);
  }
  friend auto operator==(const DateTime& self,
                         const DateTime& other) noexcept -> bool {
    return (self.date_ == other.date_) and (self.time_ == other.time_);
  }

  inline auto to_string() const noexcept -> std::string {
    return std::format("{:%F} {}", date_, time_.to_string());
  }

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
// NOLINTEND

namespace literals {
consteval auto operator"" _ms(unsigned long long ms) noexcept -> Milli {
  return Milli(static_cast<std::int32_t>(ms));
}
consteval auto operator"" _s(unsigned long long s) noexcept -> Second {
  return Second(static_cast<std::uint32_t>(s));
}
consteval auto operator"" _m(unsigned long long mins) noexcept -> Minute {
  return Minute(static_cast<std::uint32_t>(mins));
}
consteval auto operator"" _h(unsigned long long hours) noexcept -> Hour {
  return Hour(static_cast<std::uint32_t>(hours));
}
}  // namespace literals

MAYBE_CONSTEXPR auto operator/(Hour&& hour,
                               Minute&& minute) noexcept -> Minute {
  return Minute{static_cast<std::uint32_t>(
      (hour.value_ * Time::minutes_per_hour::num) + minute.value_)};
}
MAYBE_CONSTEXPR auto operator/(Minute&& minute,
                               Second&& second) noexcept -> Second {
  return Second{static_cast<std::uint32_t>(
      (minute.value_ * Time::seconds_per_minute::num) + second.value_)};
}
MAYBE_CONSTEXPR auto operator/(Second&& second,
                               Milli&& milli) noexcept -> Milli {
  return Milli{static_cast<std::uint32_t>(
      (second.value_ * Time::milliseconds_per_second::num) + milli.value_)};
}
MAYBE_CONSTEXPR auto operator/(Hour&& hour,
                               Second&& second) noexcept -> Second {
  return Second{static_cast<std::uint32_t>(
      (hour.value_ * Time::milliseconds_per_hour::num /
       Time::milliseconds_per_second::num) +
      second.value_)};
}
MAYBE_CONSTEXPR auto operator/(Minute&& minute,
                               Milli&& milli) noexcept -> Milli {
  return Milli{static_cast<std::uint32_t>(
      (minute.value_ * Time::milliseconds_per_minute::num) + milli.value_)};
}
MAYBE_CONSTEXPR auto operator/(Hour&& hour, Milli&& milli) noexcept -> Milli {
  return Milli{static_cast<std::uint32_t>(
      (hour.value_ * Time::milliseconds_per_hour::num) + milli.value_)};
}

}  // namespace csc::date

#endif  // CSC_DATE_HPP
