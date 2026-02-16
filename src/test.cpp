// main.cpp

#define LUABIND_IMPLEMENTATION
#include <luabind.h>

#define JLIB_IMPLEMENTATION
#include <jlib/log.h>

#include <string>
#include <tuple>
#include <vector>
#include <string_view>
#include <cmath>
using namespace std::literals;

#define log(...) log<true, false>(Colors::FG_CYAN2, "INFO:", Colors::FG_DEFAULT, __VA_ARGS__)
#define warn(...) log<true, false>(Colors::FG_YELLOW, "WARNING:", Colors::FG_DEFAULT, __VA_ARGS__)

int quz(int a, float b, const char* c) {
	// log("quz: ", a, b, c ? c : "--");
	return 12;
}

void vff(float a, float b) {
	// log(a, b);
}

std::tuple<int, float> multi(std::string bar) {
	// log("BAR: ", bar);
	return std::tuple { 32, 64.8 };
}

#include <filesystem>


struct Example {
	std::string name;
};

std::string example_name(Example* e) {
	return e->name;
}
void set_name(Example* e, std::string_view name) {
	e->name = name;
}

auto C = Example { "global" };

int main() {
	log<true, false>(Colors::FG_CYAN2, "=== LUABIND ===", Colors::FG_DEFAULT);

	log("STEM: ", std::filesystem::path("input.lua").stem().c_str());

	auto engine = std::make_unique<ScriptEngine>();
	engine->set_path("../../scripts/");

	engine->bind_func("multi", LUAFUNC(multi));
	engine->bind_func("sqrtf", LUAFUNC(sqrtf));
	engine->bind_func("quz", LUAFUNC(quz));
	engine->bind_func("vff", LUAFUNC(vff));

	engine->bind_func("lambda", LUAFUNC([](int x, float y) -> float {
		log("test lambda", x, y);
		return x + y;
	}));

	engine->bind_func("returns_vec", LUAFUNC([]() {
		return std::vector { 2, 4, 6, 7, 3 };
	}));

	engine->bind_func("takes_bool", LUAFUNC([](bool x){}));

	engine->bind_func("returns_bool", LUAFUNC([](){ return bool(rand() & 1); }));

	// engine->bind_func("vff", [](lua_State* lua) {
	// 	std::apply(vff, lb_args<typename func_info<decltype(vff)>::args_tuple>(lua)); return 0;\
	// });
	
	engine->bind_func("example_name", LUAFUNC(example_name));
	engine->bind_func("set_name", LUAFUNC(set_name));

	// load main file
	engine->dofile("main.lua");
	engine->dofile("foo.lua");

	engine->call("foo", "bar_func");

	auto A = Example { "hello" };
	auto B = Example { "world" };

	engine->call("foo", "add_example", (void*)&A);
	engine->call("foo", "add_example", (void*)&B);
	engine->call("foo", "add_example", (void*)&C);

	engine->call("foo", "foo1");

	auto [ a, b ] = engine->call<const void*, float>("foo", "foo2", 1, 2.4, "hello");
	log("ab", a, b);

	return 0;
}
