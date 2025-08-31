
#ifndef STUDY_BUDDY_STRING_UTILS_H
#define STUDY_BUDDY_STRING_UTILS_H

#include <string>
#include <vector>
#include <unordered_map>

std::string trim(const std::string& s);
std::vector<std::string> split_tokens_quoted(const std::string& line);
std::unordered_map<std::string, std::string> parse_flags(const std::vector<std::string>& tokens);
bool iequals(const std::string& a, const std::string& b);

#endif // STUDY_BUDDY_STRING_UTILS_H
