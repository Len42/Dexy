#pragma once

// No Windows headers required
//#include <SDKDDKVer.h>
//#define _WIN32_WINNT _WIN32_WINNT_WIN10
//#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
//#include <windows.h>

namespace Version
{
	enum class Compiler { other, msc, msclang, clang, gcc };

	constexpr Compiler compilerId =
#if defined(__clang__)
#if defined(_MSC_VER)
		Compiler::msclang;
#else
		Compiler::clang;
#endif
#elif defined(__GNUC__)
		Compiler::gcc;
#elif defined(_MSC_VER)
		Compiler::msc;
#else
		Compiler::other;
#endif

	consteval const char* CompilerName()
	{
		switch (compilerId) {
			using enum Compiler;
		case other:		return "?";
		case msc:		return "Microsoft";
		case msclang:	return "MS/Clang";
		case clang:		return "Clang";
		case gcc:		return "gcc";
		}
	}

	constexpr unsigned cppVersion = __cplusplus / 100 % 100;

	constexpr const char* compilerName = CompilerName();

	#define sym_to_string_helper(X) #X
	#define sym_to_string(X) sym_to_string_helper(X)

	constexpr const char* compilerBuildConfig = sym_to_string(BUILD_CONFIG);

} // namespace Version
