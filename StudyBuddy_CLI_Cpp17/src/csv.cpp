
#include "csv.h"
#include <sstream>

std::vector<std::string> csv::parse_line(const std::string& line) {
    std::vector<std::string> fields;
    std::string cur;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (in_quotes) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    cur.push_back('"'); // escaped quote
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (c == ',') {
                fields.push_back(cur);
                cur.clear();
            } else if (c == '"') {
                in_quotes = true;
            } else {
                cur.push_back(c);
            }
        }
    }
    if (in_quotes) {
        return {}; // malformed
    }
    fields.push_back(cur);
    return fields;
}

std::string csv::escape_field(const std::string& field) {
    bool need_quotes = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') { need_quotes = true; break; }
    }
    if (!need_quotes) return field;
    std::string out = "\"";
    for (char c : field) {
        if (c == '"') out += "\"\"";
        else out.push_back(c);
    }
    out += "\"";
    return out;
}

std::string csv::join_fields(const std::vector<std::string>& fields) {
    std::ostringstream oss;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) oss << ",";
        oss << escape_field(fields[i]);
    }
    return oss.str();
}
