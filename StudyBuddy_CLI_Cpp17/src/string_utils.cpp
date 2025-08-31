
#include "string_utils.h"
#include <cctype>
#include <sstream>

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    return s.substr(a, b - a);
}

std::vector<std::string> split_tokens_quoted(const std::string& line) {
    std::vector<std::string> tokens;
    std::string cur;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (in_quotes) {
            if (c == '"') {
                if (i+1 < line.size() && line[i+1] == '"') {
                    cur.push_back('"'); ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                cur.push_back(c);
            }
        } else {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
            } else if (c == '"') {
                in_quotes = true;
            } else {
                cur.push_back(c);
            }
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

std::unordered_map<std::string, std::string> parse_flags(const std::vector<std::string>& tokens) {
    std::unordered_map<std::string, std::string> out;
    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& t = tokens[i];
        if (t.rfind("--", 0) == 0) {
            std::string key = t;
            std::string val;
            if (i + 1 < tokens.size() && tokens[i+1].rfind("--", 0) != 0) {
                val = tokens[i+1];
                ++i;
            } else {
                val = "true"; // flag without value
            }
            out[key] = val;
        }
    }
    return out;
}

bool iequals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) return false;
    }
    return true;
}
