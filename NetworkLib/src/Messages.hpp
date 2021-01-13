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
					ConnectionInterrupted,
					ConnectionResumed,
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
					Refused,
					TimedOut,
				};
				Connection(const Address& emitter, uint64 emitterid, Result r)
					: Base(Type::Connection, emitter, emitterid)
					, result(r)
				{}
				Result result;
			};
		#if BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
			class ConnectionInterrupted : public Base
			{
				DECLARE_MESSAGE(ConnectionInterrupted);
			public:
				ConnectionInterrupted(const Address& emitter, uint64 emitterid, bool isDirect)
					: Base(Type::ConnectionInterrupted, emitter, emitterid)
					, isDirectInterruption(isDirect)
				{}
				// True if the emitter is directly interrupted to us. False if the emitter forwarded an interruption on his side.
				bool isDirectInterruption;
			};
			class ConnectionResumed : public Base
			{
				DECLARE_MESSAGE(ConnectionResumed);
			public:
				ConnectionResumed(const Address& emitter, uint64 emitterid, bool networkResume)
					: Base(Type::ConnectionResumed, emitter, emitterid)
					, isNetworkResumed(networkResume)
				{}
				// True if the network is now completely resumed. False if network is not yet resumed due to another client being interrupted.
				bool isNetworkResumed;
			};
		#endif // BOUSKNET_ALLOW_NETWORK_INTERRUPTION == BOUSKNET_SETTINGS_ENABLED
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

#include <iosfwd>
std::ostream& operator<<(std::ostream& out, Bousk::Network::Messages::Connection::Result result);
std::ostream& operator<<(std::ostream& out, Bousk::Network::Messages::Disconnection::Reason reason);