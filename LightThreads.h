/*
 * LightThreads.h
 *
 * Created: 24.04.2020 18:13:48
 * Author: Grzegorz Świstak
 */ 

#include <stdint.h>
#include "LightThreadsConfig.h"

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
	LT_READY,
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
#ifdef LT_USE_DELAY
	uint16_t delay;
#endif
};

/* some utilities */
#define _CONCAT(s1, s2) s1##s2
#define LABEL(line) _CONCAT(lt_, line)
#define YIELD_POINT lt_context->nextPoint = &&LABEL(__LINE__); return; LABEL(__LINE__):;

#define LT_TASK(name)\
	void name(lt_thread_t *lt_context, void *arg)

#define LT_START\
	do {\
		if(lt_context->nextPoint != NULL)\
			goto *(lt_context->nextPoint);\
	} while (0)
	
	
		
#define LT_YIELD\
	do {\
		YIELD_POINT\
	} while (0)

#define LT_END\
	do {\
		lt_context->flag = LT_INACTIVE;\
		return;\
	} while (0)
	
#ifdef LT_USE_SEMAPHORES
	uint8_t lt_semaphoreTake(lt_semaphoreBinary_t *sem, lt_thread_t *thread);
	uint8_t lt_semaphoreGive(lt_semaphoreBinary_t *sem);

	#define LT_SEMAPHORE_TAKE(_semaphore)\
		do {\
		lt_semaphoreTake(&_semaphore, lt_context);\
		YIELD_POINT\
		} while (0)

	#define LT_SEMAPHORE_GIVE(_semaphore)\
		do {\
		lt_semaphoreGive(&_semaphore);\
		YIELD_POINT\
		} while (0)
#endif

#ifdef LT_USE_DELAY
	uint8_t lt_delay(uint16_t time, lt_thread_t *thread);
	void lt_tick();

	#define LT_DELAY(delay)\
		do {\
			lt_delay(delay, lt_context);\
			YIELD_POINT\
		} while (0)
#endif

#ifdef LT_USE_NOTIFICATIONS
	uint8_t lt_notifyTake(lt_thread_t *thread);
	uint8_t lt_notifyGive(lt_thread_t *thread);

	#define LT_NOTIFY_TAKE()\
		do {\
			lt_notifyTake(lt_context);\
			YIELD_POINT\
		} while (0)

	#define LT_NOTIFY_GIVE(thread)\
		do {\
			lt_notifyGive(&thread);\
			YIELD_POINT\
		} while (0)
#endif
	
uint8_t lt_taskCreate(lt_thread_t *thread, lt_function_t function, void *arg);
uint8_t lt_handle();
	
#ifdef __cplusplus
}
#endif

#endif /* LIGHTTHREADS_H_ */
