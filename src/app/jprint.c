#include "jprint.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ble_sim_uart_tt.h"
#include "logs.h"

void jprint_error(const char *function_name, const char *detail)
{
    cJSON *jroot = NULL;
    cJSON *jpayload = NULL;
    char *jstr = NULL;

    if (detail == NULL)
    {
        detail = "";
    }

    jroot = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jroot, "type", "lpa");
    jpayload = cJSON_CreateObject();
    cJSON_AddNumberToObject(jpayload, "code", -1);
    cJSON_AddStringOrNullToObject(jpayload, "message", function_name);
    cJSON_AddStringOrNullToObject(jpayload, "data", detail);
    cJSON_AddItemToObject(jroot, "payload", jpayload);

    jstr = cJSON_PrintUnformatted(jroot);
    cJSON_Delete(jroot);

    // printf("%s\r\n", jstr);
    // fflush(stdout);

    LOGI("error: %s", jstr);

    ble_sim_uart_tt_send_data((uint8_t*)jstr, strlen(jstr));

    free(jstr);
}

void jprint_progress_with_step(const char* function_name, const char* detail, int step, int total_steps)
{
    cJSON* jroot = NULL;
    cJSON* jpayload = NULL;
    char* jstr = NULL;

    jroot = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jroot, "type", "progress");
    jpayload = cJSON_CreateObject();
    cJSON_AddNumberToObject(jpayload, "code", 0);
    cJSON_AddStringOrNullToObject(jpayload, "message", function_name);
    cJSON_AddStringOrNullToObject(jpayload, "data", detail);
    cJSON_AddNumberToObject(jpayload, "step", step);
    cJSON_AddNumberToObject(jpayload, "total_steps", total_steps);
    cJSON_AddItemToObject(jroot, "payload", jpayload);

    jstr = cJSON_PrintUnformatted(jroot);
    cJSON_Delete(jroot);

    // printf("%s\r\n", jstr);
    // fflush(stdout);

    LOGI("send: %s", jstr);

    ble_sim_uart_tt_send_data((uint8_t*)jstr, strlen(jstr));

    free(jstr);
}

void jprint_progress(const char* function_name, const char* detail)
{
    cJSON *jroot = NULL;
    cJSON *jpayload = NULL;
    char *jstr = NULL;

    jroot = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jroot, "type", "progress");
    jpayload = cJSON_CreateObject();
    cJSON_AddNumberToObject(jpayload, "code", 0);
    cJSON_AddStringOrNullToObject(jpayload, "message", function_name);
    cJSON_AddStringOrNullToObject(jpayload, "data", detail);
    cJSON_AddItemToObject(jroot, "payload", jpayload);

    jstr = cJSON_PrintUnformatted(jroot);
    cJSON_Delete(jroot);

    // printf("%s\r\n", jstr);
    // fflush(stdout);

    LOGI("send: %s", jstr);

    ble_sim_uart_tt_send_data((uint8_t*)jstr, strlen(jstr));

    free(jstr);
}

void jprint_success(cJSON *jdata, const char *data_type)
{
    cJSON *jroot = NULL;
    cJSON *jpayload = NULL;
    char *jstr = NULL;

    jroot = cJSON_CreateObject();
    cJSON_AddStringOrNullToObject(jroot, "type", "lpa");
    jpayload = cJSON_CreateObject();
    cJSON_AddNumberToObject(jpayload, "code", 0);
    cJSON_AddStringOrNullToObject(jpayload, "message", "success");
    cJSON_AddStringOrNullToObject(jpayload, "dataType", data_type);

    if (jdata)
    {
        cJSON_AddItemToObject(jpayload, "data", jdata);
    }
    else
    {
        cJSON_AddNullToObject(jpayload, "data");
    }
    cJSON_AddItemToObject(jroot, "payload", jpayload);

    jstr = cJSON_PrintUnformatted(jroot);
    cJSON_Delete(jroot);

    // printf("%s\r\n", jstr);
    // fflush(stdout);

    LOGI("send: %s", jstr);

    ble_sim_uart_tt_send_data((uint8_t *)jstr, strlen(jstr));

    free(jstr);
}
