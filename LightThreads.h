#include <stdint.h>

typedef struct {
    void *nextFunction;
    void *args;
    // private - user shall not modify or read that field:
    void *nextContext;
} context_t;

typedef void * thread_t(void *);

enum lt_error
{
    LT_OK,
    LT_SCHEDULER_NULL_FUNCTION,
    LT_SCHEDULE_INCORRECT_CONTEXT
};

int lt_schedule(context_t *context);
int lt_schedulerHandler();