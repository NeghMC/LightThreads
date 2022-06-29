#include "LightThreads.h"

void ls_criticalStart();
void ls_criticalEnd();


static volatile lt_threadsList_t sScheduledList;
static volatile lt_thread_t *sDelayed;
static lt_thread_t *sIdle;


static void pushToEnd(lt_threadsList_t *list, lt_thread_t *thread)
{
	if(thread != NULL)
	{
		if(list->first == NULL)
		{ // its empty, we insert first one
			list->first = thread;
		}
		else
		{
			list->last->nextThread = thread;
		}
		list->last = thread;
	}
}

static lt_thread_t * popFromStart(lt_threadsList_t *list)
{
	lt_thread_t *toReturn;

	if(list->first == NULL)
	{
		toReturn = NULL;
	}
	else
	{
		toReturn = list->first;
		list->first = list->first->nextThread;
		toReturn->nextThread = NULL;
	}

	return toReturn;
}

uint8_t lt_schedule(lt_thread_t *thread, lt_function_t function, void *arg)
{
	thread->function = function;
	thread->arg = arg;
	thread->flag = LT_INACTIVE;
	thread->nextPoint = NULL;
	thread->nextThread = NULL;

	ls_criticalStart();
	pushToEnd(&sScheduledList, thread); // schedule
	ls_criticalEnd();

	return 0;
}

/*
void ls_assignIdleTask(ls_idleTask_t *fun)
{
	ls_criticalStart();
	sIdle = fun;
	ls_criticalEnd();
}
*/

uint8_t lt_handle()
{
	static volatile lt_thread_t *current;
	uint8_t wasExecuted;

	ls_criticalStart();
	current = popFromStart(&sScheduledList);
	ls_criticalEnd();

	// process
	if(current != NULL && current->function != NULL)	
	{
		current->function(current, current->arg);

		// after processing
		switch(current->flag)
		{
			case LT_YIELDED:
				ls_criticalStart();
				pushToEnd(&sScheduledList, current);
				ls_criticalEnd();
				break;
			default:
				break;
		}
		wasExecuted = 1;
	}
	else
	{
		wasExecuted = 0;
	}

	return wasExecuted;
}

#ifdef LT_USE_SEMAPHORES

uint8_t lt_semaphoreTake(lt_semaphoreBinary_t *sem, lt_thread_t *thread)
{
	if(sem != NULL)
	{
		ls_criticalStart();
		if(sem->taken)
		{
			pushToEnd(&sem->waiting, thread);
		}
		else
		{
			sem->taken = 1;
		}
		ls_criticalEnd();
	}
}

uint8_t lt_semaphoreGive(lt_semaphoreBinary_t *sem)
{
	if(sem != NULL)
	{
		ls_criticalStart();
		if(sem->taken)
		{
			lt_thread_t *thread =  popFromStart(&sem->waiting);
			if(thread == NULL)
			{
				sem->taken = 0;
			}
			else
			{
				pushToEnd(&sScheduledList, thread);
			}
		}
		ls_criticalEnd();
	}
}

#endif

#ifdef LT_USE_DELAY

void lt_delay(uint16_t ticks, lt_thread_t *thread)
{
	if(ticks == 0)
	{
		return;
	}
	thread->delay = ticks;

	ls_criticalStart();
	{
		if(sDelayed == NULL)
		{
			sDelayed = thread;
		}
		else if(thread->delay < sDelayed->delay)
		{
			thread->nextThread = sDelayed;
			sDelayed = thread;
			thread->nextThread->delay -= thread->delay;
		}
		else
		{
			volatile lt_thread_t *temp = sDelayed;
			while(1)
			{
				thread->delay -= temp->delay;
				if(temp->nextThread == NULL)
				{
					temp->nextThread = thread;
					break;
				}
				else if(thread->delay < temp->nextThread->delay)
				{
					thread->nextThread = temp->nextThread;
					temp->nextThread = thread;
					thread->nextThread->delay -= thread->delay;
					break;
				}
				else
				{
					temp = temp->nextThread;
				}
			}
		}
	}
	ls_criticalEnd();
}

void lt_tick()
{
	volatile lt_thread_t *current;

	if(sDelayed != NULL)
	{
		sDelayed->delay--;
		while(sDelayed->delay == 0u)
		{
			current = sDelayed;
			sDelayed = sDelayed->nextThread;
			current->nextThread = NULL;
			
			// schedule
			pushToEnd(&sScheduledList, current);

			if(sDelayed == NULL)
			{
				break;
			}
		}
	}
}
#endif
