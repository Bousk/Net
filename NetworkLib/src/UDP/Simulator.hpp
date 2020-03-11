#pragma once

#include <Settings.hpp>
#include <Address.hpp>
#include <RangedInteger.hpp>
#include <UDP/Datagram.hpp>

#include <chrono>
#include <random>

#if BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED
namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			class Simulator
			{
			public:
				Simulator();

				void enable() { mEnabled = true; }
				void disable() { mEnabled = false; }
				bool isEnabled() const { return mEnabled; }
				// Initialize random generators with the given seed, useful for tests especially
				void seed(unsigned int value);

				void setDuplicate(RangedInteger<1, 10> duplicate) { mDuplicate = duplicate; }
				void setLossRate(RangedInteger<0, 100> loss) { mLoss = loss; }
				void setDelay(RangedInteger<0, 250> fixed, RangedInteger<0, 100> random) { mFixedDelay = fixed; mRandomDelay = random; }

				// Push datagram to the simulator to store them and apply delay etc.
				void push(const Datagram& datagram, const Address& from);
				// Retrieve datagrams ready to be processed. Can return more datagrams than pushed as they can be duplicated.
				std::vector<std::pair<Datagram, Address>> poll();

			private:
				struct PendingDatagram {
					Datagram datagram;
					Address from;
					std::chrono::milliseconds receivedTime;
					std::chrono::milliseconds processTime;
				};
				std::vector<PendingDatagram> mPendingDatagrams;

				RangedInteger<1, 10> mDuplicate{ 3 };
				RangedInteger<0, 1000> mDuplicateRate{ 1 }; // This value should remain small in normal usage as that leads to duplication which can take a lot of extra memory and poor performances
				std::default_random_engine mDuplicateGenerator;
				RangedInteger<0, 100> mLoss{ 5 };
				std::default_random_engine mLossGenerator;
				RangedInteger<0, 250> mFixedDelay{ 0 };
				RangedInteger<0, 100> mRandomDelay{ 50 };
				std::default_random_engine mRandomDelayGenerator;
				bool mEnabled{ false };
			};
		}
	}
}
#endif // BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED