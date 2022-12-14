#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>

// boolean
using Bool = bool;

// Float and double
using Float = float_t;
using Double = double_t;

// signed  integers
using Int8 = int8_t;
using Int16 = int16_t;
using Int32 = int32_t;
using Int64 = int64_t;

// unsigned integers
using Uint8 = uint8_t;
using Uint16 = uint16_t;
using Uint32 = uint32_t;
using Uint64 = uint64_t;

// signed char
using AnsiChar = char;
using Char8 = char8_t;
using Char16 = char16_t;
using Char32 = char32_t;

// integer vector types
using Ivec2 = glm::ivec2;
using Ivec3 = glm::ivec3;
using Ivec4 = glm::ivec4;

// floating point vector types
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;

// unsigned integer vector types
using Uvec2 = glm::uvec2;
using Uvec3 = glm::uvec3;
using Uvec4 = glm::uvec4;

// string
using String = std::string;


// buffers
using ByteBufferDynamic = std::vector<char>;
template<Uint64 BUFFER_SIZE>
using ByteBuffer = std::array<char, BUFFER_SIZE>;