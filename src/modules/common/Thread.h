#pragma once

#include "common/IRunnable.h"
#include "common/NonCopyable.h"
#include <SDL_thread.h>

class Thread: public NonCopyable {
public:
	typedef enum Priority {
		NORM_PRIORITY = 0, MAX_PRIORITY, MIN_PRIORITY
	} Priority;

	explicit Thread (IRunnable* target);
	virtual ~Thread ();

	void start ();
	int join ();
	void sleep ();
	void terminate ();

protected:
	SDL_Thread* _thread;
	Priority _priority;
	bool _running;
	const char* _name;
	IRunnable *_runnable;

	static int executeThread (void* data);
};
