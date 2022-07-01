#include "LightThreads.h"

void lt_criticalStart();
void lt_criticalEnd();

#ifdef LT_USE_IDLE_HANDLER
	void lt_idleHandler();
#endif


static volatile lt_threadsList_t sScheduledList;
static volatile lt_thread_t *sDelayed;


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

uint8_t lt_taskCreate(lt_thread_t *thread, lt_function_t function, void *arg)
{
	thread->function = function;
	thread->arg = arg;
	thread->flag = LT_READY;
	thread->nextPoint = NULL;
	thread->nextThread = NULL;

	lt_criticalStart();
	pushToEnd(&sScheduledList, thread); // schedule
	lt_criticalEnd();

	return 0;
}

uint8_t lt_handle()
{
	static volatile lt_thread_t *current;
	uint8_t wasExecuted = 0;

	lt_criticalStart();
	current = popFromStart(&sScheduledList);
	lt_criticalEnd();

	// process
	if(current != NULL)
	{
		if(current->function != NULL)	
		{
			current->function(current, current->arg);

			// after processing
			if(current->flag == LT_READY)
			{
				lt_criticalStart();
				pushToEnd(&sScheduledList, current);
				lt_criticalEnd();
			}
			wasExecuted = 1;
		}
	}
#ifdef LT_USE_IDLE_HANDLER
	else
	{
		lt_idleHandler();
	}
#endif

	return wasExecuted;
}

#ifdef LT_USE_SEMAPHORES

uint8_t lt_semaphoreTake(lt_semaphoreBinary_t *sem, lt_thread_t *thread)
{
	if(sem != NULL)
	{
		lt_criticalStart();
		if(sem->taken)
		{
			pushToEnd(&sem->waiting, thread);
			thread->flag = LT_BLOCKED;
		}
		else
		{
			sem->taken = 1;
		}
		lt_criticalEnd();
	}
}

uint8_t lt_semaphoreGive(lt_semaphoreBinary_t *sem)
{
	if(sem != NULL)
	{
		lt_criticalStart();
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
		lt_criticalEnd();
	}
}

#endif

#ifdef LT_USE_DELAY

uint8_t lt_delay(uint16_t ticks, lt_thread_t *thread)
{
	if(ticks != 0)
	{
		thread->delay = ticks;
		thread->flag = LT_BLOCKED;

		lt_criticalStart();
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
		lt_criticalEnd();
	}
	return ticks;
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
			current->flag = LT_READY;

			if(sDelayed == NULL)
			{
				break;
			}
		}
	}
}
#endif

#ifdef LT_USE_NOTIFICATIONS

uint8_t lt_notifyTake(lt_thread_t *thread)
{
	if(thread != NULL && thread->flag == LT_READY)
	{
		thread->flag = LT_BLOCKED;
	}
	return 0;
}
uint8_t lt_notifyGive(lt_thread_t *thread)
{
	if(thread != NULL && thread->flag == LT_BLOCKED)
	{
		lt_criticalStart();
		pushToEnd(&sScheduledList, thread);
		thread->flag = LT_READY;
		lt_criticalEnd();
	}
	return 0;
}

#endif
