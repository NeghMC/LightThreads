#include "LightThreads.h"

void ls_criticalStart();
void ls_criticalEnd();

struct contextList
{
	volatile lt_thread_t *first;
	volatile lt_thread_t *last;
}; 

static volatile struct contextList sScheduledList;
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

uint8_t lt_schedule(lt_thread_t *thread, lt_function_t function, void *arg)
{
	thread->function = function;
	thread->arg = arg;
	thread->flag = 0;
	thread->nextPoint = NULL;
	thread->nextThread = NULL;

	ls_criticalStart();

	// schedule
	if(sScheduledList.first == NULL)
	{ // its empty, we insert first one
		sScheduledList.first = thread;
	}
	else
	{
		sScheduledList.last->nextThread = thread;
	}
	sScheduledList.last = thread;
	sScheduledList.last->nextThread = sScheduledList.first;
	
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
	if(current == NULL)
	{
		if(sScheduledList.first == NULL)
		{
			current = NULL;
		}
		else
		{
			current = sScheduledList.first;
		}
	}
	else
	{
		current = current->nextThread;
	}
	ls_criticalEnd();
	
	// process
	if(current != NULL && current->function != NULL)	
	{
		current->function(current, current->arg);
		wasExecuted = 1;
	}
	else
	{
		wasExecuted = 0;
	}

	return wasExecuted;
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