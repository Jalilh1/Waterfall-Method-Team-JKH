
#ifndef STUDY_BUDDY_VALIDATION_H
#define STUDY_BUDDY_VALIDATION_H

#include <string>

bool is_valid_email(const std::string& email);
bool is_valid_course(const std::string& code);
bool is_valid_day(int d);
bool is_valid_hour(int h);
bool is_valid_avail_range(int start, int end);

#endif // STUDY_BUDDY_VALIDATION_H
