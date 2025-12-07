// main.cpp

#define LUABIND_IMPLEMENTATION
#include <luabind.h>

#define JLIB_IMPLEMENTATION
#include <jlib/log.h>

#include <string>
#include <string_view>
using namespace std::literals;

#define log(...) log<true, false>(Colors::FG_CYAN2, "INFO:", Colors::FG_DEFAULT, __VA_ARGS__)
#define warn(...) log<true, false>(Colors::FG_YELLOW, "WARNING:", Colors::FG_DEFAULT, __VA_ARGS__)

// int quz(int a, float b, const char* c) {
// 	log("quz: ", a, b, c ? c : "--");
// 	return 12;
// }

int main() {
	log<true, false>(Colors::FG_CYAN2, "=== LUABIND ===", Colors::FG_DEFAULT);

	auto engine = std::make_unique<ScriptEngine>();
	
	// load main file
	engine->dofile("../../scripts/main.lua");
	engine->dofile("../../scripts/foo.lua");

	return 0;
}