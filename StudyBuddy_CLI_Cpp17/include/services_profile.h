
#ifndef STUDY_BUDDY_PROFILE_SERVICE_H
#define STUDY_BUDDY_PROFILE_SERVICE_H

#include "storage.h"
#include <optional>

class ProfileService {
public:
    explicit ProfileService(Storage& s): store(s) {}

    std::optional<int> create_profile(const std::string& name, const std::string& email, const std::optional<std::string>& passcode);
    bool edit_profile_name(int student_id, const std::string& new_name);
    bool edit_profile_email(int student_id, const std::string& new_email);
private:
    Storage& store;
};

#endif // STUDY_BUDDY_PROFILE_SERVICE_H
