#include "Messages.hpp"

#include <iostream>
std::ostream& operator<<(std::ostream& out, Bousk::Network::Messages::Connection::Result result)
{
	switch (result)
	{
		case Bousk::Network::Messages::Connection::Result::Success: out << "success"; break;
		case Bousk::Network::Messages::Connection::Result::Failed: out << "failed"; break;
		case Bousk::Network::Messages::Connection::Result::Refused: out << "refused"; break;
		case Bousk::Network::Messages::Connection::Result::TimedOut: out << "timed out"; break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, Bousk::Network::Messages::Disconnection::Reason reason)
{
	switch (reason)
	{
		case Bousk::Network::Messages::Disconnection::Reason::Disconnected: out << "Disconnected"; break;
		case Bousk::Network::Messages::Disconnection::Reason::Lost: out << "Lost"; break;
	}
	return out;
}