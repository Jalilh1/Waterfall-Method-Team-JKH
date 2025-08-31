
#include "services_course.h"
#include "validation.h"
#include <algorithm>

bool CourseService::add_course(int student_id, const std::string& course_code, std::string& err) {
    if (!is_valid_course(course_code)) { err = "BAD_COURSE"; return false; }
    // duplicate check
    for (const auto& e : store.enrollments) {
        if (e.student_id == student_id && e.course_code == course_code) {
            err = "DUP_COURSE"; return false;
        }
    }
    Enrollment e{student_id, course_code};
    store.enrollments.push_back(e);
    store.enrollmentsByCourse.emplace(course_code, student_id);
    store.save_enrollments();
    return true;
}

bool CourseService::remove_course(int student_id, const std::string& course_code, std::string& err) {
    // check sessions not cancelled
    for (const auto& kv : store.sessions) {
        const auto& s = kv.second;
        if (s.course_code != course_code) continue;
        if (s.status == SessionStatus::CANCELLED) continue;
        bool involved = (s.organizer_id == student_id);
        if (!involved) {
            for (const auto& p : store.participants) {
                if (p.session_id == s.id && p.student_id == student_id) { involved = true; break; }
            }
        }
        if (involved) { err = "SESSIONS_EXIST"; return false; }
    }
    // erase enrollment
    bool removed = false;
    store.enrollments.erase(std::remove_if(store.enrollments.begin(), store.enrollments.end(),
        [&](const Enrollment& e){ 
            if (e.student_id == student_id && e.course_code == course_code) { removed = true; return true; }
            return false;
        }), store.enrollments.end());
    if (!removed) { err = "COURSE_NOT_ENROLLED"; return false; }

    // rebuild enrollmentsByCourse (simple & safe)
    store.recompute_indices();
    store.save_enrollments();
    return true;
}

std::vector<std::string> CourseService::list_courses(int student_id) const {
    std::vector<std::string> out;
    for (const auto& e : store.enrollments) if (e.student_id == student_id) out.push_back(e.course_code);
    std::sort(out.begin(), out.end());
    return out;
}

bool CourseService::enrolled(int student_id, const std::string& course_code) const {
    for (const auto& e : store.enrollments) if (e.student_id == student_id && e.course_code == course_code) return true;
    return false;
}
