
#ifndef __GK_EXECUTION_CONTEXT__H_
#define __GK_EXECUTION_CONTEXT__H_

#include "gk_results.h"
#include "gk_logging.h"

typedef struct gk_execution_context gk_execution_context;

struct gk_execution_context {
    const char *purpose;
    gk_result *result;
    gk_execution_context *child_context;
    log_Component *log_component;
};

gk_execution_context *gk_execution_context_new(const char *purpose, log_Component *log_component);
void gk_execution_context_free(gk_execution_context *context);
void gk_execution_context_push(gk_execution_context *context, const char *purpose, log_Component *log_component);
void gk_execution_context_pop(gk_execution_context *context, const char *purpose);
int gk_execution_context_success(gk_execution_context *context);
void gk_execution_context_log_stack_failure(gk_execution_context *context);
size_t gk_execution_context_stack_size(gk_execution_context *context);

#endif // __GK_EXECUTION_CONTEXT_H__
