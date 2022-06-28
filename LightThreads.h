/*
 * LightThreads.h
 *
 * Created: 24.04.2020 18:13:48
 * Author: Grzegorz Świstak
 */ 

#include <stdint.h>

#ifndef LIGHTTHREADS_H_
#define LIGHTTHREADS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void*)0u)
#endif

enum lt_flag {
	LT_INACTIVE,
	LT_YIELDED,
	LT_BLOCKED
};

typedef struct lt_thread lt_thread_t;
typedef void (*lt_function_t)(struct lt_thread*, void*); // pointer to function
typedef struct lt_threadsList lt_threadsList_t;

struct lt_threadsList
{
	volatile lt_thread_t *first;
	volatile lt_thread_t *last;
}; 
typedef struct {
	volatile uint8_t taken;
	lt_threadsList_t waiting;
} lt_semaphoreBinary_t;


struct lt_thread {
	lt_function_t function;
	void * arg;

	enum lt_flag flag;
	void * nextPoint;
	lt_thread_t *nextThread;
};

/* some utilities */
#define _CONCAT(s1, s2) s1##s2
#define LABEL(line) _CONCAT(lt_, line)
#define FORCE_SEMICOLON do{}while(0)

#define LT_TASK(name)\
	void name(lt_thread_t *lt_context, void *arg)

#define LT_START\
	if(lt_context->nextPoint != NULL)\
		goto *(lt_context->nextPoint);\
	FORCE_SEMICOLON
		
#define LT_YIELD\
	lt_context->flag = LT_YIELDED;\
	lt_context->nextPoint = &&LABEL(__LINE__);\
	return;\
	LABEL(__LINE__):\
	FORCE_SEMICOLON

#define LT_END\
	lt_context->nextPoint = NULL;\
	lt_context->flag = LT_INACTIVE;\
	return;\
	FORCE_SEMICOLON
	

#define LT_SEMAPHORE_TAKE(_semaphore)\
	lt_semaphoreTake(&_semaphore, lt_context);\
	lt_context->flag = LT_BLOCKED;\
	lt_context->nextPoint = &&LABEL(__LINE__);\
	return;\
	LABEL(__LINE__):\
	FORCE_SEMICOLON

#define LT_SEMAPHORE_GIVE(_semaphore)\
	lt_semaphoreGive(&_semaphore);\
	FORCE_SEMICOLON

/*

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
*/
	
	
uint8_t lt_schedule(lt_thread_t *thread, lt_function_t function, void *arg);
//void ls_assignIdleTask(ls_idleTask_t *fun);
uint8_t lt_handle();
uint8_t lt_semaphoreTake(lt_semaphoreBinary_t *sem, lt_thread_t *thread);
uint8_t lt_semaphoreGive(lt_semaphoreBinary_t *sem);
	
#ifdef __cplusplus
}
#endif

#endif /* LIGHTTHREADS_H_ */
