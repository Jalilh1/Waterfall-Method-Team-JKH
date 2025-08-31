
#include "cli.h"
#include "string_utils.h"
#include "validation.h"
#include <iostream>
#include <sstream>

CLI::CLI()
: store("data"),
  profileSvc(store),
  courseSvc(store),
  availSvc(store),
  matchSvc(store, courseSvc),
  sessionSvc(store, courseSvc, availSvc)
{}

void CLI::print_welcome() const {
    std::cout << "Study Buddy CLI — type 'help' for commands.\n";
}

void CLI::print_help() const {
    std::cout << "Commands:\n"
              << "  create_profile --name <str> --email <str> [--passcode <str>]\n"
              << "  login --email <str> [--passcode <str>]\n"
              << "  whoami\n"
              << "  edit_profile [--name <str>] [--email <str>]\n"
              << "  add_course --code <DEPT NUM>\n"
              << "  remove_course --code <DEPT NUM>\n"
              << "  list_courses\n"
              << "  add_availability --day <0..6> --start <0..23> --end <1..24>\n"
              << "  remove_availability --day <0..6> --start <..> --end <..>\n"
              << "  list_availability\n"
              << "  search_matches --course <DEPT NUM>\n"
              << "  schedule_session --course <DEPT NUM> --day <0..6> --start <0..23> --invite <id,id,..>\n"
              << "  confirm_session --id <session_id>\n"
              << "  cancel_session --id <session_id> [--reason <text>]\n"
              << "  list_sessions\n"
              << "  list_invitations\n"
              << "  help | exit\n";
}

bool CLI::require_logged_in() const {
    if (current_user < 0) {
        std::cerr << "[ERROR] Not logged in. Use 'login --email <str>' or create_profile.\n";
        return false;
    }
    return true;
}

void CLI::cmd_create_profile(const std::unordered_map<std::string,std::string>& args) {
    auto itN = args.find("--name");
    auto itE = args.find("--email");
    if (itN == args.end() || itE == args.end()) {
        std::cerr << "Usage: create_profile --name <str> --email <str> [--passcode <str>]\n";
        return;
    }
    std::optional<std::string> pw;
    auto itP = args.find("--passcode");
    if (itP != args.end()) pw = itP->second;
    auto id = profileSvc.create_profile(itN->second, itE->second, pw);
    if (id) current_user = *id;
}

void CLI::cmd_login(const std::unordered_map<std::string,std::string>& args) {
    auto itE = args.find("--email");
    if (itE == args.end()) { std::cerr << "Usage: login --email <str> [--passcode <str>]\n"; return; }
    auto it = store.studentsByEmail.find(itE->second);
    if (it == store.studentsByEmail.end()) { std::cerr << "[ERROR] NO_SUCH_USER\n"; return; }
    int id = it->second;
    auto& stu = store.students[id];
    auto itP = args.find("--passcode");
    if (stu.pass_hash) {
        if (itP == args.end()) { std::cerr << "[ERROR] PASSCODE_REQUIRED\n"; return; }
        std::hash<std::string> hasher;
        if (*stu.pass_hash != hasher(itP->second)) { std::cerr << "[ERROR] BAD_PASSCODE\n"; return; }
    }
    current_user = id;
    std::cout << "Logged in as id=" << id << " (" << stu.name << ")\n";
}

void CLI::cmd_whoami() const {
    if (!require_logged_in()) return;
    const auto& s = store.students.at(current_user);
    std::cout << "Current user: id=" << s.id << " name=" << s.name << " email=" << s.email << "\n";
}

void CLI::cmd_edit_profile(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    bool any = false;
    auto itN = args.find("--name");
    if (itN != args.end()) { any = profileSvc.edit_profile_name(current_user, itN->second) || any; }
    auto itE = args.find("--email");
    if (itE != args.end()) { any = profileSvc.edit_profile_email(current_user, itE->second) || any; }
    if (!any) std::cout << "Nothing to update.\n";
}

void CLI::cmd_add_course(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto it = args.find("--code");
    if (it == args.end()) { std::cerr << "Usage: add_course --code <DEPT NUM>\n"; return; }
    std::string err;
    if (courseSvc.add_course(current_user, it->second, err)) {
        std::cout << "Course added.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_remove_course(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto it = args.find("--code");
    if (it == args.end()) { std::cerr << "Usage: remove_course --code <DEPT NUM>\n"; return; }
    std::string err;
    if (courseSvc.remove_course(current_user, it->second, err)) {
        std::cout << "Course removed.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_list_courses() {
    if (!require_logged_in()) return;
    auto list = courseSvc.list_courses(current_user);
    if (list.empty()) { std::cout << "(no courses)\n"; return; }
    for (auto& c : list) std::cout << c << "\n";
}

void CLI::cmd_add_availability(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto d = args.find("--day"); auto s = args.find("--start"); auto e = args.find("--end");
    if (d == args.end() || s == args.end() || e == args.end()) {
        std::cerr << "Usage: add_availability --day <0..6> --start <0..23> --end <1..24>\n"; return;
    }
    std::string err;
    if (availSvc.add_availability(current_user, std::stoi(d->second), std::stoi(s->second), std::stoi(e->second), err)) {
        std::cout << "Availability added/merged.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_remove_availability(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto d = args.find("--day"); auto s = args.find("--start"); auto e = args.find("--end");
    if (d == args.end() || s == args.end() || e == args.end()) {
        std::cerr << "Usage: remove_availability --day <0..6> --start <..> --end <..>\n"; return;
    }
    std::string msg;
    if (availSvc.remove_availability_exact(current_user, std::stoi(d->second), std::stoi(s->second), std::stoi(e->second), msg)) {
        std::cout << "Availability removed.\n";
    } else {
        std::cout << msg << "\n";
    }
}

void CLI::cmd_list_availability() {
    if (!require_logged_in()) return;
    auto slots = availSvc.list_availability(current_user);
    if (slots.empty()) { std::cout << "(no availability)\n"; return; }
    for (const auto& a : slots) {
        std::cout << "Day " << a.day << ": " << a.start << "-" << a.end << "\n";
    }
}

void CLI::cmd_search_matches(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto it = args.find("--course");
    if (it == args.end()) { std::cerr << "Usage: search_matches --course <DEPT NUM>\n"; return; }
    std::string err;
    auto matches = matchSvc.suggest_matches(current_user, it->second, err);
    if (!err.empty()) { std::cerr << "[ERROR] " << err << "\n"; return; }
    if (matches.empty()) { std::cout << "No matches found.\n"; return; }
    for (const auto& m : matches) {
        std::cout << "#" << m.classmate_id << " " << m.classmate_name << ": ";
        bool first = true;
        for (const auto& pr : m.overlaps) {
            if (!first) std::cout << " | ";
            first = false;
            std::cout << "Day " << pr.first << " [";
            for (size_t i = 0; i < pr.second.size(); ++i) {
                if (i) std::cout << ",";
                std::cout << pr.second[i];
            }
            std::cout << "]";
        }
        std::cout << "\n";
    }
}

void CLI::cmd_schedule_session(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto c = args.find("--course"); auto d = args.find("--day"); auto s = args.find("--start"); auto inv = args.find("--invite");
    if (c == args.end() || d == args.end() || s == args.end() || inv == args.end()) {
        std::cerr << "Usage: schedule_session --course <DEPT NUM> --day <0..6> --start <0..23> --invite <id,id,..>\n"; return;
    }
    std::vector<int> ids;
    std::stringstream ss(inv->second);
    std::string tok;
    while (std::getline(ss, tok, ',')) {
        tok = trim(tok);
        if (!tok.empty()) ids.push_back(std::stoi(tok));
    }
    std::string err;
    if (sessionSvc.schedule_session(current_user, c->second, std::stoi(d->second), std::stoi(s->second), ids, err)) {
        std::cout << "Session PROPOSED. Awaiting confirmations.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_confirm_session(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto it = args.find("--id");
    if (it == args.end()) { std::cerr << "Usage: confirm_session --id <session_id>\n"; return; }
    std::string err;
    if (sessionSvc.confirm_session(current_user, std::stoi(it->second), err)) {
        std::cout << "Confirmed.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_cancel_session(const std::unordered_map<std::string,std::string>& args) {
    if (!require_logged_in()) return;
    auto it = args.find("--id");
    std::string reason;
    auto r = args.find("--reason");
    if (r != args.end()) reason = r->second; else reason = "No reason provided";
    if (it == args.end()) { std::cerr << "Usage: cancel_session --id <session_id> [--reason <text>]\n"; return; }
    std::string err;
    if (sessionSvc.cancel_session(current_user, std::stoi(it->second), reason, err)) {
        std::cout << "Cancelled.\n";
    } else {
        std::cerr << "[ERROR] " << err << "\n";
    }
}

void CLI::cmd_list_sessions() {
    if (!require_logged_in()) return;
    auto list = sessionSvc.list_sessions_for(current_user);
    if (list.empty()) { std::cout << "(no sessions)\n"; return; }
    // Print grouped by status
    auto print_group = [&](SessionStatus st, const char* title){
        std::cout << title << ":\n";
        for (const auto& s : list) {
            if (s.status != st) continue;
            std::cout << "  [" << s.id << "] " << s.course_code << " Day " << s.day << " " << s.start << ":00-" << (s.start+1) << ":00"
                      << " Organizer:" << s.organizer_id;
            // participants + confirmed flags
            std::cout << " Participants:";
            bool first = true;
            for (const auto& p : store.participants) {
                if (p.session_id == s.id) {
                    if (!first) std::cout << ",";
                    first = false;
                    std::cout << p.student_id << (p.confirmed ? "(Y)" : "(N)");
                }
            }
            if (s.status == SessionStatus::CANCELLED && s.cancel_reason) std::cout << " Reason:" << *s.cancel_reason;
            std::cout << "\n";
        }
    };
    print_group(SessionStatus::PROPOSED, "PROPOSED");
    print_group(SessionStatus::CONFIRMED, "CONFIRMED");
    print_group(SessionStatus::CANCELLED, "CANCELLED");
}

void CLI::cmd_list_invitations() {
    if (!require_logged_in()) return;
    auto list = sessionSvc.list_pending_invitations_for(current_user);
    if (list.empty()) { std::cout << "(no pending invitations)\n"; return; }
    for (const auto& s : list) {
        std::cout << "  [" << s.id << "] " << s.course_code << " Day " << s.day << " " << s.start << ":00-" << (s.start+1) << ":00\n";
    }
}

void CLI::handle_command(const std::string& line) {
    auto tokens = split_tokens_quoted(line);
    if (tokens.empty()) return;
    std::string cmd = tokens[0];
    auto args = parse_flags(tokens);
    if (cmd == "help") { print_help(); return; }
    if (cmd == "exit") { std::cout << "Goodbye\n"; std::exit(0); }
    if (cmd == "create_profile") { cmd_create_profile(args); return; }
    if (cmd == "login") { cmd_login(args); return; }
    if (cmd == "whoami") { cmd_whoami(); return; }
    if (cmd == "edit_profile") { cmd_edit_profile(args); return; }
    if (cmd == "add_course") { cmd_add_course(args); return; }
    if (cmd == "remove_course") { cmd_remove_course(args); return; }
    if (cmd == "list_courses") { cmd_list_courses(); return; }
    if (cmd == "add_availability") { cmd_add_availability(args); return; }
    if (cmd == "remove_availability") { cmd_remove_availability(args); return; }
    if (cmd == "list_availability") { cmd_list_availability(); return; }
    if (cmd == "search_matches") { cmd_search_matches(args); return; }
    if (cmd == "schedule_session") { cmd_schedule_session(args); return; }
    if (cmd == "confirm_session") { cmd_confirm_session(args); return; }
    if (cmd == "cancel_session") { cmd_cancel_session(args); return; }
    if (cmd == "list_sessions") { cmd_list_sessions(); return; }
    if (cmd == "list_invitations") { cmd_list_invitations(); return; }

    std::cerr << "Unknown command. Type 'help'.\n";
}

int CLI::run() {
    print_welcome();
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        handle_command(line);
    }
    return 0;
}
