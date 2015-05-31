#include "tests/MemoryAllocatorTest.h"
#include "common/Logger.h"
#include <stdint.h>

TEST(MemoryAllocatorTest, testAllocateObject)
{
	struct TestMemObject: public IMemoryAllocationObject {
		void* pv;
		int i;
		char c;

		size_t size () const
		{
			return sizeof(*this);
		}
	};

	const size_t size = sizeof(TestMemObject);
	MemoryAllocator memAlloc(size);
	TestMemObject* object = new (memAlloc) TestMemObject();
	ASSERT_EQ(memAlloc.getSizeLeft(), (size_t) 0);
	operator delete(object, memAlloc);
	ASSERT_EQ(memAlloc.getSizeTotal(), memAlloc.getSizeLeft());
}
