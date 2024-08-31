#pragma once

#include <memory>

#include "boost/serialization/access.hpp"
#include "boost/serialization/export.hpp";
#include "boost/serialization/base_object.hpp"

namespace MemoryDetection
{
#define MEMORY_INFO_ID(i) public: static inline int ID{ i }; int id() const override{ return ID; }
	class MemoryInfo
	{
		friend class boost::serialization::access;

	public:
		enum class AllocatedOperate : uint8_t
		{
			New,
			NewArray,
		};

		enum class ReleaseOperate : uint8_t
		{
			Delete = AllocatedOperate::New,
			DeleteArray = AllocatedOperate::NewArray,
		};

	public:
		explicit MemoryInfo(AllocatedOperate allocatedOperate, const void* address, size_t size);
		virtual ~MemoryInfo() = default;

	public:
		virtual int id() const = 0;

	public:
		bool isMatchingReleaseOperate(const ReleaseOperate releaseOperate) const;

	private:
		template<typename Archive>
		void serialize(Archive& ar, const unsigned int version) { ar& allocatedOperate_& address_& size_; }

	public:
		const AllocatedOperate allocatedOperate_;
		const uintptr_t address_;
		const size_t size_;
	};

	class NewMemoryInfo : public MemoryInfo
	{
		friend class boost::serialization::access;

		MEMORY_INFO_ID(0x01)

	public:
		NewMemoryInfo(const void* address, size_t size);
	};

	class NewArrayMemoryInfo : public MemoryInfo
	{
		friend class boost::serialization::access;

		MEMORY_INFO_ID(0x02)

	public:
		NewArrayMemoryInfo(const void* address, size_t size);
	};
}

BOOST_CLASS_EXPORT(MemoryDetection::MemoryInfo)
BOOST_CLASS_EXPORT(MemoryDetection::NewMemoryInfo)
BOOST_CLASS_EXPORT(MemoryDetection::NewArrayMemoryInfo)
