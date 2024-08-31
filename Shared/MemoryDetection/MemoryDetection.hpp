#pragma once

#include "boost/serialization/access.hpp"
#include "boost/serialization/base_object.hpp"

namespace MemoryDetection
{
#define MESSAGE_ID(i) public: static inline int ID{ i }; int id() const override{ return ID; }

	class AbstractMessage
	{
		friend class boost::serialization::access;

	public:
		virtual ~AbstractMessage() = default;

	public: 
		virtual int id() const = 0;

	private:
		template<typename Archive>
		void serialize(Archive& ar, const unsigned int version) { ar& id(); }
	};

	template<typename T>
	class AbstractMessageWith : public AbstractMessage
	{
		friend class boost::serialization::access;

	public:
		AbstractMessageWith(const T& info) : AbstractMessage(), info_{ info } {} 

	private:
		template<typename Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar& boost::serialization::base_object<AbstractMessage>(*this);
			ar& info_;
		}

	public:
		const T info_;
	};
}
