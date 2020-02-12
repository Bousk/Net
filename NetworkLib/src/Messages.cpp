#include "Messages.hpp"

#include <iostream>
std::ostream& operator<<(std::ostream& out, Bousk::Network::Messages::Disconnection::Reason reason)
{
	switch (reason)
	{
		case Bousk::Network::Messages::Disconnection::Reason::Disconnected: out << "Disconnected"; break;
		case Bousk::Network::Messages::Disconnection::Reason::Lost: out << "Lost"; break;
	}
	return out;
}