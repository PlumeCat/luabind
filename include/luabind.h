// luabind.h
#pragma once

#ifdef LUABIND_USE_LUAJIT
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luajit.h>
}
#else
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif

#include <jlib/log.h>
using namespace std::literals;

template<typename T> concept stringlike = requires (T t) {
	{ t.size() } -> std::integral;
	{ t.data() } -> std::convertible_to<const char*>;
};

// pushing funcs
void tpush(lua_State* lua, const std::integral auto& value) {
	lua_pushinteger(lua, (lua_Integer)value);
}
void tpush(lua_State* lua, const std::floating_point auto& value) {
	lua_pushnumber(lua, (lua_Number)value);
}
void tpush(lua_State* lua, const char* value) {
	lua_pushstring(lua, value);
}
void tpush(lua_State* lua, const stringlike auto& value) {
	lua_pushlstring(lua, value.data(), value.size());
}
void tpush(lua_State* lua, void* value) {
	lua_pushlightuserdata(lua, value);
}

// popping funcs
template<std::same_as<bool> T> T tpop(lua_State* lua, int n) {
	return T { (bool)lua_toboolean(lua, n) };
}
template<std::integral T> T tpop(lua_State* lua, int n) {
	return (T)lua_tointeger(lua, n);
}
template<std::floating_point T> T tpop(lua_State* lua, int n) {
	return (T)lua_tonumber(lua, n);
}
template<std::same_as<const char*> T> T tpop(lua_State* lua, int n) {
	return T { lua_tostring(lua, n) };
}
template<std::constructible_from<const char*, int> T> T tpop(lua_State* lua, int n) {
	auto size = 0ul;
	const auto* str = lua_tolstring(lua, n, &size);
	return T { str, size };
}
template<std::constructible_from<const void*> T> T tpop(lua_State* lua, int n) {
	return T { lua_topointer(lua, n) };
}
// {cfunction, userdata, thread}

template<int N, typename ... Ts> void tpop(lua_State* lua, std::tuple<Ts...>& values) {
	if constexpr(N < sizeof...(Ts)) {
		constexpr auto M = sizeof...(Ts) - (N+1);
		std::get<M>(values) = tpop<std::tuple_element_t<M, std::tuple<Ts...>>>(lua, -(N+1));
		tpop<N+1, Ts...>(lua, values);
	}
}
template<typename... Ts> std::tuple<Ts...> tpop(lua_State* lua) {
	auto retval = std::tuple<Ts...>{};
	tpop<0, Ts...>(lua, retval);
	lua_pop(lua, (int)sizeof...(Ts));
	return retval;
}

const char* error_str(int res);

class ScriptEngine {
public:
	ScriptEngine();
	~ScriptEngine();

	void dofile(std::string_view fname);

	template<typename... return_types>
	std::tuple<return_types...> call(const char* ns, const char* name, const auto&... args) {
		using return_type = std::tuple<return_types...>;
		constexpr auto nargs = (int)sizeof...(args);
		constexpr auto nret = (int)sizeof...(return_types);
		
		// push func
		// lua_getglobal(lua, name);
		lua_getglobal(lua, ns);
		if (!lua_istable(lua, -1)) {
			log("not a namespace", ns);
			return {};
		}

		lua_getfield(lua, -1, name);
		if (!lua_isfunction(lua, -1)) {
			log("not a function", ns, name);
			return {};
		}
		
		// push arguments ...
		(tpush(lua, args), ...);

		if (const auto res = lua_pcall(lua, nargs, nret, errpos); res != LUA_OK) {
			// an error occurred; log it and pop it; error handler already added traceback
			log<true, false>(Colors::FG_RED, error_str(res), Colors::FG_RED2, lua_tostring(lua, -1), Colors::FG_DEFAULT);
			lua_pop(lua, 1);
			return {};
		}
		
		// pop returns ...
		return tpop<return_types...>(lua);
	}

private:
	lua_State* lua;
	int errpos; // error handler here always
};


#ifdef LUABIND_IMPLEMENTATION

#include <filesystem>
namespace fs = std::filesystem;

static int panic_func(lua_State* lua) {
	log<true, false>(Colors::FG_RED, "LUA PANIC!", Colors::FG_DEFAULT);
	return 0;
}
static int error_handler(lua_State* lua) {
	luaL_traceback(lua, lua, (std::string(lua_tostring(lua, -1)) + Colors::FG_YELLOW2).c_str(), 0);
	return 1;
}
static int lua_exception_wrapper(lua_State* lua, lua_CFunction f) {
	// log<false>("FUNCWRAPPER");
	try {return f(lua);}
	catch (const char* c) {lua_pushstring(lua, ("c++ error"s + c).c_str());}
	catch (std::exception& e) {lua_pushstring(lua, ("c++ exception"s + e.what()).c_str());}
	catch (...) {lua_pushliteral(lua, "unknown error");}
	return lua_error(lua);
}

const char* error_str(int res) {
	switch (res) {
		case LUA_ERRRUN: 	return "runtime error:";
		case LUA_ERRERR: 	return "error handler error:";
		case LUA_ERRFILE: 	return "filesystem error:";
		case LUA_ERRMEM: 	return "memory error:";
		case LUA_ERRSYNTAX: return "syntax error:";
	}
	return "";
}

// make new namespace object and leave it on top of the stack
static void make_namespace(lua_State* lua, std::string_view name) {
	// _G[name] = setmetatable({}, { __index = _G })

	lua_newtable(lua);
	lua_newtable(lua);
	#ifdef LUABIND_USE_LUAJIT
	lua_pushvalue(lua, LUA_GLOBALSINDEX);
	#else
	lua_pushglobaltable(lua);
	#endif
	lua_setfield(lua, -2, "__index");
	lua_setmetatable(lua, -2);
	// lua_pushvalue(lua, -1);
	lua_setglobal(lua, name.data());
	lua_getglobal(lua, name.data());
}

ScriptEngine::ScriptEngine() {
	lua = luaL_newstate();
	lua_atpanic(lua, panic_func);
	lua_pushcfunction(lua, error_handler);
	errpos = lua_gettop(lua);
	luaL_openlibs(lua);

	#ifdef LUABIND_USE_LUAJIT
		luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);
		lua_pushlightuserdata(lua, (void*)lua_exception_wrapper);
		luaJIT_setmode(lua, -1, LUAJIT_MODE_WRAPCFUNC|LUAJIT_MODE_ON);
		lua_pop(lua, 1);
	#endif
}
ScriptEngine::~ScriptEngine() {
	// pop error handler
	lua_pop(lua, 1);
	lua_close(lua);
}
void ScriptEngine::dofile(std::string_view path) {
	const auto ns = std::filesystem::path(path).stem().c_str();
	const auto nargs = 0;
	const auto nret = 0;

	// load file
	if (const auto res = luaL_loadfilex(lua, path.data(), nullptr); res != LUA_OK) {
		// loadfile error
		log("execute loadfile error: ", error_str(res), lua_tostring(lua, -1));
		lua_pop(lua, 1);
		return;
	}

	// make namespace
	make_namespace(lua, ns);
	if (!lua_setupvalue(lua, -2, 1)) {
		lua_pop(lua, 1);
	}

	if (const auto res = lua_pcall(lua, nargs, nret, errpos); res != LUA_OK) {
		// pcall error
		log("execute pcall error: ", error_str(res), lua_tostring(lua, -1));
		lua_pop(lua, 1);
	}
}
#endif

