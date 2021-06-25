
#include "results.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

gk_result_t gk_result(int code, const char *message) {
    gk_result_t result;
    result.code = code;
    result.message = message == NULL ? NULL : strdup(message);
    return result;
}

gk_result_t gk_result_success() {
    return gk_result(0, NULL);
}

void gk_result_free(gk_result_t *result) {
    if (result != NULL) {
        if (result->message != NULL) {
            free(result->message);
            result->message = NULL;
        }
    }
}
