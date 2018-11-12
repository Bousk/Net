#pragma once

#include <iostream>
#ifdef _MSC_VER
#define DEBUGBREAK() __debugbreak()
#else
// TODO
#define DEBUGBREAK() 
#endif

#define TO_STRING_INTERNAL(token) #token
#define TO_STRING(token) TO_STRING_INTERNAL(token)
#define CHECK_SUCCESS(expr) do { /*std::cout << "[OK] " #expr << std::endl;*/ } while(0)
#define CHECK_FAILURE(expr) do { std::cout << "[FAILURE][" __FILE__ ":" TO_STRING(__LINE__) "] " #expr << std::endl; DEBUGBREAK(); } while(0)
#define CHECK(expr) do { if (expr) { CHECK_SUCCESS(expr); } else { CHECK_FAILURE(expr); } } while(0)