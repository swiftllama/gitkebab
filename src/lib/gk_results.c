
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
    return gk_result_new(GK_SUCCESS, NULL);
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

const char *gk_result_code_as_string(int code) {
    switch(code) {
    case GK_SUCCESS: return "GK_SUCCESS"; break;
    case GK_FAILURE: return "GK_FAILURE"; break;
    case GK_ERR: return "GK_ERR"; break;
    case GK_ERR_CLONE_INEXISTENT_SOURCE_PATH: return "GK_ERR_CLONE_INEXISTENT_SOURCE_PATH"; break;
    case GK_ERR_CLONE_INVALID_DESTINATION_PATH: return "GK_ERR_CLONE_INVALID_DESTINATION_PATH"; break;
    case GK_ERR_CLONE_DESTINATION_PATH_NONEMPTY: return "GK_ERR_CLONE_DESTINATION_PATH_NONEMPTY"; break;
    case GK_ERR_REPOSITORY_NO_LOCAL_CHECKOUT: return "GK_ERR_REPOSITORY_NO_LOCAL_CHECKOUT"; break;
    case GK_ERR_NOT_FOUND: return "GK_ERR_NOT_FOUND"; break;
    case GK_ERR_MERGE_HAS_CONFLICTS: return "GK_ERR_MERGE_HAS_CONFLICTS"; break;
    default:
        log_error(COMP_TEST, "Unknown GK error code [%d]", code);
        return "<unknown code>";
    }
}
