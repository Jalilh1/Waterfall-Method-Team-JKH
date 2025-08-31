
#include "storage.h"
#include "csv.h"
#include <fstream>
#include <iostream>

using std::string;
namespace fs = std::filesystem;

Storage::Storage(const std::string& data_dir) {
    dataDir = fs::path(data_dir);
    studentsFile = dataDir / "students.csv";
    enrollmentsFile = dataDir / "enrollments.csv";
    availabilityFile = dataDir / "availability.csv";
    sessionsFile = dataDir / "sessions.csv";
    participantsFile = dataDir / "session_participants.csv";
    ensure_files();
    load_all();
}

void Storage::ensure_files() {
    try {
        fs::create_directories(dataDir);
        auto ensure = [](const fs::path& p){
            if (!fs::exists(p)) {
                std::ofstream ofs(p); ofs.close();
            }
        };
        ensure(studentsFile);
        ensure(enrollmentsFile);
        ensure(availabilityFile);
        ensure(sessionsFile);
        ensure(participantsFile);
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Failed to ensure data directory/files: " << e.what() << "\n";
    }
}

void Storage::atomic_write(const fs::path& path, const std::vector<std::string>& lines) {
    fs::path tmp = path; tmp += ".tmp";
    std::ofstream ofs(tmp, std::ios::binary);
    if (!ofs) { std::cerr << "[ERROR] IO_WRITE: cannot open temp for " << path << "\n"; return; }
    for (size_t i = 0; i < lines.size(); ++i) {
        ofs << lines[i];
        if (i + 1 < lines.size()) ofs << "\n";
    }
    ofs.flush();
    ofs.close();
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        // try replace
        fs::remove(path, ec);
        fs::rename(tmp, path, ec);
        if (ec) std::cerr << "[ERROR] IO_RENAME: " << ec.message() << "\n";
    }
}

void Storage::load_all() {
    students.clear(); studentsByEmail.clear();
    enrollments.clear(); enrollmentsByCourse.clear();
    availability.clear();
    sessions.clear(); participants.clear();

    // students.csv: id,name,email,pass_hash?
    {
        std::ifstream ifs(studentsFile);
        std::string line; int ln=0;
        while (std::getline(ifs, line)) {
            ++ln;
            if (line.empty()) continue;
            auto fields = csv::parse_line(line);
            if (fields.empty()) { std::cerr << "Warning: malformed line " << ln << " in students.csv\n"; continue; }
            if (fields.size() < 3) { std::cerr << "Warning: short line " << ln << " in students.csv\n"; continue; }
            try {
                Student s;
                s.id = std::stoi(fields[0]);
                s.name = fields[1];
                s.email = fields[2];
                if (fields.size() >= 4 && !fields[3].empty()) {
                    s.pass_hash = static_cast<std::size_t>(std::stoull(fields[3]));
                }
                students[s.id] = s;
                studentsByEmail[s.email] = s.id;
            } catch (...) { std::cerr << "Warning: bad data at line " << ln << " in students.csv\n"; }
        }
    }
    // enrollments.csv: student_id,course_code
    {
        std::ifstream ifs(enrollmentsFile);
        std::string line; int ln=0;
        while (std::getline(ifs, line)) {
            ++ln; if (line.empty()) continue;
            auto fields = csv::parse_line(line);
            if (fields.size() < 2) { std::cerr << "Warning: malformed line " << ln << " in enrollments.csv\n"; continue; }
            try {
                Enrollment e;
                e.student_id = std::stoi(fields[0]);
                e.course_code = fields[1];
                enrollments.push_back(e);
                enrollmentsByCourse.emplace(e.course_code, e.student_id);
            } catch (...) { std::cerr << "Warning: bad data at line " << ln << " in enrollments.csv\n"; }
        }
    }
    // availability.csv: student_id,day,start,end
    {
        std::ifstream ifs(availabilityFile);
        std::string line; int ln=0;
        while (std::getline(ifs, line)) {
            ++ln; if (line.empty()) continue;
            auto fields = csv::parse_line(line);
            if (fields.size() < 4) { std::cerr << "Warning: malformed line " << ln << " in availability.csv\n"; continue; }
            try {
                Availability a;
                a.student_id = std::stoi(fields[0]);
                a.day = std::stoi(fields[1]);
                a.start = std::stoi(fields[2]);
                a.end = std::stoi(fields[3]);
                availability.push_back(a);
            } catch (...) { std::cerr << "Warning: bad data at line " << ln << " in availability.csv\n"; }
        }
    }
    // sessions.csv: id,course_code,day,start,duration,organizer_id,status,cancel_reason
    {
        std::ifstream ifs(sessionsFile);
        std::string line; int ln=0;
        while (std::getline(ifs, line)) {
            ++ln; if (line.empty()) continue;
            auto fields = csv::parse_line(line);
            if (fields.size() < 7) { std::cerr << "Warning: malformed line " << ln << " in sessions.csv\n"; continue; }
            try {
                Session s;
                s.id = std::stoi(fields[0]);
                s.course_code = fields[1];
                s.day = std::stoi(fields[2]);
                s.start = std::stoi(fields[3]);
                s.duration = std::stoi(fields[4]);
                s.organizer_id = std::stoi(fields[5]);
                std::string st = fields[6];
                if (st == "PROPOSED") s.status = SessionStatus::PROPOSED;
                else if (st == "CONFIRMED") s.status = SessionStatus::CONFIRMED;
                else s.status = SessionStatus::CANCELLED;
                if (fields.size() >= 8 && !fields[7].empty()) s.cancel_reason = fields[7];
                sessions[s.id] = s;
            } catch (...) { std::cerr << "Warning: bad data at line " << ln << " in sessions.csv\n"; }
        }
    }
    // session_participants.csv: session_id,student_id,confirmed
    {
        std::ifstream ifs(participantsFile);
        std::string line; int ln=0;
        while (std::getline(ifs, line)) {
            ++ln; if (line.empty()) continue;
            auto fields = csv::parse_line(line);
            if (fields.size() < 3) { std::cerr << "Warning: malformed line " << ln << " in session_participants.csv\n"; continue; }
            try {
                SessionParticipant p;
                p.session_id = std::stoi(fields[0]);
                p.student_id = std::stoi(fields[1]);
                p.confirmed = (fields[2] == "true" || fields[2] == "1");
                participants.push_back(p);
            } catch (...) { std::cerr << "Warning: bad data at line " << ln << " in session_participants.csv\n"; }
        }
    }

    recompute_indices();
    set_next_ids();
}

void Storage::save_students() {
    std::vector<std::string> lines;
    lines.reserve(students.size());
    for (const auto& kv : students) {
        const Student& s = kv.second;
        std::string hashStr = s.pass_hash ? std::to_string(*s.pass_hash) : "";
        lines.push_back(csv::join_fields({std::to_string(s.id), s.name, s.email, hashStr}));
    }
    atomic_write(studentsFile, lines);
}

void Storage::save_enrollments() {
    std::vector<std::string> lines;
    lines.reserve(enrollments.size());
    for (const auto& e : enrollments) {
        lines.push_back(csv::join_fields({std::to_string(e.student_id), e.course_code}));
    }
    atomic_write(enrollmentsFile, lines);
}

void Storage::save_availability() {
    std::vector<std::string> lines;
    lines.reserve(availability.size());
    for (const auto& a : availability) {
        lines.push_back(csv::join_fields({std::to_string(a.student_id), std::to_string(a.day),
                                          std::to_string(a.start), std::to_string(a.end)}));
    }
    atomic_write(availabilityFile, lines);
}

void Storage::save_sessions() {
    std::vector<std::string> lines;
    lines.reserve(sessions.size());
    for (const auto& kv : sessions) {
        const Session& s = kv.second;
        std::string statusStr = (s.status == SessionStatus::PROPOSED) ? "PROPOSED"
                               : (s.status == SessionStatus::CONFIRMED) ? "CONFIRMED" : "CANCELLED";
        std::string cancelStr = s.cancel_reason ? *s.cancel_reason : "";
        lines.push_back(csv::join_fields({
            std::to_string(s.id), s.course_code, std::to_string(s.day), std::to_string(s.start),
            std::to_string(s.duration), std::to_string(s.organizer_id), statusStr, cancelStr
        }));
    }
    atomic_write(sessionsFile, lines);
}

void Storage::save_participants() {
    std::vector<std::string> lines;
    lines.reserve(participants.size());
    for (const auto& p : participants) {
        lines.push_back(csv::join_fields({std::to_string(p.session_id), std::to_string(p.student_id),
                                          p.confirmed ? "true" : "false"}));
    }
    atomic_write(participantsFile, lines);
}

void Storage::recompute_indices() {
    studentsByEmail.clear();
    for (const auto& kv : students) {
        studentsByEmail[kv.second.email] = kv.first;
    }
    enrollmentsByCourse.clear();
    for (const auto& e : enrollments) {
        enrollmentsByCourse.emplace(e.course_code, e.student_id);
    }
}

void Storage::set_next_ids() {
    int maxStu = 0;
    for (const auto& kv : students) if (kv.first > maxStu) maxStu = kv.first;
    nextStudentId = maxStu + 1;

    int maxSess = 0;
    for (const auto& kv : sessions) if (kv.first > maxSess) maxSess = kv.first;
    nextSessionId = maxSess + 1;
}
