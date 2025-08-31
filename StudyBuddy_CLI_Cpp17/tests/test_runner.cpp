#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <functional>
#include <filesystem>
#include <optional>
#include <sstream>
#include <unordered_map>

// Project headers
#include "storage.h"
#include "services_profile.h"
#include "services_course.h"
#include "services_availability.h"
#include "services_match.h"
#include "services_session.h"
#include "validation.h"

namespace fs = std::filesystem;

struct TestResult {
    std::string id;
    std::string title;
    bool passed;
    std::string detail;
};

struct TestContext {
    std::string dataDir;
    std::unique_ptr<Storage> store;
    std::unique_ptr<ProfileService> profile;
    std::unique_ptr<CourseService> course;
    std::unique_ptr<AvailabilityService> avail;
    std::unique_ptr<MatchService> match;
    std::unique_ptr<SessionService> session;
};

static void reset_data_dir(const std::string& dir) {
    std::error_code ec;
    if (fs::exists(dir)) fs::remove_all(dir, ec);
    fs::create_directories(dir, ec);
    // Touch empty CSVs
    std::ofstream(dir + "/students.csv").close();
    std::ofstream(dir + "/enrollments.csv").close();
    std::ofstream(dir + "/availability.csv").close();
    std::ofstream(dir + "/sessions.csv").close();
    std::ofstream(dir + "/session_participants.csv").close();
}

static TestContext make_ctx(const std::string& dir) {
    TestContext ctx;
    ctx.dataDir = dir;
    ctx.store   = std::make_unique<Storage>(dir);
    ctx.profile = std::make_unique<ProfileService>(*ctx.store);
    ctx.course  = std::make_unique<CourseService>(*ctx.store);
    ctx.avail   = std::make_unique<AvailabilityService>(*ctx.store);
    ctx.match   = std::make_unique<MatchService>(*ctx.store, *ctx.course);
    ctx.session = std::make_unique<SessionService>(*ctx.store, *ctx.course, *ctx.avail);
    return ctx;
}

static std::string csv_escape(const std::string& s) {
    bool need = false;
    for (char c: s) if (c==',' || c=='"' || c=='\n' || c=='\r') { need = true; break; }
    if (!need) return s;
    std::string out = "\"";
    for (char c: s) out += (c=='"') ? "\"\"" : std::string(1, c);
    out += '"';
    return out;
}

static void write_csv(const std::string& path, const std::vector<TestResult>& results) {
    std::ofstream ofs(path);
    ofs << "TestID,Title,Outcome,Detail\n";
    for (const auto& r : results) {
        ofs << r.id << ","
            << csv_escape(r.title) << ","
            << (r.passed ? "PASSED" : "FAILED") << ","
            << csv_escape(r.detail) << "\n";
    }
}

int main() {
    std::vector<TestResult> results;
    const std::string DIR = "test_data";
    reset_data_dir(DIR);
    auto ctx = make_ctx(DIR);

    // ---- Profile ----
    { // T01 Create profile
        auto id = ctx.profile->create_profile("Avery Tiger","avery@clemson.edu", std::nullopt);
        bool ok = id.has_value() && *id == 1 && ctx.store->students.count(1);
        results.push_back({"T01","Create profile", ok, ok ? "" : "Expected id=1"});
    }
    { // T02 Duplicate email
        auto id = ctx.profile->create_profile("Dup","avery@clemson.edu", std::nullopt);
        bool ok = !id.has_value();
        results.push_back({"T02","Duplicate email rejected", ok, ok ? "" : "Duplicate allowed"});
    }

    // ---- Courses ----
    { // T03 Add course
        std::string err;
        bool ok = ctx.course->add_course(1, "CPSC 2120", err);
        results.push_back({"T03","Add course CPSC 2120", ok, ok ? "" : err});
    }
    { // T05 Duplicate course
        std::string err;
        bool ok = ctx.course->add_course(1, "CPSC 2120", err);
        bool ok2 = (!ok && err=="DUP_COURSE");
        results.push_back({"T05","Duplicate course rejected", ok2, ok2 ? "" : ("err="+err)});
    }
    { // T04 List courses
        auto list = ctx.course->list_courses(1);
        bool found=false; for (auto& c: list) if (c=="CPSC 2120") { found=true; break; }
        results.push_back({"T04","List courses includes CPSC 2120", found, found ? "" : "Course missing"});
    }

    // ---- Availability ----
    { // T06 Add availability
        std::string err;
        bool ok = ctx.avail->add_availability(1, 2, 14, 17, err);
        results.push_back({"T06","Add availability 2:14-17", ok, ok ? "" : err});
    }
    { // T08 Remove non-existent availability (message)
        std::string msg;
        bool ok = !ctx.avail->remove_availability_exact(1, 2, 10, 11, msg);
        bool ok2 = ok && msg.find("No matching") != std::string::npos;
        results.push_back({"T08","Remove non-existent availability reports message", ok2, ok2 ? "" : msg});
    }
    { // T07 Remove existing availability
        std::string msg;
        bool ok = ctx.avail->remove_availability_exact(1, 2, 14, 17, msg);
        results.push_back({"T07","Remove existing availability", ok, ok ? "" : msg});
        // Re-add for subsequent tests
        std::string err;
        ctx.avail->add_availability(1, 2, 14, 17, err);
    }

    // ---- Matching ----
    int userB = -1;
    { // Seed User B
        auto id = ctx.profile->create_profile("Jordan Lee","jlee3@clemson.edu", std::nullopt);
        userB = id.value_or(-1);
        std::string e;
        ctx.course->add_course(userB, "CPSC 2120", e);
        ctx.avail->add_availability(userB, 2, 15, 17, e);
    }
    { // T09 Overlap exists
        std::string err;
        auto matches = ctx.match->suggest_matches(1, "CPSC 2120", err);
        bool ok = err.empty() && !matches.empty();
        results.push_back({"T09","Suggest matches (overlap exists)", ok, ok ? "" : err});
    }
    { // T10 No overlap
        std::string e;
        ctx.avail->remove_availability_exact(userB, 2, 15, 17, e);
        ctx.avail->add_availability(userB, 3, 10, 12, e);
        std::string err;
        auto matches = ctx.match->suggest_matches(1, "CPSC 2120", err);
        bool ok = err.empty() && matches.empty();
        results.push_back({"T10","Suggest matches (no overlap)", ok, ok ? "" : "Unexpected candidates"});
        // Restore overlap
        ctx.avail->remove_availability_exact(userB, 3, 10, 12, e);
        ctx.avail->add_availability(userB, 2, 15, 17, e);
    }

    // ---- Scheduling ----
    int sessId = -1;
    { // T11 Valid schedule
        std::string err;
        bool ok = ctx.session->schedule_session(1, "CPSC 2120", 2, 15, std::vector<int>{userB}, err);
        if (ok) {
            int mx=0; for (auto& kv : ctx.store->sessions) mx = std::max(mx, kv.first);
            sessId = mx;
        }
        results.push_back({"T11","Schedule valid session", ok, ok ? "" : err});
    }
    { // T12 Outside organizer availability
        std::string err;
        bool ok = !ctx.session->schedule_session(1, "CPSC 2120", 2, 18, std::vector<int>{userB}, err)
                  && err=="OUTSIDE_AVAIL_ORG";
        results.push_back({"T12","Schedule outside availability rejected", ok, ok ? "" : err});
    }
    int userC = -1;
    { // T18 Non-enrolled invitee
        auto id = ctx.profile->create_profile("Casey","casey@clemson.edu", std::nullopt);
        userC = id.value_or(-1);
        std::string err;
        bool ok = !ctx.session->schedule_session(1, "CPSC 2120", 2, 16, std::vector<int>{userC}, err)
                  && err=="INV_NOT_ENROLLED";
        results.push_back({"T18","Schedule with non-enrolled invitee rejected", ok, ok ? "" : ("err="+err)});
    }

    // ---- Confirmation / Cancellation ----
    { // T13 Confirm (both) => CONFIRMED
        std::string err1, err2;
        bool ok1 = ctx.session->confirm_session(1,     sessId, err1);
        bool ok2 = ctx.session->confirm_session(userB, sessId, err2);
        bool statusConfirmed = ctx.store->sessions[sessId].status == SessionStatus::CONFIRMED;
        bool ok = ok1 && ok2 && statusConfirmed;
        std::ostringstream ss; ss << "org="<<ok1<<" inv="<<ok2<<" confirmed="<<statusConfirmed;
        results.push_back({"T13","Confirm session transitions to CONFIRMED", ok, ss.str()});
    }
    { // T14 Non-participant confirm
        std::string err;
        bool ok = !ctx.session->confirm_session(userC, sessId, err) && err=="NOT_PARTICIPANT";
        results.push_back({"T14","Confirm by non-participant rejected", ok, ok ? "" : err});
    }

    { // T19 Cancel
        std::string err;
        bool ok = ctx.session->cancel_session(1, sessId, "Conflict", err);
        bool cancelled = ctx.store->sessions[sessId].status == SessionStatus::CANCELLED;
        results.push_back({"T19","Cancel session sets CANCELLED", ok && cancelled, ok ? "" : err});
    }

    // Output CSV
    write_csv("test_results.csv", results);

    // Console summary
    int pass=0; for (auto& r: results) if (r.passed) ++pass;
    std::cout << "Tests passed: " << pass << "/" << results.size() << "\n";
    for (auto& r: results) {
        std::cout << r.id << " - " << (r.passed ? "PASSED" : "FAILED");
        if (!r.detail.empty()) std::cout << " : " << r.detail;
        std::cout << "\n";
    }
    return (pass == (int)results.size()) ? 0 : 1;
}
