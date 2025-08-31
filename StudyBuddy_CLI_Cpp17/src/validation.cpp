
#include "validation.h"
#include <regex>

bool is_valid_email(const std::string& email) {
    // Simple heuristic: something@something.something
    static const std::regex re(R"(^[^@\s]+@[^@\s]+\.[^@\s]+$)");
    return std::regex_match(email, re);
}

bool is_valid_course(const std::string& code) {
    static const std::regex re(R"(^[A-Z]{2,5} [0-9]{3,4}$)");
    return std::regex_match(code, re);
}

bool is_valid_day(int d) { return 0 <= d && d <= 6; }
bool is_valid_hour(int h) { return 0 <= h && h <= 24; }
bool is_valid_avail_range(int start, int end) { return is_valid_hour(start) && is_valid_hour(end) && start < end; }
