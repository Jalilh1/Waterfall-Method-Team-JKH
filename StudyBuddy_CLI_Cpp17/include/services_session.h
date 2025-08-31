
#ifndef STUDY_BUDDY_SESSION_SERVICE_H
#define STUDY_BUDDY_SESSION_SERVICE_H

#include "storage.h"
#include "services_course.h"
#include "services_availability.h"

class SessionService {
public:
    SessionService(Storage& s, const CourseService& cs, const AvailabilityService& as)
        : store(s), courseSvc(cs), availSvc(as) {}

    bool schedule_session(int organizer_id, const std::string& course_code, int day, int start,
                          const std::vector<int>& invitees, std::string& err);

    bool confirm_session(int actor_id, int session_id, std::string& err);
    bool cancel_session(int actor_id, int session_id, const std::string& reason, std::string& err);

    std::vector<Session> list_sessions_for(int student_id) const;
    std::vector<Session> list_sessions_by_status_for(int student_id, SessionStatus status) const;
    std::vector<Session> list_pending_invitations_for(int student_id) const;

    bool has_conflict(int student_id, int day, int start) const;

private:
    Storage& store;
    const CourseService& courseSvc;
    const AvailabilityService& availSvc;
};

#endif // STUDY_BUDDY_SESSION_SERVICE_H
