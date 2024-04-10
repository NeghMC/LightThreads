#include <stdio.h>
#include "../../sources/LightThreads.h"
#include <time.h>

//#define DEBUG

void lt_criticalEnter() {}
void lt_criticalExit() {}
void lt_idleCallback() {}

lt_context_t task1;
void task1fun(void *arg);
lt_context_t task2;
void task2fun(void *arg);

void task1fun(void *arg)
{
    static int counter = 0;
    printf("task1: %d\n", counter++);
    lt_delay(&task1, task1fun, NULL, 2);
}

void task2fun(void *arg)
{
    static int counter = 0;
    printf("task2: %d\n", counter++);
    lt_delay(&task2, task2fun, NULL, 4);
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    lt_schedule(&task1, task1fun, NULL);
    lt_schedule(&task2, task2fun, NULL);

#ifdef DEBUG
    while(1)
    {
        for(int i = 0; i < 5; ++i)
        {
           lt_runNext(); 
        }
        lt_tick();
    }
#else
    time_t current = time(NULL);
    while(1)
    {
        if(time(NULL) - current >= 1)
        {
            current = time(NULL);
            lt_tick();
        }
        lt_runNext();
    }
#endif
}