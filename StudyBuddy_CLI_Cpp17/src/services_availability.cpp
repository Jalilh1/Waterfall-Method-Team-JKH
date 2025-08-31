
#include "services_availability.h"
#include "validation.h"
#include <algorithm>
#include <iostream>


bool AvailabilityService::add_availability(int student_id, int day, int start, int end, std::string& err) {
    if (!is_valid_day(day) || !is_valid_avail_range(start, end)) { err = "BAD_RANGE"; return false; }
    // Collect existing slots for same student/day
    std::vector<Availability> slots;
    for (const auto& a : store.availability) if (a.student_id == student_id && a.day == day) slots.push_back(a);
    // Add new slot
    slots.push_back(Availability{student_id, day, start, end});
    // Sort by start
    std::sort(slots.begin(), slots.end(), [](const Availability& x, const Availability& y){ return x.start < y.start; });
    // Merge
    std::vector<Availability> merged;
    for (const auto& cur : slots) {
        if (merged.empty() || merged.back().end < cur.start - 0) { // strictly non-adjacent
            merged.push_back(cur);
        } else {
            merged.back().end = std::max(merged.back().end, cur.end);
        }
    }
    // Remove old entries for that student/day
    store.availability.erase(std::remove_if(store.availability.begin(), store.availability.end(),
        [&](const Availability& a){ return a.student_id == student_id && a.day == day; }), store.availability.end());
    // Append merged
    store.availability.insert(store.availability.end(), merged.begin(), merged.end());
    store.save_availability();
    return true;
}

bool AvailabilityService::remove_availability_exact(int student_id, int day, int start, int end, std::string& msg) {
    bool removed = false;
    auto it = std::remove_if(store.availability.begin(), store.availability.end(),
        [&](const Availability& a){ 
            if (a.student_id == student_id && a.day == day && a.start == start && a.end == end) { removed = true; return true; }
            return false;
        });
    if (it != store.availability.end()) {
        store.availability.erase(it, store.availability.end());
        store.save_availability();
    }
    if (!removed) { msg = "No matching slot found"; }
    return removed;
}

std::vector<Availability> AvailabilityService::list_availability(int student_id) const {
    std::vector<Availability> out;
    for (const auto& a : store.availability) if (a.student_id == student_id) out.push_back(a);
    std::sort(out.begin(), out.end(), [](const Availability& x, const Availability& y){
        if (x.day != y.day) return x.day < y.day;
        return x.start < y.start;
    });
    return out;
}

bool AvailabilityService::within_availability(int student_id, int day, int start, int end) const {
    for (const auto& a : store.availability) {
        if (a.student_id == student_id && a.day == day) {
            if (a.start <= start && end <= a.end) return true;
        }
    }
    return false;
}
