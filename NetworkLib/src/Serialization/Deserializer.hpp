#pragma once

#include <Settings.hpp>
#include <Types.hpp>

#include <string>
#include <vector>

class Serialization_Test;
namespace Bousk
{
	namespace Serialization
	{
		class Serializable;
		class Deserializer
		{
			friend class Serialization_Test;
		public:
			Deserializer(const uint8* buffer, const size_t bufferSize)
				: mBuffer(buffer)
				, mBufferSize(bufferSize)
			{}

			bool read(Serializable& data);

			bool read(uint8& data, uint8 minValue, uint8 maxValue);
			bool read(uint16& data, uint16 minValue, uint16 maxValue);
			bool read(uint32& data, uint32 minValue, uint32 maxValue);
			bool read(uint64& data, uint64 minValue, uint64 maxValue);
			inline bool read(uint8& data) { return read(data, std::numeric_limits<uint8>::min(), std::numeric_limits<uint8>::max()); }
			inline bool read(uint16& data) { return read(data, std::numeric_limits<uint16>::min(), std::numeric_limits<uint16>::max()); }
			inline bool read(uint32& data) { return read(data, std::numeric_limits<uint32>::min(), std::numeric_limits<uint32>::max()); }
			inline bool read(uint64& data) { return read(data, std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::max()); }
			
			bool read(int8& data, int8 minValue, int8 maxValue);
			bool read(int16& data, int16 minValue, int16 maxValue);
			bool read(int32& data, int32 minValue, int32 maxValue);
			bool read(int64& data, int64 minValue, int64 maxValue);
			inline bool read(int8& data) { return read(data, std::numeric_limits<int8>::min(), std::numeric_limits<int8>::max()); }
			inline bool read(int16& data) { return read(data, std::numeric_limits<int16>::min(), std::numeric_limits<int16>::max()); }
			inline bool read(int32& data) { return read(data, std::numeric_limits<int32>::min(), std::numeric_limits<int32>::max()); }
			inline bool read(int64& data) { return read(data, std::numeric_limits<int64>::min(), std::numeric_limits<int64>::max()); }

			bool read(bool& data);

			template<class E>
			typename std::enable_if<std::is_enum<E>::value, bool>::type read(E& data)
			{
				using T = std::underlying_type<E>::type;
				T temp{};
				if (read(temp, static_cast<T>(E::Min), static_cast<T>(E::Max)))
				{
					data = static_cast<E>(temp);
					return true;
				}
				return false;
			}

		#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			bool read(float32& data);
		#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED

			template<class T>
			inline bool read(std::vector<T>& data) { return readContainer(data); }
			inline bool read(std::string& data) { return readContainer(data); }

		private:
			bool readBits(uint8 nbBits, uint8* buffer, uint8 bufferSize);
			template<class CONTAINER>
			bool readContainer(CONTAINER& container);

			inline size_t remainingBytes() const { return mBufferSize - mBytesRead; }
			inline size_t bufferSizeBits() const { return mBufferSize * 8; }
			inline size_t bufferReadBits() const { return mBytesRead * 8 + mBitsRead; }
			inline size_t remainingBits() const { return bufferSizeBits() - bufferReadBits(); }

			// For std::string support
			inline bool read(char& data) { return read(reinterpret_cast<uint8&>(data)); }

		private:
			const uint8* mBuffer;
			const size_t mBufferSize;
			size_t mBytesRead{ 0 };
			uint8 mBitsRead{ 0 };
		};

	}
}

#include <Serialization/Deserializer.inl>