
#ifndef __GITKEBAB_RESULTS_H__
#define __GITKEBAB_RESULTS_H__

#include "gk_logging.h"

enum GK_RESULT_CODE {
    GK_SUCCESS, GK_FAILURE
};
    
typedef struct gk_result_t gk_result_t;

struct gk_result_t {
    int code;
    char *message;
};


    
gk_result_t *gk_result(int code, const char *message);
gk_result_t *gk_result_v(int code, const char *message, ...);
gk_result_t *gk_result_vargs(int code, const char *message, va_list args);
gk_result_t *gk_result_success();
void gk_result_free(gk_result_t *result);

int gk_result_code(gk_result_t *result);
const char *gk_result_message(gk_result_t *result);

gk_result_t *gk_fail_result(log_Component *component, int code, const char *message, ...);

#endif // __GITKEBAB_RESULTS_H__
