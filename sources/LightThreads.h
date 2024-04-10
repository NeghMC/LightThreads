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

#ifndef LT_HEADER
#define LT_HEADER

#include "context.h"

void lt_schedule(lt_context_t *context, thread_t function, void *args); // schedule thread
void lt_runNext(); // run next scheduled thread

void lt_delay(lt_context_t *context, thread_t function, void *args, uint16_t ticks); // schedule thread after 'tick' ticks
void lt_tick(); // increase system tick (call inside timer interrupt)

// !!! TO IMPLEMENT !!!
void lt_criticalEnter(void); // this function shall disable interrupts
void lt_criticalExit(void); // this function shall restore interrupts state
void lt_idleCallback(void); // this function gets called when no other threads are scheduled

#endif // LT_HEADER