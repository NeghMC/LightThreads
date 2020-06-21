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

typedef struct {
	volatile int count;
} lt_semaphore_t;

struct lt_thread_s {
	lt_task_p task;
	enum lt_flag flag;
	lt_semaphore_t * semaphore;
	void * nextPoint;
	void * context;
};

/* some utilities */
#define _CONCAT(s1, s2) s1##s2
#define LABEL(line) _CONCAT(lt_, line)
#define FORCE_SEMICOLON do{}while(0)
#define LT_NULL 0

#define LT_TASK(name)\
	void name(lt_thread_t * lt_context)
	
#define LT_THREAD_CREATE(taskName, _context)\
	(lt_thread_t){.task = taskName, .flag = LT_YIELDED, .context = _context, .nextPoint = LT_NULL, .semaphore = LT_NULL}

#define LT_START\
	if(lt_context->nextPoint != LT_NULL)\
		goto *(lt_context->nextPoint);\
	FORCE_SEMICOLON
	
#define LT_WAIT_UNTIL(cond)\
	lt_context->nextPoint = &&LABEL(__LINE__);\
	LABEL(__LINE__):\
	if(!(cond)) {\
		lt_context->flag = LT_YIELDED;\
		return;\
	}\
	FORCE_SEMICOLON
		
#define LT_YIELD\
	lt_context->flag = LT_YIELDED;\
	lt_context->nextPoint = &&LABEL(__LINE__);\
	return;\
	LABEL(__LINE__):\
	FORCE_SEMICOLON

#define LT_END\
	lt_context->nextPoint = LT_NULL;\
	lt_context->flag = LT_ENDED;\
	return;\
	FORCE_SEMICOLON
	
#define LT_SEMAPHORE_TAKE(semaphore)\
	if(semaphore.count == 0) {\
		lt_context->semaphore = &semaphore;\
		lt_context->flag = LT_BLOCKED;\
		lt_context->nextPoint = &&LABEL(__LINE__);\
		return;\
	}\
	LABEL(__LINE__):\
	FORCE_SEMICOLON

#define LT_SEMAPHORE_SIGNAL(semaphore)\
	semaphore.count++

#define LT_GET_CONTEXT\
	(lt_context->context)

#define LT_SCHEDULE(threadName)\
	switch(threadName.flag) {\
		case LT_YIELDED:\
			threadName.task(&threadName);\
			break;\
		case LT_BLOCKED:\
			if(threadName.semaphore->count > 0) {\
				threadName.semaphore->count--;\
				threadName.task(&threadName);\
			}\
			break;\
		default:\
			break;\
	}\
	FORCE_SEMICOLON

#endif /* LIGHTTHREADS_H_ */
