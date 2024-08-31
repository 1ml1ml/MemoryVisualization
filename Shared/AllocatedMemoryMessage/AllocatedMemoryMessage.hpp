#pragma once

#include <type_traits>

#include "MemoryInfo/MemoryInfo.h"
#include "MemoryDetection/MemoryDetection.hpp"

namespace MemoryDetection
{
	template<typename T>
	class AllocatedMemoryMessageWith : public AbstractMessageWith<T> {};

	template<> class AllocatedMemoryMessageWith<MemoryInfo*> : public AbstractMessageWith<MemoryInfo*>
	{
	public:
		using AbstractMessageWith::AbstractMessageWith;

	public:
		int id() const override { return info_->id(); }
	};
}
