
# Study Buddy (CLI, C++17)

A simple command-line app for Clemson students to create profiles, enroll in courses, manage weekly availability, discover overlapping availability with classmates by exact course code, and schedule study sessions. Implements the approved SRS and pseudocode using standard C++17 only (no third-party dependencies).


## Build & Run
```bash
make          # builds the project into ./study_buddy
make run      # builds and runs
make clean    # removes objects and binary
```

## Data Location
CSV files are stored in `./data/`. On first run, empty files are created as needed.

- `students.csv` — `id,name,email,pass_hash` (pass_hash optional; educational hash via `std::hash`)
- `enrollments.csv` — `student_id,course_code`
- `availability.csv` — `student_id,day,start,end`
- `sessions.csv` — `id,course_code,day,start,duration,organizer_id,status,cancel_reason`
- `session_participants.csv` — `session_id,student_id,confirmed` (true/false)

Sample seed data is included for quick testing.
Every new user added updates the csv to store the information.

## Example quick start functionality.
```bash
# 1. Create and login
create_profile --name "Avery Tiger" --email averyt@clemson.edu
login --email averyt@clemson.edu [--passcode 1234]

# 2. Add/Remove a course and availability
add_course --code "CPSC 2120"
add_availability --day 2 --start 14 --end 17
remove_course --code "CPSC 2120"
remove_availability --day 2 --start 14 --end 17

# 3. Search matches and schedule study sessions with classmates
search_matches --course "CPSC 2120"
schedule_session --course "CPSC 2120" --day 2 --start 15 --invite 7

# 4. Confirm meetings / viewing study sessions
confirm_session --id 1
list_sessions
list_invitations
```

## Command Reference (Examples)
> **Note:** Values containing spaces must be quoted.

### Create or Log In
```bash
create_profile --name "Avery Tiger" --email averyt@clemson.edu [--passcode 1234]
login --email averyt@clemson.edu [--passcode 1234]
```

### Profile Edit
```bash
edit_profile --name "Avery Q. Tiger"
edit_profile --email aqt@clemson.edu
```

### Courses
```bash
add_course --code "CPSC 2120"
remove_course --code "CPSC 2120"
list_courses
```

### Availability (0=Sun..6=Sat; hours 0..24, exclusive end)
```bash
add_availability --day 2 --start 14 --end 17
remove_availability --day 2 --start 14 --end 17
list_availability
```
- Adding overlapped/adjacent ranges merges them automatically.

### Search Classmate Matches (by exact course code)
```bash
search_matches --course "CPSC 2120"
```
- Shows classmates (#id and name) with overlapping days/hours.

### Schedule / Confirm / Cancel Sessions
```bash
# Invite one or more classmates by their numeric ids
schedule_session --course "CPSC 2120" --day 2 --start 15 --invite 7,12

# Confirm an invitation you are part of (organizer must also confirm)
confirm_session --id 31

# Cancel (any participant can cancel)
cancel_session --id 31 --reason "Conflict"
```

### Lists
```bash
list_sessions       # grouped by PROPOSED, CONFIRMED, CANCELLED
list_invitations    # pending confirmations for current user
help                # show all commands
exit                # quit the program
```

## Notes & Guarantees
- Single-user, offline CLI; operations are persisted immediately with atomic file writes.
- Email and course codes are validated. Duplicate emails or course enrollments are prevented.
- Availability is stored with 1-hour granularity and merged to avoid overlaps.
- A session becomes CONFIRMED only when all participants (including the organizer) confirm.
- On confirmation, availability and time conflicts are re-checked.


