#include "Thread.h"
#include "common/Log.h"
#include <SDL.h>

Thread::Thread (IRunnable* target) :
		_thread(nullptr), _priority(NORM_PRIORITY), _running(false), _name("runnable"), _runnable(target)
{
}

Thread::~Thread ()
{
	terminate();
}

void Thread::start ()
{
	if (!_running) {
		_running = true;
		_thread = SDL_CreateThread(executeThread, _name, _runnable);
		if (_thread == nullptr) {
			Log::error(LOG_THREAD, "failed to create thread: %s", SDL_GetError());
			_running = false;
		}
	}
}

int Thread::executeThread (void* data)
{
	IRunnable* runnable = static_cast<IRunnable*>(data);
	return runnable->run();
}

void Thread::sleep ()
{
}

int Thread::join ()
{
	if (_running) {
		_running = false;
		int returnValue = 0;
		SDL_WaitThread(_thread, &returnValue);
		return returnValue;
	}

	return 0;
}

void Thread::terminate ()
{
	if (_running) {
		join();
	}
}
