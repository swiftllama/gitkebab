
#include "results.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "gk_logging.h"

static gk_result_t *gk_result_new() {
    return (gk_result_t *)malloc(sizeof(gk_result_t));
}

gk_result_t *gk_result(int code, const char *message) {
    gk_result_t *result = gk_result_new();
    if (result == NULL) {
        log_error(COMP_GENERAL, "Error allocating gk_result_t for error '%d', '%s'", code, message);
        return NULL;
    }
    result->code = code;
    result->message = message == NULL ? NULL : strdup(message);
    result.cause = NULL;
    return result;
}

gk_result_t *gk_result_with_cause(int code, const char* message, gk_result_t *cause) {
    gk_result_t *result = gk_result(code, message);
    if (result != NULL) {
        result->cause = cause;
    }
    return result;
}

gk_result_t *gk_result_success() {
    return gk_result(0, NULL);
}

void gk_result_free(gk_result_t *result) {
    if (result != NULL) {
        if (result->message != NULL) {
            free(result->message);
            result->message = NULL;
        }
        if (result->cause != NULL) {
            gk_result_free(result->cause);
            result->cause = NULL;
        }
        free(result);
    }
}

const char *gk_result_message(gk_result_t *result) {
    return result != NULL ? result -> message : "";
}
