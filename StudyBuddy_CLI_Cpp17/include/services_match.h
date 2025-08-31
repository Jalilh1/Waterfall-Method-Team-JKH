
#ifndef STUDY_BUDDY_MATCH_SERVICE_H
#define STUDY_BUDDY_MATCH_SERVICE_H

#include "storage.h"
#include "services_course.h"

struct MatchCandidate {
    int classmate_id;
    std::string classmate_name;
    // list of (day, hours)
    std::vector<std::pair<int, std::vector<int>>> overlaps;
};

class MatchService {
public:
    MatchService(Storage& s, const CourseService& cs): store(s), courseSvc(cs) {}
    std::vector<MatchCandidate> suggest_matches(int student_id, const std::string& course_code, std::string& err) const;
private:
    Storage& store;
    const CourseService& courseSvc;
};

#endif // STUDY_BUDDY_MATCH_SERVICE_H
