#include "TimeManager.h"

TimeManager::TimeManager () :
	_memAllocator(MEMORY_POOL_SIZE)
{
	reset();
}

TimeManager::~TimeManager ()
{
	reset();
}

void TimeManager::update (uint32_t deltaTime)
{
	if (_timeoutMap.empty()) {
		return;
	}

	_invalidateTimeUpdate = false;
	_time += deltaTime;

	for (TimeoutMapIter iter = _timeoutMap.begin(); iter != _timeoutMap.end();) {
		if (iter->execTime <= deltaTime) {
			if (iter->execTime > 0) {
				iter->execTime = 0;
				(*iter->callback)();
			}
			if (_invalidateTimeUpdate) {
				// timeout was created within callback thus invalidating iter
				break;
			}
			operator delete(iter->callback, _memAllocator);
			iter = _timeoutMap.erase(iter);
		} else {
			iter->execTime -= deltaTime;
			++iter;
		}
	}
}

bool TimeManager::clearTimeout (TimerID id)
{
	if (id <= 0)
		return false;

	for (TimeoutMapIter c = _timeoutMap.begin(); c != _timeoutMap.end(); ++c) {
		if (c->id != id)
			continue;
		operator delete(c->callback, _memAllocator);
		c = _timeoutMap.erase(c);
		_invalidateTimeUpdate = true;
		return true;
	}
	return false;
}

void TimeManager::reset ()
{
	_invalidateTimeUpdate = true;
	for (TimeoutMapIter iter = _timeoutMap.begin(); iter != _timeoutMap.end(); ++iter) {
		operator delete(iter->callback, _memAllocator);
	}
	_timeoutMap.clear();
	_time = 0;
	_autoIncID = 0;
}
