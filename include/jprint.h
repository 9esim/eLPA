#pragma once
#include <cjson/cJSON_ex.h>

void jprint_error(const char *function_name, const char *detail);
void jprint_progress(const char* function_name, const char* detail);
void jprint_progress_with_step(const char* function_name, const char* detail, int step, int total_steps);
// void jprint_success(cJSON* jdata);
void jprint_success(cJSON* jdata, const char* data_type);
