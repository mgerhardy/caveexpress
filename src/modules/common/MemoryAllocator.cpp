#include "MemoryAllocator.h"
#include "common/String.h"
#include "common/Log.h"
#include "common/System.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <SDL.h>

MemoryAllocator::MemoryAllocator (size_t nbytes) :
	_memIndexMap(), _memHeap(nullptr), _size(nbytes), _sizeLeft(nbytes)
{
	_memHeap = (uint8_t*) SDL_malloc(_size);
}

MemoryAllocator::~MemoryAllocator ()
{
	SDL_free(_memHeap);
}

void* MemoryAllocator::allocate (size_t nbytes)
{
	const size_t memsize = calculateMemorySpace(nbytes);
	uint8_t* placement = _memHeap;
	_sizeLeft -= memsize;
	for (MemoryIndexMapIter iter = _memIndexMap.begin(); iter != _memIndexMap.end(); ++iter) {
		uint8_t* mem = (uint8_t*) iter->first;
		ptrdiff_t diff = mem - placement;
		if (diff >= static_cast<ptrdiff_t>(memsize)) {
			_memIndexMap[placement] = memsize;
			return placement;
		}
		placement = mem + iter->second;
	}
	if (placement + memsize <= _memHeap + _size) {
		_memIndexMap[placement] = memsize;
		return placement;
	}
	System.exit("fatal: out of pool memory!", EXIT_FAILURE);
	return nullptr;
}

void MemoryAllocator::deallocate (void* ptr)
{
	IMemoryAllocationObject* object = reinterpret_cast<IMemoryAllocationObject*>(ptr);
	object->~IMemoryAllocationObject();
	MemoryIndexMapIter iter = _memIndexMap.find(ptr);
	_sizeLeft += calculateMemorySpace(iter->second);
	_memIndexMap.erase(iter);
}

size_t MemoryAllocator::getSizeTotal () const
{
	return _size;
}

size_t MemoryAllocator::getSizeLeft () const
{
	return _sizeLeft;
}

size_t MemoryAllocator::calculateMemorySpace (size_t nbytes) const
{
	const size_t alignExcess = nbytes % WORD_SIZE;
	size_t memsize = nbytes;
	if (alignExcess > 0)
		return memsize + (WORD_SIZE - alignExcess);
	return memsize;
}
