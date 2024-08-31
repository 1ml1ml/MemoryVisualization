#include <assert.h>

#include "MemoryInfo.h"

MemoryDetection::MemoryInfo::MemoryInfo(AllocatedOperate allocatedOperate, const void* address, size_t size) :
	allocatedOperate_{ allocatedOperate }, address_{ reinterpret_cast<uintptr_t>(address) }, size_{ size }
{
	assert(address);
}

bool MemoryDetection::MemoryInfo::isMatchingReleaseOperate(const ReleaseOperate releaseOperate) const
{
	return static_cast<uint8_t>(allocatedOperate_) == static_cast<uint8_t>(releaseOperate);
}

MemoryDetection::NewMemoryInfo::NewMemoryInfo(const void* address, size_t size) : MemoryInfo(AllocatedOperate::New, address, size)
{
}

MemoryDetection::NewArrayMemoryInfo::NewArrayMemoryInfo(const void* address, size_t size) : MemoryInfo(AllocatedOperate::NewArray, address, size)
{
}
