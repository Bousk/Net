#include <UDP/Simulator.hpp>

#include <algorithm>

#if BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED
namespace Bousk
{
	namespace Network
	{
		namespace UDP
		{
			Simulator::Simulator()
			{
				// Initialize with a default seed
				seed(0);
			}
			void Simulator::seed(unsigned int value)
			{
				// TODO : Log seed value
				mDuplicateGenerator.seed(value);
				mLossGenerator.seed(value);
				mRandomDelayGenerator.seed(value);
			}

			void Simulator::push(const Datagram& datagram, const Address& from)
			{
				assert(isEnabled());
				const bool isLost = std::uniform_int_distribution(1, 100)(mLossGenerator) < mLoss.get();
				if (isLost)
					return;

				using namespace std::chrono_literals;
				const auto now = Utils::Now();
				const bool shouldDuplicate = std::uniform_int_distribution(1, static_cast<int>(mDuplicateRate.Max()))(mDuplicateGenerator) < mDuplicateRate.get();
				const unsigned int nbCopies = shouldDuplicate ? std::uniform_int_distribution(static_cast<int>(mDuplicate.Min()), static_cast<int>(mDuplicate.get()))(mDuplicateGenerator) : 1;
				PendingDatagram copy;
				copy.datagram = datagram;
				copy.from = from;
				copy.receivedTime = now;
				for (unsigned int i = 0; i < nbCopies; ++i)
				{
					copy.processTime = now + mFixedDelay.get() * 1ms + std::uniform_int_distribution(static_cast<int>(mRandomDelay.Min()), static_cast<int>(mRandomDelay.get()))(mRandomDelayGenerator) * 1ms;
					mPendingDatagrams.push_back(copy);
				}
			}
			std::vector<std::pair<Datagram, Address>> Simulator::poll()
			{
				std::stable_sort(mPendingDatagrams.begin(), mPendingDatagrams.end(), [](const PendingDatagram& left, const PendingDatagram& right) { return left.processTime < right.processTime; });

				auto firstNotToReturn = std::upper_bound(mPendingDatagrams.cbegin(), mPendingDatagrams.cend(), Utils::Now(), [](std::chrono::milliseconds now, const PendingDatagram& entry) { return now < entry.processTime; });
				std::vector<std::pair<Datagram, Address>> returned;
				returned.reserve(firstNotToReturn - mPendingDatagrams.cbegin());
				for (auto it = mPendingDatagrams.cbegin(); it != firstNotToReturn; ++it)
				{
					returned.emplace_back(it->datagram, it->from);
				}
				mPendingDatagrams.erase(mPendingDatagrams.begin(), firstNotToReturn);
				return returned;
			}
		}
	}
}
#endif // BOUSKNET_ALLOW_NETWORK_SIMULATOR == BOUSKNET_SETTINGS_ENABLED