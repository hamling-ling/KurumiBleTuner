#ifndef _STOPWATCH_H_
#define _STOPWATCH_H_

class StopWatch
{
public:
	StopWatch(const char* msg);
	~StopWatch();
private:
	char _msg[16];
	const unsigned long _start;
};

#endif
