#pragma once

#include <Types.hpp>

#include <string>
#include <vector>

namespace Bousk
{
	namespace Serialization
	{
		class Serializer
		{
		public:
			Serializer() = default;

			bool write(uint8 data);
			bool write(uint16 data);
			bool write(uint32 data);
			bool write(bool data);
			bool write(int8 data);
			bool write(int16 data);
			bool write(int32 data);
			bool write(float32 data);

			template<class T>
			bool write(const std::vector<T>& data);
			bool write(const std::string& data);

			inline const uint8* buffer() const { return mBuffer.data(); }
			inline size_t bufferSize() const { return mBuffer.size(); }

		private:
			bool writeBytes(const uint8* buffer, size_t nbBytes);
			template<class T>
			bool writeArray(const T* data, uint8 nbElements);

		private:
			std::vector<uint8> mBuffer;
		};
	}
}

#include <Serialization/Serializer.inl>