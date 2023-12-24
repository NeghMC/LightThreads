#include "lightThreads.h"

#define LT_NULL ((void*)0)

#if !defined(LT_PRIORITIES)
#define LT_PRIORITIES 1
#endif // LT_PRIORITIES

extern void lt_criticalEnter(void);
extern void lt_criticalExit(void);

typedef struct {
    context_t *head;
    context_t *tail;
} contextsQueue_t;

static contextsQueue_t mainQueue = {LT_NULL, LT_NULL};

static void pushToEnd(contextsQueue_t *list, context_t *thread)
{
	if(thread != LT_NULL && list != LT_NULL)
	{
		if(list->head == LT_NULL)
		{ // its empty, we insert first one
			list->head = thread;
		}
		else
		{
			list->tail->nextContext = thread;
		}
		list->tail = thread;
	}
}

static context_t * popFromStart(contextsQueue_t *list)
{
	context_t *toReturn;

	if((list == LT_NULL) || (list->head == LT_NULL))
	{
		toReturn = LT_NULL;
	}
	else
	{
		toReturn = list->head;
		list->head = list->head->nextContext;
		toReturn->nextContext = LT_NULL;
	}

	return toReturn;
}

int lt_schedule(context_t *context) {
    if(context->nextContext != LT_NULL)
    {
        return LT_SCHEDULE_INCORRECT_CONTEXT;
    }
    lt_criticalEnter();
    pushToEnd(&mainQueue, context);
    lt_criticalExit();
    return LT_OK;
}

int lt_schedulerHandler() {
    lt_criticalEnter();
    context_t *currentContext = popFromStart(&mainQueue);
    lt_criticalExit();
    if(currentContext == LT_NULL)
    {
        return 0;
    }
    thread_t *functionToCall = (thread_t*)currentContext->nextFunction;
    if(functionToCall == LT_NULL)
    {
        // something went wrong, the context should have been removed
        return LT_SCHEDULER_NULL_FUNCTION;
    }
    currentContext->nextFunction = functionToCall(currentContext->args);
    if(currentContext->nextFunction != LT_NULL)
    {
        lt_criticalEnter();
        pushToEnd(&mainQueue, currentContext);
        lt_criticalExit();   
    }
    return 0;
}