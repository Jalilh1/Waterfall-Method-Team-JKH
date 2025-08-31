
#include "services_match.h"
#include "validation.h"
#include <algorithm>
#include <iostream>

static bool overlap_pair(int s1, int e1, int s2, int e2, int& out_s, int& out_e) {
    out_s = std::max(s1, s2);
    out_e = std::min(e1, e2);
    return out_s < out_e;
}

std::vector<MatchCandidate> MatchService::suggest_matches(int student_id, const std::string& course_code, std::string& err) const {
    std::vector<MatchCandidate> result;
    if (!is_valid_course(course_code)) { err = "BAD_COURSE"; return result; }
    if (!courseSvc.enrolled(student_id, course_code)) { err = "NOT_ENROLLED"; return result; }

    // Build my slots by day
    std::vector<Availability> my;
    for (const auto& a : store.availability) if (a.student_id == student_id) my.push_back(a);

    // Classmates
    std::vector<int> others;
    auto range = store.enrollmentsByCourse.equal_range(course_code);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second != student_id) others.push_back(it->second);
    }
    std::sort(others.begin(), others.end());
    others.erase(std::unique(others.begin(), others.end()), others.end());

    for (int mate_id : others) {
        std::vector<Availability> mate;
        for (const auto& a : store.availability) if (a.student_id == mate_id) mate.push_back(a);
        std::vector<std::pair<int, std::vector<int>>> candidates; // (day, [hours])

        for (const auto& m1 : my) {
            for (const auto& m2 : mate) {
                if (m1.day != m2.day) continue;
                int s,e;
                if (overlap_pair(m1.start, m1.end, m2.start, m2.end, s, e)) {
                    std::vector<int> hours;
                    for (int h = s; h < e && h <= 23; ++h) hours.push_back(h);
                    if (!hours.empty()) candidates.push_back({m1.day, hours});
                }
            }
        }
        if (!candidates.empty()) {
            std::sort(candidates.begin(), candidates.end(), [](auto& a, auto& b){
                if (a.first != b.first) return a.first < b.first;
                return (!a.second.empty() && !b.second.empty()) ? a.second.front() < b.second.front() : a.second.size() < b.second.size();
            });
            MatchCandidate mc{mate_id, store.students.at(mate_id).name, candidates};
            result.push_back(std::move(mc));
        }
    }

    std::sort(result.begin(), result.end(), [](const MatchCandidate& a, const MatchCandidate& b){
        int ax = a.overlaps.empty() ? 10 : (a.overlaps.front().first * 24 + (a.overlaps.front().second.empty()? 24 : a.overlaps.front().second.front()));
        int bx = b.overlaps.empty() ? 10 : (b.overlaps.front().first * 24 + (b.overlaps.front().second.empty()? 24 : b.overlaps.front().second.front()));
        if (ax != bx) return ax < bx;
        return a.classmate_name < b.classmate_name;
    });
    return result;
}
