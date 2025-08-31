
#ifndef STUDY_BUDDY_AVAILABILITY_SERVICE_H
#define STUDY_BUDDY_AVAILABILITY_SERVICE_H

#include "storage.h"

class AvailabilityService {
public:
    explicit AvailabilityService(Storage& s): store(s) {}

    bool add_availability(int student_id, int day, int start, int end, std::string& err);
    bool remove_availability_exact(int student_id, int day, int start, int end, std::string& msg);
    std::vector<Availability> list_availability(int student_id) const;
    bool within_availability(int student_id, int day, int start, int end) const;

private:
    Storage& store;
};

#endif // STUDY_BUDDY_AVAILABILITY_SERVICE_H
