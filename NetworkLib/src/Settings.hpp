#pragma once

#include <chrono>

// Enable serialization of float32
//#define BOUSKNET_ALLOW_FLOAT32_SERIALIZATION

#define BOUSKNET_DEFAULT_UDP_TIMEOUT std::chrono::seconds(1)