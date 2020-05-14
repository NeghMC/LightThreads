/*
 * LightThreads.h
 *
 * Created: 24.04.2020 18:13:48
 * Author: Grzegorz Świstak
 */ 

#include <stdint.h>

#ifndef LIGHTTHREADS_H_
#define LIGHTTHREADS_H_

enum lt_flag {
	LT_YIELDED,
	LT_BLOCKED,
	LT_SUSPENDED,
	LT_ENDED
};

typedef struct lt_thread_s lt_thread_t;
typedef void (*lt_task_p)(lt_thread_t*); // pointer to function

struct lt_thread_s {
	lt_task_p task;
	void * nextPoint;
	void * context;
	enum lt_flag flag;
};

typedef struct {
	volatile int count;
} lt_semaphore_t;


/* some utilities */
#define _CONCAT(s1, s2) s1##s2
#define LABEL(line) _CONCAT(lt_, line)

#ifndef NULL
	#define NULL 0
#endif

#define LT_TASK(name) \
	void name(lt_thread_t * lt_context)
	
#define LT_THREAD_CREATE(taskName, context) \
	(lt_thread_t){taskName, NULL, context, 0}

#define LT_START								\
		do {									\
			if(lt_context->nextPoint != NULL) {	\
				goto *(lt_context->nextPoint);	\
			}									\
		} while(0)
	
#define LT_WAIT_UNTIL(cond)													\
		do {																\
			LABEL(__LINE__): lt_context->nextPoint = &&LABEL(__LINE__);		\
			if(!(cond)) {													\
				lt_context->flag = LT_BLOCKED;								\
				return;														\
			}																\
		} while(0)
		
#define LT_YIELD \
	lt_context->nextPoint = &&LABEL(__LINE__); lt_context->flag = LT_YIELDED; return; LABEL(__LINE__):

#define LT_END							\
	do {								\
		lt_context->nextPoint = NULL;	\
		lt_context->flag = LT_ENDED;	\
		return;							\
	} while(0)
	
#define LT_SEMAPHORE_WAIT(semaphore)	\
	LT_WAIT_UNTIL(semaphore.count > 0);	\
	semaphore.count--

#define LT_SEMAPHORE_SIGNAL(semaphore)	\
	semaphore.count++

#define LT_CONTEXT	(lt_context->context)

#define LT_SCHEDULE(threadName) \
	threadName.task(&threadName)

#endif /* LIGHTTHREADS_H_ */