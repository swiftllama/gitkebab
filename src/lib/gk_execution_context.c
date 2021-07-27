
#include <stdlib.h>
#include <string.h>

#include "gk_execution_context.h"
#include "gk_logging.h"
#include "gk_results.h"

gk_execution_context *gk_execution_context_new(const char *purpose, log_Component *log_component) {
    const char *safe_purpose = purpose != NULL ? purpose : "<unspecified execution context purpose>";
    gk_execution_context *context = (gk_execution_context *)malloc(sizeof(gk_execution_context));
    context->purpose = safe_purpose;
    context->result = gk_result_success();
    context->child_context = NULL;
    context->log_component = log_component != NULL ? log_component : &COMP_GENERAL;
    return context;
}

void gk_execution_context_free(gk_execution_context *context) {
    if (context == NULL) {
        return;
    }
    if (context->child_context != NULL) {
        gk_execution_context_free(context->child_context);
        context->child_context = NULL;
    }
    if (context->result != NULL) {
        gk_result_free(context->result);
        context->result = NULL;
    }
    context->purpose = NULL;
    context->log_component = NULL;
    free(context);
}

gk_execution_context *gk_execution_context_last_parent(gk_execution_context *context) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot find last parent for NULL context");
        return NULL;
    }
    gk_execution_context *last_parent = context;
    int count = 0;
    while ((last_parent->child_context != NULL) && (last_parent->child_context->child_context != NULL)) {
        last_parent = last_parent->child_context;
        count += 1;
        if (count >= 64) {
            log_error(COMP_EXCTX, "Could not find last child in execution context stack larger than 64, last purpose is [%s]", last_parent->purpose);
        break;
        }
    }
    return last_parent;
}

gk_execution_context *gk_execution_context_last_child(gk_execution_context *context) {
    gk_execution_context *last_parent = gk_execution_context_last_parent(context);
    return last_parent->child_context != NULL ? last_parent->child_context : last_parent;
}

 void print_execution_chain(gk_execution_context *context) {
    gk_execution_context *next_context = context;
    log_error(COMP_EXCTX, "[CHAIN]\n");
    for (int i = 0; i <= 10; i += 1) {
        log_error(COMP_EXCTX, "  (%d) is [%p] [%s]\n", i, (void *)next_context, next_context->purpose);
        next_context = next_context->child_context;
        if (next_context == NULL) {
            log_error(COMP_EXCTX, "[CHAIN END]\n\n");
            return;
        }
    }
    log_error(COMP_EXCTX, "[CHAIN] end not found!\n");
}

void gk_execution_context_push(gk_execution_context *context, const char *purpose, log_Component *log_component) {
    gk_execution_context *last_child = gk_execution_context_last_child(context);
    last_child->child_context = gk_execution_context_new(purpose, log_component != NULL ? log_component : context->log_component);
}

void gk_execution_context_pop(gk_execution_context *context, const char *purpose) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot pop NULL context with purpose [%s]", purpose);
        return;
    }
    if (context->child_context == NULL) {
        log_error(COMP_EXCTX, "Cannot pop contex with NULL child for purpose [%s]", purpose);
        return;
    }
    gk_execution_context *last_parent = gk_execution_context_last_parent(context);
    gk_execution_context *last_child = last_parent->child_context;
    if (strcmp(last_child->purpose, purpose) != 0) {
        log_error(COMP_EXCTX, "Asked to pop context with purpose [%s], but last child has different purpose [%s]", purpose,  last_child->purpose);
        return;
    }
    gk_execution_context_free(last_child);
    last_parent->child_context = NULL;
}

void gk_execution_context_log_stack_failure(gk_execution_context *context) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot log context stack for NULL context");
        return;
    }
    gk_execution_context *next_context = context;
    const char *prefix = "Cannot";
    while (next_context != NULL) {
        if (next_context->result != NULL) {
            log_log(LOG_ERROR, __FILE__, __LINE__, context->log_component, "%s %s: %s (%d)", prefix, next_context->purpose, gk_result_message(next_context->result), gk_result_code(next_context->result));
        }
        else {
            log_log(LOG_ERROR, __FILE__, __LINE__, context->log_component, "%s %s", prefix, next_context->purpose);
        }

        prefix = "due to failure to";
        next_context = next_context->child_context;
    }
    log_log(LOG_ERROR, __FILE__, __LINE__, context->log_component, "----------------------");
}
                                 
int gk_execution_context_is_success(gk_execution_context *context) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot determine success for NULL execution contxt");
        return 0;
    }
    
    if (context->result == NULL) {
        log_error(COMP_EXCTX, "Cannot determine success of execution context [%s], result is unexpectedly NULL", context->purpose);
        return 0;
    }
    return gk_result_code(context->result) == GK_SUCCESS;
}


size_t gk_execution_context_stack_size(gk_execution_context *context) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot determine stack size for NULL execution context");
        return 0;
    }
    size_t stack_size = 1;
    gk_execution_context *next_context = context;
    while (next_context->child_context != NULL) {
        next_context = next_context->child_context;
        stack_size += 1;
    }
    return stack_size;
}

void gk_execution_context_set_result(gk_execution_context *context, gk_result *result) {
    if (context == NULL) {
        log_error(COMP_EXCTX, "Cannot set result for NULL execution context");
        return;
    }
    gk_result_free(context->result);
    context->result = result;
}
