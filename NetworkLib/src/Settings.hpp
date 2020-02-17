#pragma once

#include <chrono>

// Enable serialization of float32
#ifndef BOUSKNET_ALLOW_FLOAT32_SERIALIZATION
	//#define BOUSKNET_ALLOW_FLOAT32_SERIALIZATION
#endif // BOUSKNET_ALLOW_FLOAT32_SERIALIZATION

// Default UDP timeout
#ifndef BOUSKNET_DEFAULT_UDP_TIMEOUT
	#define BOUSKNET_DEFAULT_UDP_TIMEOUT std::chrono::seconds(1)
#endif // BOUSKNET_DEFAULT_UDP_TIMEOUT
