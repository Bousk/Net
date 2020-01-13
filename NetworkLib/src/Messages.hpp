#pragma once
#include "Address.hpp"

#include <cassert>
#include <numeric>
#include <vector>

namespace Bousk
{
	namespace Network
	{
		namespace Messages
		{
			#define DECLARE_MESSAGE(name) friend class Base; static const Base::Type StaticType = Base::Type::name
			class Base
			{
			public:
				template<class M>
				bool is() const { return mType == M::StaticType; }
				template<class M>
				const M* as() const { assert(is<M>()); return static_cast<const M*>(this); }

				const Address& emitter() const { return mEmitter; }
				uint64 emmiterId() const { return mEmitterId; }

			protected:
				enum class Type {
					IncomingConnection,
					Connection,
					Disconnection,
					UserData,
				};
				Base(Type type, const Address& emitter, uint64 emitterid)
					: mType(type)
					, mEmitter(emitter)
					, mEmitterId(emitterid)
				{}
			private:
				Address mEmitter;
				uint64 mEmitterId;
				Type mType;
			};
			class IncomingConnection : public Base
			{
				DECLARE_MESSAGE(IncomingConnection);
			public:
				IncomingConnection(const Address& emitter, uint64 emitterid)
					: Base(Type::IncomingConnection, emitter, emitterid)
				{}
			};
			class Connection : public Base
			{
				DECLARE_MESSAGE(Connection);
			public:
				enum class Result {
					Success,
					Failed,
				};
				Connection(const Address& emitter, uint64 emitterid, Result r)
					: Base(Type::Connection, emitter, emitterid)
					, result(r)
				{}
				Result result;
			};
			class Disconnection : public Base
			{
				DECLARE_MESSAGE(Disconnection);
			public:
				enum class Reason {
					Disconnected,
					Lost,
				};
				Disconnection(const Address& emitter, uint64 emitterid, Reason r)
					: Base(Type::Disconnection, emitter, emitterid)
					, reason(r)
				{}
				Reason reason;
			};
			class UserData : public Base
			{
				DECLARE_MESSAGE(UserData);
			public:
				UserData(const Address& emitter, uint64 emitterid, std::vector<unsigned char>&& d)
					: Base(Type::UserData, emitter, emitterid)
					, data(std::move(d))
				{}
				std::vector<unsigned char> data;
			};
			#undef DECLARE_MESSAGE
		}
	}
}