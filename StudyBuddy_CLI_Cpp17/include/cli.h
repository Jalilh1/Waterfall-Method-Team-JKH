
#ifndef STUDY_BUDDY_CLI_H
#define STUDY_BUDDY_CLI_H

#include "storage.h"
#include "services_profile.h"
#include "services_course.h"
#include "services_availability.h"
#include "services_match.h"
#include "services_session.h"

class CLI {
public:
    CLI();
    int run();

private:
    Storage store;
    ProfileService profileSvc;
    CourseService courseSvc;
    AvailabilityService availSvc;
    MatchService matchSvc;
    SessionService sessionSvc;

    int current_user{-1};

    void print_help() const;
    void print_welcome() const;

    void handle_command(const std::string& line);
    void cmd_create_profile(const std::unordered_map<std::string,std::string>& args);
    void cmd_login(const std::unordered_map<std::string,std::string>& args);
    void cmd_whoami() const;
    void cmd_edit_profile(const std::unordered_map<std::string,std::string>& args);
    void cmd_add_course(const std::unordered_map<std::string,std::string>& args);
    void cmd_remove_course(const std::unordered_map<std::string,std::string>& args);
    void cmd_list_courses();
    void cmd_add_availability(const std::unordered_map<std::string,std::string>& args);
    void cmd_remove_availability(const std::unordered_map<std::string,std::string>& args);
    void cmd_list_availability();
    void cmd_search_matches(const std::unordered_map<std::string,std::string>& args);
    void cmd_schedule_session(const std::unordered_map<std::string,std::string>& args);
    void cmd_confirm_session(const std::unordered_map<std::string,std::string>& args);
    void cmd_cancel_session(const std::unordered_map<std::string,std::string>& args);
    void cmd_list_sessions();
    void cmd_list_invitations();

    bool require_logged_in() const;
};

#endif // STUDY_BUDDY_CLI_H
