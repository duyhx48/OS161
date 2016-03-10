/*
 * Copyright (c) 2001, 2002, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Driver code is in kern/tests/synchprobs.c We will
 * replace that file. This file is yours to modify as you see fit.
 *
 * You should implement your solution to the whalemating problem below.
 */

#include <types.h>
#include <lib.h>
#include <thread.h>
#include <test.h>
#include <synch.h>

/*
 * Called by the driver during initialization.
 */


 /* we will need 3 semaphores for the 3 actors and their respective counters*/
struct semaphore *male_sem_e;
struct semaphore *fem_sem_e;
struct semaphore *mm_sem_e;

struct semaphore *male_sem_l;
struct semaphore *fem_sem_l;
struct semaphore *mm_sem_l;

void whalemating_init() {
	 
	 // mutex = lock_create("Mutex");
	 
	 male_sem_e = sem_create("Male Enter",0);
	 fem_sem_e = sem_create("Female Enter",0);
	 mm_sem_e = sem_create("Matchmaker Enter",0);

	 male_sem_l = sem_create("Male Leave",0);
	 fem_sem_l  = sem_create("Female Leave",0);
	 mm_sem_l   = sem_create("Matchmaker Leave",0);

	return;
}

/*
 * Called by the driver during teardown.
 */

void
whalemating_cleanup() {

	sem_destroy(male_sem_e);
	sem_destroy(fem_sem_e);
	sem_destroy(mm_sem_e);
	sem_destroy(male_sem_l);
	sem_destroy(fem_sem_l);
	sem_destroy(mm_sem_l);

	return;
}

void
male(uint32_t index)
{

  male_start(index);

  V(male_sem_e);
  P(fem_sem_l);

  male_end(index);
  
  return;
	
}

void
female(uint32_t index)
{

  female_start(index);

  V(fem_sem_e);
  P(male_sem_l);

  female_end(index);
 
  return;
}

void
matchmaker(uint32_t index)
{
	matchmaker_start(index);

	P(male_sem_e);
	P(fem_sem_e);


	V(male_sem_l);
	V(fem_sem_l);
	
    matchmaker_end(index);
  
 return;
}
