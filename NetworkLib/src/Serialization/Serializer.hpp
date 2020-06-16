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
		class Serializer
		{
			friend class Serialization_Test;
		public:
			Serializer() = default;

			bool write(const Serializable& serializable);

			bool write(uint8 data, uint8 minValue, uint8 maxValue);
			bool write(uint16 data, uint16 minValue, uint16 maxValue);
			bool write(uint32 data, uint32 minValue, uint32 maxValue);
			bool write(uint64 data, uint64 minValue, uint64 maxValue);
			inline bool write(uint8 data) { return write(data, std::numeric_limits<uint8>::min(), std::numeric_limits<uint8>::max()); }
			inline bool write(uint16 data) { return write(data, std::numeric_limits<uint16>::min(), std::numeric_limits<uint16>::max()); }
			inline bool write(uint32 data) { return write(data, std::numeric_limits<uint32>::min(), std::numeric_limits<uint32>::max()); }
			inline bool write(uint64 data) { return write(data, std::numeric_limits<uint64>::min(), std::numeric_limits<uint64>::max()); }

			bool write(int8 data, int8 minValue, int8 maxValue);
			bool write(int16 data, int16 minValue, int16 maxValue);
			bool write(int32 data, int32 minValue, int32 maxValue);
			bool write(int64 data, int64 minValue, int64 maxValue);
			inline bool write(int8 data) { return write(data, std::numeric_limits<int8>::min(), std::numeric_limits<int8>::max()); }
			inline bool write(int16 data) { return write(data, std::numeric_limits<int16>::min(), std::numeric_limits<int16>::max()); }
			inline bool write(int32 data) { return write(data, std::numeric_limits<int32>::min(), std::numeric_limits<int32>::max()); }
			inline bool write(int64 data) { return write(data, std::numeric_limits<int64>::min(), std::numeric_limits<int64>::max()); }

			inline bool write(bool data) { return write(data ? BoolTrue : BoolFalse, static_cast<uint8>(0), static_cast<uint8>(1)); }

			// To use if your enum has Min & Max entries
			template<class E>
			typename std::enable_if<std::is_enum<E>::value, bool>::type write(E value)
			{
				using T = typename std::underlying_type<E>::type;
				return write(value, E::Min, E::Max);
			}
			// Use this one to use custom min & max enum values
			template<class E>
			typename std::enable_if<std::is_enum<E>::value, bool>::type write(E value, E min, E max)
			{
				using T = typename std::underlying_type<E>::type;
				return write(static_cast<T>(value), static_cast<T>(min), static_cast<T>(max));
			}

		#if BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED
			bool write(float32 data);
		#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION == BOUSKNET_SETTINGS_ENABLED

			template<class T>
			inline bool write(const std::vector<T>& data) { return writeContainer(data); }
			inline bool write(const std::string& data) { return writeContainer(data); }

			inline const uint8* buffer() const { return mBuffer.data(); }
			inline size_t bufferSize() const { return mBuffer.size(); }

		private:
			bool writeBits(const uint8* buffer, const uint8 buffersize, const uint8 nbBits);
			template<class CONTAINER>
			bool writeContainer(const CONTAINER& container);

			// For std::string support
			inline bool write(char data) { return write(*reinterpret_cast<uint8*>(&data)); }

		private:
			std::vector<uint8> mBuffer;
			uint8 mUsedBits{ 0 };
		};
	}
}

#include <Serialization/Serializer.inl>
