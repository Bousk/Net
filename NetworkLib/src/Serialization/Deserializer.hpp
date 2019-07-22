#pragma once

#include <Types.hpp>

#include <string>
#include <vector>

namespace Bousk
{
	namespace Serialization
	{
		class Deserializer
		{
		public:
			Deserializer(const uint8* buffer, const size_t bufferSize)
				: mBuffer(buffer)
				, mBufferSize(bufferSize)
			{}

			bool read(uint8& data);
			bool read(uint16& data);
			bool read(uint32& data);
			bool read(bool& data);
			bool read(int8& data);
			bool read(int16& data);
			bool read(int32& data);
			bool read(float32& data);

			template<class T>
			bool read(std::vector<T>& data) { return readContainer(data); }
			bool read(std::string& data) { return readContainer(data); }

			inline size_t remainingBytes() const { return mBufferSize - mBytesRead; }

		private:
			bool readBytes(size_t nbBytes, uint8* buffer);
			template<class CONTAINER>
			bool readContainer(CONTAINER& container);

			// For std::string support
			bool read(char& data) { return read(reinterpret_cast<uint8&>(data)); }

		private:
			const uint8* mBuffer;
			const size_t mBufferSize;
			size_t mBytesRead{ 0 };
		};

	}
}

#include <Serialization/Deserializer.inl>