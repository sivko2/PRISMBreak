#ifndef PRISMTHREAD_HPP_
#define PRISMTHREAD_HPP_


#include <QThread>


namespace pronic { namespace prism { // NAMESPACE


class PrismThread : public QThread
{
	public:

		static void sleep(unsigned long secs);
		static void msleep(unsigned long msecs);
		static void usleep(unsigned long usecs);

};


} } // NAMESPACE


#endif
