#include <iostream>

std::string getCurrentTime() {
    // Gets the current timestamp, expressed as a point in time on the system clock
    auto now = std::chrono::system_clock::now();

    // Convert a point in time to time t
    auto now_c = std::chrono::system_clock::to_time_t(now);

    // Gets the microsecond portion of the current time
    auto now_us =
        std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

    // Convert the time portion to local time
    std::tm tm_now{};
    localtime_r(&now_c, &tm_now);

    // Creates a stream of strings to format the time
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S") << '.' << std::setw(6) << std::setfill('0')
        << now_us.count();

    // Return a string time
    return oss.str();
}
