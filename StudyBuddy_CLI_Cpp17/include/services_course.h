
#ifndef STUDY_BUDDY_COURSE_SERVICE_H
#define STUDY_BUDDY_COURSE_SERVICE_H

#include "storage.h"

class CourseService {
public:
    explicit CourseService(Storage& s): store(s) {}

    bool add_course(int student_id, const std::string& course_code, std::string& err);
    bool remove_course(int student_id, const std::string& course_code, std::string& err);
    std::vector<std::string> list_courses(int student_id) const;
    bool enrolled(int student_id, const std::string& course_code) const;

private:
    Storage& store;
};

#endif // STUDY_BUDDY_COURSE_SERVICE_H
