
#ifndef STUDY_BUDDY_CSV_H
#define STUDY_BUDDY_CSV_H

#include <string>
#include <vector>

namespace csv {

// Parse a single CSV line into fields (supports quotes and escaped quotes by "").
// Returns empty vector if parsing fails (e.g., unmatched quote).
std::vector<std::string> parse_line(const std::string& line);

// Escape a single CSV field per RFC4180-ish rules.
std::string escape_field(const std::string& field);

// Join fields into CSV line.
std::string join_fields(const std::vector<std::string>& fields);

} // namespace csv

#endif // STUDY_BUDDY_CSV_H
