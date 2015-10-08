#pragma once

#include "common/NonCopyable.h"
#include <stddef.h>
#include <map>

namespace {
const size_t WORD_SIZE = sizeof(void*);
}

class IMemoryAllocationObject {
public:
	virtual ~IMemoryAllocationObject ()
	{
	}
};

class MemoryAllocator : public NonCopyable {
private:
	friend void* operator new (size_t nbytes, MemoryAllocator& pool);
	friend void operator delete (void* ptr, MemoryAllocator& pool);

	typedef std::map<void*, size_t> MemoryIndexMap;
	typedef MemoryIndexMap::iterator MemoryIndexMapIter;

	MemoryIndexMap _memIndexMap;
	uint8_t* _memHeap;
	size_t _size;
	size_t _sizeLeft;

	void* allocate (size_t nbytes);
	void deallocate (void* ptr);

	size_t calculateMemorySpace (size_t nbytes) const;

public:
	explicit MemoryAllocator (size_t nbytes);
	~MemoryAllocator ();

	size_t getSizeTotal () const;
	size_t getSizeLeft () const;
};

inline void* operator new (size_t nbytes, MemoryAllocator& pool)
{
	return pool.allocate(nbytes);
}

inline void operator delete (void* ptr, MemoryAllocator& pool)
{
	pool.deallocate(ptr);
}
