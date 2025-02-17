#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>

// A simple helper class around timestamps and utilities from std::chrono

namespace dave::time {

using Clock = std::chrono::system_clock;

class DTime {
  public:
    DTime() : ts_(Clock::now()) {};
    explicit DTime(uint64_t millis) : ts_(std::chrono::milliseconds {millis}) {};
    uint64_t Millis() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   ts_.time_since_epoch()
        ).count();
    };
    uint64_t Micros() const {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   ts_.time_since_epoch()
        ).count();
    };
    uint64_t Seconds() const {
        return std::chrono::duration_cast<std::chrono::seconds>(ts_.time_since_epoch())
            .count();
    };
    std::string Iso8601() const {
        const std::time_t t = Clock::to_time_t(ts_);
        std::stringstream os;
        os << std::left;
        os << std::put_time(std::gmtime(&t), "%FT%T") << "." << std::right
           << std::setfill('0') << std::setw(3) << (Millis() % 1000) << std::setw(0)
           << "Z";
        return os.str();
    };
    auto Raw() const {
        return ts_;
    };

    bool SameMillis(const DTime &other) const {
        return Millis() == other.Millis();
    }

  private:
    std::chrono::time_point<Clock> ts_;
};

bool operator==(const DTime &l, const DTime &r);

}  // namespace dave::time
