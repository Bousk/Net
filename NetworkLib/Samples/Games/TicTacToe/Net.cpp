#include <Net.hpp>
#include <Serialization/Deserializer.hpp>
#include <Serialization/Serializer.hpp>

namespace TicTacToe
{
	namespace Net
	{
		bool Play::write(Bousk::Serialization::Serializer& stream) const
		{
			return stream.write(x)
				&& stream.write(y);
		}
		bool Play::read(Bousk::Serialization::Deserializer& stream)
		{
			return stream.read(x)
				&& stream.read(y);
		}
	}
}