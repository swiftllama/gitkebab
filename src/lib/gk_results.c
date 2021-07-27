
#include "gk_results.h"
#include "gk_logging.h"

#include "stdio.h"
#include "string.h"
#include "stdlib.h"


gk_result *gk_result_new(int code, const char *message) {
    gk_result *result = (gk_result *)malloc(sizeof(gk_result));
    if (result == NULL) {
        log_error(COMP_GENERAL, "Error allocating gk_result for error '%d', '%s'", code, message);
        return NULL;
    }
    result->code = code;
    result->message = message == NULL ? NULL : strdup(message);
    return result;
}

gk_result *gk_result_v(int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_result *result = gk_result_vargs(code, message, args);
    va_end(args);
    return result;
}

gk_result *gk_result_vargs(int code, const char *message, va_list args) {
    char formatted_message[512];
    vsnprintf(formatted_message, 512, message, args);
    return gk_result_new(code, formatted_message);
}

gk_result *gk_result_success() {
    return gk_result_new(0, NULL);
}

void gk_result_free(gk_result *result) {
    if (result != NULL) {
        if (result->message != NULL) {
            free(result->message);
            result->message = NULL;
        }
        free(result);
    }
}

int gk_result_code(gk_result *result) {
    return result != NULL ? result->code : -1;
}

const char *gk_result_message(gk_result *result) {
    return result == NULL ? "(message attribute not available on NULL result)" : result->message != NULL ? result->message : "";
}

gk_result *gk_fail_result(log_Component *component, int code, const char *message, ...) {
    va_list args;
    va_start(args, message);
    gk_result *result = gk_result_v(code, message, args);
    va_end(args);
    log_log(LOG_ERROR, __FILE__, __LINE__, component, gk_result_message(result));
    return result;
}
