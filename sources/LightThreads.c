/*
 * LightThreads
 * 
 * Author: Grzegorz Świstak (NeghMC)
 * 
 * DISCLAIMER: This software is provided 'as-is', without any express
 * or implied warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "lightThreads.h"
#include <stddef.h>

typedef struct {
    lt_context_t *head;
    lt_context_t *tail;
} contextsQueue_t;

static contextsQueue_t sContextsToExecute = {NULL, NULL}; // list of ready to execute threads
static lt_context_t *sScheduledContexts = NULL; // list of the threads scheduled with 'lt_delay(...)'

static void pushToEnd(contextsQueue_t *list, lt_context_t *thread)
{
	thread->nextContext = NULL;
	if(list->head == NULL)
	{
		// its empty, insert first one
		list->head = thread;
	}
	else
	{
		// not empty, insert at the end
		list->tail->nextContext = thread;
	}
	list->tail = thread;
}

static lt_context_t * popFromStart(contextsQueue_t *list)
{
	if(list->head == NULL)
	{
		// empty, return nothing
		return NULL;
	}
	else
	{
		// not empty, return head
		lt_context_t *toReturn = list->head;
		list->head = list->head->nextContext;
		//toReturn->nextContext = NULL;
		return toReturn;
	}
}

void lt_schedule(lt_context_t *context, thread_t function, void *args)
{
	context->function = function;
	context->args = args;
    lt_criticalEnter();
    pushToEnd(&sContextsToExecute, context);
    lt_criticalExit();
}

void lt_runNext() {
    lt_criticalEnter();
    lt_context_t *scheduled = popFromStart(&sContextsToExecute);
    lt_criticalExit();
	if(scheduled != NULL)
	{
    	scheduled->function(scheduled->args);
	}
	else
	{
		lt_idleCallback();
	}
}

void lt_delay(lt_context_t *current, thread_t function, void *args, uint16_t ticks)
{
	// minimum number of ticks must be 1
	if(ticks == 0)
	{
		ticks = 1;
	}

	// update context
	current->nextContext = NULL;
	current->function = function;
	current->args = args;
	current->deltaTicks = ticks;
	// NOTE: field 'deltaTicks' represents not an absolute delay,
	// but delay between previous and the current threads on the list.

	lt_criticalEnter();

	if(sScheduledContexts == NULL)
	{
		// list is empty
		sScheduledContexts = current;
	}
	else
	{
		// There is at least one element in the list,
		if(current->deltaTicks < sScheduledContexts->deltaTicks)
		{
			// current context shall be before that element
			current->nextContext = sScheduledContexts;
			sScheduledContexts = current;
			current->nextContext->deltaTicks -= current->deltaTicks;
		}
		else
		{
			// current context shall be further down the list; we run through the list
			lt_context_t *temp = sScheduledContexts;
			while(1)
			{
				current->deltaTicks -= temp->deltaTicks;
				if(temp->nextContext == NULL)
				{
					// insert at the end
					temp->nextContext = current;
					break;
				}
				else if(current->deltaTicks < temp->nextContext->deltaTicks)
				{
					// insert in between
					current->nextContext = temp->nextContext;
					temp->nextContext = current;
					current->nextContext->deltaTicks -= current->deltaTicks;
					break;
				}
				else
				{
					// next iteration
					temp = temp->nextContext;
				}
			}
		}
	}
	lt_criticalExit();
}

void lt_tick()
{
	if(sScheduledContexts == NULL)
	{
		return;
	}

	sScheduledContexts->deltaTicks--;

	while(sScheduledContexts->deltaTicks == 0u)
	{
		lt_criticalEnter();
		lt_context_t *current = sScheduledContexts;
		sScheduledContexts = sScheduledContexts->nextContext;
		pushToEnd(&sContextsToExecute, current);
		lt_criticalExit();

		if(sScheduledContexts == NULL)
		{
			break;
		}
	}
}
