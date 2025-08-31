
#ifndef STUDY_BUDDY_STORAGE_H
#define STUDY_BUDDY_STORAGE_H

#include "models.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <filesystem>

class Storage {
public:
    // In-memory state
    std::unordered_map<int, Student> students;
    std::unordered_map<std::string, int> studentsByEmail; // email->id

    std::vector<Enrollment> enrollments;
    std::unordered_multimap<std::string, int> enrollmentsByCourse; // course->student_id

    std::vector<Availability> availability;
    
    std::unordered_map<int, Session> sessions;
    std::vector<SessionParticipant> participants;

    int nextStudentId{1};
    int nextSessionId{1};

    // Files
    std::filesystem::path dataDir;
    std::filesystem::path studentsFile;
    std::filesystem::path enrollmentsFile;
    std::filesystem::path availabilityFile;
    std::filesystem::path sessionsFile;
    std::filesystem::path participantsFile;

    Storage(const std::string& data_dir = "data");

    void load_all();
    void save_students();
    void save_enrollments();
    void save_availability();
    void save_sessions();
    void save_participants();

    // Helpers
    void recompute_indices();
    void set_next_ids();
    void ensure_files();

private:
    void atomic_write(const std::filesystem::path& path, const std::vector<std::string>& lines);
};

#endif // STUDY_BUDDY_STORAGE_H
