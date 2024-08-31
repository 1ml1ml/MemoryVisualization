#pragma once

#include <assert.h>

#include <mutex>
#include <atomic>
#include <sstream>
#include <unordered_map>

#include "boost/archive/binary_oarchive.hpp"
#include "boost/serialization/shared_ptr.hpp"

#include "FileMappingIO/FileMappingIO.h"
#include "AllocatedMemoryMessage/AllocatedMemoryMessage.hpp"

class MemoryDetectionImpl
{
private:
	using MemoryInfoPtr = std::shared_ptr<MemoryDetection::MemoryInfo>;

private:
	struct EnableGuard
	{
	public:
		EnableGuard(MemoryDetectionImpl* memoryDetection) : memoryDetection_{ memoryDetection } 
		{
			memoryDetection_->mutex_.lock();
			enable_ = memoryDetection_->enable_;
			memoryDetection_->enable_ = false;
		}
		~EnableGuard()
		{
			memoryDetection_->enable_ = enable_;
			memoryDetection_->mutex_.unlock();
		}

	public:
		operator bool() { return enable_; }

	private:
		MemoryDetectionImpl* memoryDetection_;
		bool enable_{ false };
	};

public:
	MemoryDetectionImpl();

public:
	void onOperatorNew(const void* address, size_t size);
	void onOperatorNewArray(const void* address, size_t size);

	void onAllocated(MemoryInfoPtr memoryInfo);
	void onRelease(const void* address, size_t size, MemoryDetection::MemoryInfo::ReleaseOperate releaseOperate);

private:
	bool enable_{ false };
	std::recursive_mutex mutex_{};
	std::unordered_map<uintptr_t, MemoryInfoPtr> memoryInfos_{};

	FileMappingIO fileMappingIO_{};
} hook;

MemoryDetectionImpl::MemoryDetectionImpl()
{
	{
		auto pid{ std::to_string(GetCurrentProcessId()) };
		fileMappingIO_.init("file:" + pid, 1024, "signal:" + pid, "mutex:" + pid, std::ios::in | std::ios::out);
	}
	enable_ = true;
}

void MemoryDetectionImpl::onOperatorNew(const void* address, size_t size)
{
	if (MemoryDetectionImpl::EnableGuard guard{ this }) onAllocated(std::make_shared<MemoryDetection::NewMemoryInfo>(address, size));
}

void MemoryDetectionImpl::onOperatorNewArray(const void* address, size_t size)
{
	if (MemoryDetectionImpl::EnableGuard guard{ this }) onAllocated(std::make_shared<MemoryDetection::NewArrayMemoryInfo>(address, size));
}

void MemoryDetectionImpl::onAllocated(MemoryInfoPtr memoryInfo)
{
	assert(memoryInfo);

	memoryInfos_.insert({ memoryInfo->address_, memoryInfo });

	std::stringstream ss{};
	boost::archive::binary_oarchive oa{ ss };
	oa << memoryInfo;

	auto data{ ss.str() };
	fileMappingIO_.write(data.data(), data.size());
}

void MemoryDetectionImpl::onRelease(const void* address, size_t size, MemoryDetection::MemoryInfo::ReleaseOperate releaseOperate)
{
	if (MemoryDetectionImpl::EnableGuard guard{ this }; !guard) return;

	if (auto find{ memoryInfos_.find(reinterpret_cast<uintptr_t>(address)) }; find != memoryInfos_.end())
	{
		if (const auto& memoryInfo{ find->second }; memoryInfo->isMatchingReleaseOperate(releaseOperate))
		{
			if (memoryInfo->size_ != size)
			{
			}
		}
		else
		{

		}
	}
	else
	{

	}
}

extern "C++" void* operator new(size_t size)
{
	if (auto address{ malloc(size) })
	{
		hook.onOperatorNew(address, size);
		return address;
	}
	throw std::bad_alloc{};
}

extern "C++" void operator delete(void* address, size_t size) noexcept
{
	hook.onRelease(address, size, MemoryDetection::MemoryInfo::ReleaseOperate::Delete);
	free(address);
}

extern "C++" void* operator new[](size_t size)
{
	if (auto address{ malloc(size) })
	{
		hook.onOperatorNewArray(address, size);
		return address;
	}
	throw std::bad_alloc{};
}

extern "C++" void operator delete[](void* address, size_t size) noexcept
{
	hook.onRelease(address, size, MemoryDetection::MemoryInfo::ReleaseOperate::DeleteArray);
	free(address);
}

