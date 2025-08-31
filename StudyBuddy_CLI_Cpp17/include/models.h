
#ifndef STUDY_BUDDY_MODELS_H
#define STUDY_BUDDY_MODELS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <cstddef>

enum class SessionStatus { PROPOSED, CONFIRMED, CANCELLED };

struct Student {
    int id{};
    std::string name;
    std::string email;
    std::optional<std::size_t> pass_hash;
};

struct Enrollment {
    int student_id{};
    std::string course_code;
};

struct Availability {
    int student_id{};
    int day{};   // 0..6
    int start{}; // 0..23
    int end{};   // 1..24, end > start
};

struct Session {
    int id{};
    std::string course_code;
    int day{};
    int start{};
    int duration{1};
    int organizer_id{};
    SessionStatus status{SessionStatus::PROPOSED};
    std::optional<std::string> cancel_reason;
};

struct SessionParticipant {
    int session_id{};
    int student_id{};
    bool confirmed{false};
};

#endif // STUDY_BUDDY_MODELS_H
