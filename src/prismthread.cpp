
#include "prismthread.hpp"


namespace pronic { namespace prism { // NAMESPACE


void PrismThread::sleep(unsigned long secs)
{
	QThread::sleep(secs);
}


void PrismThread::msleep(unsigned long msecs)
{
	QThread::msleep(msecs);
}


void PrismThread::usleep(unsigned long usecs)
{
	QThread::usleep(usecs);
}


} } // NAMESPACE
