
#include "services_session.h"
#include "validation.h"
#include <algorithm>
#include <iostream>

bool SessionService::has_conflict(int student_id, int day, int start) const {
    for (const auto& kv : store.sessions) {
        const Session& s = kv.second;
        if (s.status != SessionStatus::CONFIRMED) continue;
        if (s.day == day && s.start == start) {
            if (s.organizer_id == student_id) return true;
            for (const auto& p : store.participants) {
                if (p.session_id == s.id && p.student_id == student_id && p.confirmed) return true;
            }
        }
    }
    return false;
}

bool SessionService::schedule_session(int organizer_id, const std::string& course_code, int day, int start,
                          const std::vector<int>& invitees, std::string& err) {
    if (!is_valid_course(course_code)) { err = "BAD_COURSE"; return false; }
    if (!is_valid_day(day) || !(0 <= start && start <= 23)) { err = "BAD_TIME"; return false; }
    if (!courseSvc.enrolled(organizer_id, course_code)) { err = "NOT_ENROLLED_ORG"; return false; }
    if (!availSvc.within_availability(organizer_id, day, start, start+1)) { err = "OUTSIDE_AVAIL_ORG"; return false; }
    if (has_conflict(organizer_id, day, start)) { err = "ORG_CONFLICT"; return false; }

    std::vector<int> uniq;
    std::unordered_set<int> seen;
    for (int uid : invitees) {
        if (uid == organizer_id) continue;
        if (!store.students.count(uid)) { err = "INV_ID"; return false; }
        if (!courseSvc.enrolled(uid, course_code)) { err = "INV_NOT_ENROLLED"; return false; }
        if (seen.insert(uid).second) uniq.push_back(uid);
    }
    if (uniq.empty()) { err = "NO_INVITEES"; return false; }

    int sid = store.nextSessionId++;
    Session s;
    s.id = sid; s.course_code = course_code; s.day = day; s.start = start; s.duration = 1;
    s.organizer_id = organizer_id; s.status = SessionStatus::PROPOSED;
    store.sessions[sid] = s;

    store.participants.push_back(SessionParticipant{sid, organizer_id, false});
    for (int uid : uniq) store.participants.push_back(SessionParticipant{sid, uid, false});

    store.save_sessions();
    store.save_participants();
    return true;
}

bool SessionService::confirm_session(int actor_id, int session_id, std::string& err) {
    auto it = store.sessions.find(session_id);
    if (it == store.sessions.end()) { err = "NO_SESSION"; return false; }
    Session& s = it->second;
    if (s.status == SessionStatus::CANCELLED) { err = "CANCELLED"; return false; }
    // Find participant
    bool found = false;
    for (auto& p : store.participants) {
        if (p.session_id == session_id && p.student_id == actor_id) {
            found = true;
            if (has_conflict(actor_id, s.day, s.start)) { err = "TIME_CONFLICT"; return false; }
            if (!availSvc.within_availability(actor_id, s.day, s.start, s.start+1)) { err = "OUTSIDE_AVAIL"; return false; }
            p.confirmed = true;
            break;
        }
    }
    if (!found) { err = "NOT_PARTICIPANT"; return false; }
    // Check if all confirmed
    bool allConfirmed = true;
    for (const auto& p : store.participants) {
        if (p.session_id == session_id && !p.confirmed) { allConfirmed = false; break; }
    }
    if (allConfirmed) {
        s.status = SessionStatus::CONFIRMED;
        store.save_sessions();
    }
    store.save_participants();
    return true;
}

bool SessionService::cancel_session(int actor_id, int session_id, const std::string& reason, std::string& err) {
    auto it = store.sessions.find(session_id);
    if (it == store.sessions.end()) { err = "NO_SESSION"; return false; }
    Session& s = it->second;
    bool isParticipant = false;
    if (s.organizer_id == actor_id) isParticipant = true;
    for (const auto& p : store.participants) if (p.session_id == session_id && p.student_id == actor_id) { isParticipant = true; break; }
    if (!isParticipant) { err = "NOT_PARTICIPANT"; return false; }
    s.status = SessionStatus::CANCELLED;
    s.cancel_reason = reason;
    store.save_sessions();
    return true;
}

std::vector<Session> SessionService::list_sessions_for(int student_id) const {
    std::vector<Session> out;
    for (const auto& kv : store.sessions) {
        const Session& s = kv.second;
        if (s.organizer_id == student_id) out.push_back(s);
        else {
            for (const auto& p : store.participants) {
                if (p.session_id == s.id && p.student_id == student_id) { out.push_back(s); break; }
            }
        }
    }
    std::sort(out.begin(), out.end(), [](const Session& a, const Session& b){
        if (a.status != b.status) return static_cast<int>(a.status) < static_cast<int>(b.status);
        if (a.day != b.day) return a.day < b.day;
        if (a.start != b.start) return a.start < b.start;
        return a.id < b.id;
    });
    return out;
}

std::vector<Session> SessionService::list_sessions_by_status_for(int student_id, SessionStatus status) const {
    std::vector<Session> out;
    for (const auto& s : list_sessions_for(student_id)) if (s.status == status) out.push_back(s);
    return out;
}

std::vector<Session> SessionService::list_pending_invitations_for(int student_id) const {
    std::vector<Session> out;
    for (const auto& p : store.participants) {
        if (p.student_id == student_id && !p.confirmed) {
            auto it = store.sessions.find(p.session_id);
            if (it != store.sessions.end() && it->second.status == SessionStatus::PROPOSED)
                out.push_back(it->second);
        }
    }
    std::sort(out.begin(), out.end(), [](const Session& a, const Session& b){
        if (a.day != b.day) return a.day < b.day;
        if (a.start != b.start) return a.start < b.start;
        return a.id < b.id;
    });
    return out;
}
