#include "timer.h"
#include <unistd.h>
#include <stdexcept>


Timer::Timer(uint32_t prodindex, senderMetadata* sendmeta, bool& rmState)
{
	trigger(prodindex, sendmeta, rmState);
}


Timer::~Timer()
{
}


void Timer::trigger(uint32_t prodindex, senderMetadata* sendmeta, bool& rmState)
{
    unsigned int timeoutSec, timeoutuSec;
	RetxMetadata* perProdMeta = sendmeta->getMetadata(prodindex);
	if (perProdMeta != NULL)
	{
	    timeoutSec = (unsigned int) perProdMeta->retxTimeoutPeriod;
	    timeoutuSec = (unsigned int) (perProdMeta->retxTimeoutPeriod -
	                                    timeoutSec) * 1000000;
		sleep(timeoutSec);
		usleep(timeoutuSec);
		rmState = sendmeta->rmRetxMetadata(prodindex);
	}
	else
	    throw std::runtime_error("Timer::trigger() get RetxMetadata error");
}
