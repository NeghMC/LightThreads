#include "LightThreads.h"

void ls_criticalStart();
void ls_criticalEnd();


static volatile lt_threadsList_t sScheduledList;
/*
static volatile struct ls_context *sDelayed;
static ls_idleTask_t *sIdle;
*/
/*
void ls_init (void *buffer)
{
	sFreeList.first = (volatile struct ls_context *)buffer;
	uint16_t i = 0;
	while (i < (LS_SCHEDULER_CAPACITY - 1))
    {
		sFreeList.first[i].function = NULL;
		sFreeList.first[i].arg = NULL;
        sFreeList.first[i].next = &sFreeList.first[i+1];
		i++;
    }
    sFreeList.first[i].next = NULL;
	sFreeList.last = &sFreeList.first[i];
}
*/

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

/*
#ifndef LS_DONT_USE_DELAY

uint8_t ls_scheduleDelayed(ls_function *fun, void *arg, uint16_t ticks)
{
	volatile struct ls_context *current;
	uint8_t error;

	if(ticks == 0)
	{
		ticks = 1;
	}

	ls_criticalStart();
	// is free list empty
	if(sFreeList.first == NULL)
	{
		error = 1;
	}
	else
	{
		error = 0;

		// take
		current = sFreeList.first;
		sFreeList.first = sFreeList.first->next;
		current->next = NULL;

		// update
		current->function = fun;
		current->arg = arg;
		current->delay = ticks;

		if(sDelayed == NULL)
		{
			sDelayed = current;
		}
		else if(current->delay < sDelayed->delay)
		{
			current->next = sDelayed;
			sDelayed = current;
			current->next->delay -= current->delay;
		}
		else
		{
			volatile struct ls_context *temp = sDelayed;
			while(1)
			{
				current->delay -= temp->delay;
				if(temp->next == NULL)
				{
					temp->next = current;
					break;
				}
				else if(current->delay < temp->next->delay)
				{
					current->next = temp->next;
					temp->next = current;
					current->next->delay -= current->delay;
					break;
				}
				else
				{
					temp = temp->next;
				}
			}
		}
	}
	ls_criticalEnd();
	return error;
}

void ls_tickISR()
{
	volatile struct ls_context *current;

	if(sDelayed != NULL)
	{
		sDelayed->delay--;
		while(sDelayed->delay == 0u)
		{
			current = sDelayed;
			sDelayed = sDelayed->next;
			current->next = NULL;
			
			// schedule
			if(sScheduledList.first == NULL)
			{ // its empty, we insert first one
				sScheduledList.first = current;
			}
			else
			{
				sScheduledList.last->next = current;
			}
			sScheduledList.last = current;

			if(sDelayed == NULL)
			{
				break;
			}
		}
	}
}
#endif
*/