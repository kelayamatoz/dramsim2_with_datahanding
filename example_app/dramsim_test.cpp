/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland 
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*  
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*  
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*  
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include "dramsim_test.h"

using namespace DRAMSim;

/* callback functors */
void some_object::read_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	printf("[Callback] read complete: %d 0x%lx cycle=%lu\n", id, address, clock_cycle);
}

void some_object::write_complete(unsigned id, uint64_t address, uint64_t clock_cycle)
{
	printf("[Callback] write complete: %d 0x%lx cycle=%lu\n", id, address, clock_cycle);
}

/* FIXME: this may be broken, currently */
void power_callback(double a, double b, double c, double d)
{
	printf("power callback: %0.3f, %0.3f, %0.3f, %0.3f\n",a,b,c,d);
}

int some_object::add_one_and_run_serial(MultiChannelMemorySystem *mem, uint64_t addr)
{

  /* write 100 consecutive entries */
  uint64_t address = 0x900000;
  for (int i = 0; i < 100; i ++)
  {
    mem->addTransaction(true, address);
    address += 64;
  }

	for (int i=0; i<500; i++)
	{
		mem->update();
	}

  for (int i = 0; i < 100; i ++)
  {
    mem->addTransaction(false, address);
    address += 64;
  }

	for (int i=0; i<500; i++)
	{
		mem->update();
	}

	mem->printStats(true);
	return 0;
}

int some_object::add_one_and_run_random(MultiChannelMemorySystem *mem, uint64_t addr)
{
  std::vector<uint32_t> vec;
  /* write to random addresses */
  for (int i = 0; i < 100; i ++)
  {
    int randAddrOffset = rand() % 64;
    int addr = randAddrOffset * 147456;
    vec.push_back(addr);
    mem->addTransaction(true, addr);
  }

	for (int i=0; i<500; i++)
	{
		mem->update();
	}

  /* randomly read back the numbers */
  for (int i = 0; i < 100; i ++)
  {
    int addr = vec.at(i);
    mem->addTransaction(false, addr);
  }

	for (int i=0; i<45; i++)
	{
		mem->update();
	}

	/* get a nice summary of this epoch */
	mem->printStats(true);

	return 0;
}

int main()
{
	some_object obj;
	TransactionCompleteCB *read_cb = new Callback<some_object, void, unsigned, uint64_t, uint64_t>(&obj, &some_object::read_complete);
	TransactionCompleteCB *write_cb = new Callback<some_object, void, unsigned, uint64_t, uint64_t>(&obj, &some_object::write_complete);

	/* pick a DRAM part to simulate */
  MultiChannelMemorySystem *mem =
    getMemorySystemInstance("ini/DDR3_micron_32M_8B_x4_sg125.ini",
        "system.ini", "..", "example_app_serial", 32768);
  mem->setCPUClockSpeed(622222222);
	mem->RegisterCallbacks(read_cb, write_cb, power_callback);

	printf("dramsim_test main()\n");
	printf("-----MEM1------\n");
	obj.add_one_and_run_serial(mem, 0x100001UL);
	return 0;
}

