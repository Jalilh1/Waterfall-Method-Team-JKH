
#include "services_profile.h"
#include "validation.h"
#include <iostream>
#include <functional>

std::optional<int> ProfileService::create_profile(const std::string& name, const std::string& email, const std::optional<std::string>& passcode) {
    if (!is_valid_email(email)) {
        std::cerr << "[ERROR] BAD_EMAIL: " << email << "\n";
        return std::nullopt;
    }
    if (store.studentsByEmail.count(email)) {
        std::cerr << "[ERROR] DUP_EMAIL: " << email << "\n";
        return std::nullopt;
    }
    Student s;
    s.id = store.nextStudentId++;
    s.name = name;
    s.email = email;
    if (passcode && !passcode->empty()) {
        std::hash<std::string> hasher;
        s.pass_hash = hasher(*passcode);
    }
    store.students[s.id] = s;
    store.studentsByEmail[s.email] = s.id;
    store.save_students();
    std::cout << "Profile created: id=" << s.id << "\n";
    return s.id;
}

bool ProfileService::edit_profile_name(int student_id, const std::string& new_name) {
    auto it = store.students.find(student_id);
    if (it == store.students.end()) {
        std::cerr << "[ERROR] NO_STUDENT\n";
        return false;
    }
    it->second.name = new_name;
    store.save_students();
    return true;
}

bool ProfileService::edit_profile_email(int student_id, const std::string& new_email) {
    if (!is_valid_email(new_email)) {
        std::cerr << "[ERROR] BAD_EMAIL\n";
        return false;
    }
    if (store.studentsByEmail.count(new_email)) {
        std::cerr << "[ERROR] DUP_EMAIL\n";
        return false;
    }
    auto it = store.students.find(student_id);
    if (it == store.students.end()) {
        std::cerr << "[ERROR] NO_STUDENT\n";
        return false;
    }
    store.studentsByEmail.erase(it->second.email);
    it->second.email = new_email;
    store.studentsByEmail[new_email] = student_id;
    store.save_students();
    return true;
}
