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

template<typename R, typename...A>
static void bind(lua_State* lua, std::string_view name, R (*func)(A...)) {
	log("bind function: ");
	lua_setglobal(lua, name.data());
}

int main() {
	log<true, false>(Colors::FG_CYAN2, "=== LUABIND ===", Colors::FG_DEFAULT);

	auto engine = std::make_unique<ScriptEngine>();

	// load main file
	engine->dofile("../../scripts/main.lua");
	engine->dofile("../../scripts/foo.lua");

	// call function
	const auto r = engine->call<int, float, std::string>("main", "bar", "hello"s, 2.1, "world"sv, 400, -12, 0, 0.0, "from c++");
	log(r);
	
	// call another function
	// const auto [ i, s, f, i2, s2 ] = engine->call<int, float, std::string, float, int>("main", "bar");
	// log(i, s, f, i2, s2);

	engine->call("main", "foo2");
	engine->call("foo", "foo1");

	return 0;
}