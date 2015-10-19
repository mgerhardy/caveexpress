#pragma once

#include "common/MemoryAllocator.h"
#include "common/Timer.h"
#include <new>
#include <stdint.h>
#include <stddef.h>
#include <vector>

namespace {
const size_t MEMORY_POOL_SIZE = 2048;
}

class ITimeoutCallback: public IMemoryAllocationObject {
public:
	virtual void operator() () = 0;
};

template<class Class>
class TimeoutCallbackNoParamFunc: public ITimeoutCallback {
public:
	TimeoutCallbackNoParamFunc (Class* object, void(Class::*pFunc) ()) :
		_object(object), _pFunc(pFunc)
	{
	}

	void operator() () override
	{
		(_object->*_pFunc)();
	}

private:
	Class* _object;
	void (Class::*_pFunc) ();
};

template<class Class, class Param>
class TimeoutCallbackFunc: public ITimeoutCallback {
public:
	TimeoutCallbackFunc (Class* object, void(Class::*pFunc) (Param*), Param* param) :
		_object(object), _pFunc(pFunc), _param(param)
	{
	}

	void operator() () override
	{
		(_object->*_pFunc)(_param);
	}

private:
	Class* _object;
	void (Class::*_pFunc) (Param*);
	Param* _param;
};

class TimeManager {
private:
	typedef struct CallbackData {
		ITimeoutCallback* callback;
		// the time delta (ms) that still must be passed until the callback is executed
		uint32_t execTime;
		TimerID id;
	} CallbackData;

	typedef std::vector<CallbackData> TimeoutMap;
	typedef TimeoutMap::iterator TimeoutMapIter;

	MemoryAllocator _memAllocator;
	TimeoutMap _timeoutMap;
	uint32_t _time;
	TimerID _autoIncID;
	bool _invalidateTimeUpdate;

	inline TimerID setTimeout (uint32_t delay, ITimeoutCallback* functor)
	{
		const CallbackData data = { functor, delay, ++_autoIncID };
		_timeoutMap.push_back(data);
		_invalidateTimeUpdate = true;
		return _autoIncID;
	}

public:
	TimeManager ();
	~TimeManager ();

	bool clearTimeout (TimerID id);
	void reset ();
	void update (uint32_t deltaTime);

	template<class Class>
	TimerID resetTimeout (TimerID timerID, uint32_t delay, Class* object, void(Class::*callback) ())
	{
		clearTimeout(timerID);
		ITimeoutCallback* functor = new (_memAllocator) TimeoutCallbackNoParamFunc<Class> (object, callback);
		return setTimeout(delay, functor);
	}

	template<class Class>
	TimerID setTimeout (uint32_t delay, Class* object, void(Class::*callback) ())
	{
		ITimeoutCallback* functor = new (_memAllocator) TimeoutCallbackNoParamFunc<Class> (object, callback);
		return setTimeout(delay, functor);
	}

	template<class Class, class Param>
	TimerID resetTimeout (TimerID timerID, uint32_t delay, Class* object, void(Class::*callback) (Param *), Param* param)
	{
		clearTimeout(timerID);
		ITimeoutCallback* functor = new (_memAllocator) TimeoutCallbackFunc<Class, Param> (object, callback, param);
		return setTimeout(delay, functor);
	}

	template<class Class, class Param>
	TimerID setTimeout (uint32_t delay, Class* object, void(Class::*callback) (Param *), Param* param)
	{
		ITimeoutCallback* functor = new (_memAllocator) TimeoutCallbackFunc<Class, Param> (object, callback, param);
		return setTimeout(delay, functor);
	}
};

