
#ifndef __GITKEBAB_RESULTS_H__
#define __GITKEBAB_RESULTS_H__

typedef struct {
    int code;
    char *message;
    gk_result_t *cause;
} gk_result_t;
    
gk_result_t *gk_result(int code, const char *message);
gk_result_t *gk_result_with_cause(int code, const char* message, gk_result_t *cause);
gk_result_t *gk_result_success();
void gk_result_free(gk_result_t *result);
                          
#endif // __GITKEBAB_RESULTS_H__
