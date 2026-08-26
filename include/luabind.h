// luabind.h
#pragma once

#include <type_traits>
#include <string>
#include <vector>

#ifdef LUABIND_USE_LUAJIT
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <luajit.h>
}
#else
#include <lua.hpp>
#include <lualib.h>
#include <lauxlib.h>
#endif

#include <jlib/log.h>
using namespace std::literals;

/*
TODO: handle pushing/popping of:
 - cdata
 - cfunction
 - userdata
 - threads (aka coroutines)
 - tables
*/

template<typename T> concept stringlike = requires (T t) {
    { t.size() } -> std::integral;
    { t.data() } -> std::convertible_to<const char*>;
};

template<typename T> concept pointer_notstr = std::is_pointer_v<T> && !std::same_as<const char*, T>;

template<typename T> concept integral_notbool = std::integral<T> && !std::same_as<bool, T>;

// pushing funcs
int lpush(lua_State* lua, const std::same_as<bool> auto& value) {
    lua_pushboolean(lua, value);
    return 1;
}
int lpush(lua_State* lua, const integral_notbool auto& value) {
    lua_pushinteger(lua, (lua_Integer)value);
    return 1;
}
int lpush(lua_State* lua, const std::floating_point auto& value) {
    lua_pushnumber(lua, (lua_Number)value);
    return 1;
}
int lpush(lua_State* lua, const stringlike auto& value) {
    lua_pushlstring(lua, value.data(), value.size());
    return 1;
}
int lpush(lua_State* lua, const std::convertible_to<const char*> auto& value) {
    lua_pushstring(lua, value);
    return 1;
}
int lpush(lua_State* lua, const pointer_notstr auto& value) {
    lua_pushlightuserdata(lua, (void*)value);
    return 1;
}


template<typename T> int lpush(lua_State* lua, const safe_vector<T>& vec) {
    lua_newtable(lua);
    for (auto i = 0; i < vec.size(); i++) {
        lua_pushinteger(lua, i+1);
        lpush(lua, vec.at(i));
        lua_settable(lua, -3);
    }
    return 1;
}
template<int M, typename Tuple> int lpush(lua_State* lua, const Tuple& values) {
    if constexpr (M < std::tuple_size_v<Tuple>) {
        lpush(lua, std::get<M>(values));
        lpush<M+1, Tuple>(lua, values);
    }
    return std::tuple_size_v<Tuple>;
}
template<typename... Ts> int lpush(lua_State* lua, const std::tuple<Ts...>& values) {
    return lpush<0, std::tuple<Ts...>>(lua, values);
}

// popping funcs
// template<std::same_as<bool> T> T lto(lua_State* lua, int n) {
// 	return T { (bool)lua_toboolean(lua, n) };
// }

template<typename T> concept voidptr = std::constructible_from<T, const void*> && !std::same_as<T, bool>;

template<std::same_as<bool> T> T lto(lua_State* lua, int n) {
    return (T)lua_toboolean(lua, n);
}
template<integral_notbool T> T lto(lua_State* lua, int n) {
    return (T)lua_tointeger(lua, n);
}
template<std::floating_point T> T lto(lua_State* lua, int n) {
    return (T)lua_tonumber(lua, n);
}
template<std::same_as<const char*> T> T lto(lua_State* lua, int n) {
    return T { lua_tostring(lua, n) };
}
template<std::constructible_from<const char*, size_t> T> T lto(lua_State* lua, int n) {
    auto size = size_t{0};
    const auto* str = lua_tolstring(lua, n, &size);
    return T { str, size };
}
template<pointer_notstr T> T lto(lua_State* lua, int n) {
    return (T)lua_topointer(lua, n);
}

template<int N, typename Tuple> void lto(lua_State* lua, Tuple& values) {
    if constexpr(N < std::tuple_size_v<Tuple>) {
        constexpr auto M = std::tuple_size_v<Tuple> - (N+1);
        std::get<M>(values) = lto<std::tuple_element_t<M, Tuple>>(lua, -(N+1));
        lto<N+1, Tuple>(lua, values);
    }
}
template<typename Tuple> Tuple lb_args(lua_State* lua) {
    auto retval = Tuple{};
    lto<0, Tuple>(lua, retval);
    return retval;
}
template<typename... Ts> std::tuple<Ts...> lb_rets(lua_State* lua) {
    auto retval = std::tuple<Ts...>{};
    lto<0, decltype(retval)>(lua, retval);
    lua_pop(lua, (int)sizeof...(Ts));
    return retval;
}


template<typename T> concept is_functor = !(
    std::is_function_v<T> ||
    std::is_pointer_v<T> ||
    std::is_member_function_pointer_v<T>
);

template<typename F, typename... As> struct func_info {};

// function pointer
template<typename R, typename... A> struct func_info<R(*)(A...)> {
    using return_type = R;
    using args_tuple = std::tuple<A...>;
};

// noexcept function pointer
template<typename R, typename... A> struct func_info<R(*)(A...) noexcept> : func_info<R(*)(A...)> {};
template<typename T, typename R, typename... A> struct func_info<R(T::*)(A...)> : func_info<R(*)(A...)> {};
template<typename T, typename R, typename... A> struct func_info<R(T::*)(A...) const> : func_info<R(*)(A...)> {};

// callables
template<is_functor C>
struct func_info<C> {
    using return_type = typename func_info<decltype(&C::operator())>::return_type;
    using args_tuple = typename func_info<decltype(&C::operator())>::args_tuple;
};


template<typename Func> int lcall(lua_State* lua, Func func) {
    using info = func_info<Func>;
    if constexpr(std::is_void_v<typename info::return_type>) {
        std::apply(func, lb_args<typename info::args_tuple>(lua)); return 0;
    } else {
        auto retval = std::apply(func, lb_args<typename info::args_tuple>(lua));
        return lpush(lua, retval);
    }
}

std::string convert_chars(const char* c);
std::string convert_chars(const wchar_t* c);
void make_namespace(lua_State* lua, std::string_view name);
const char* error_str(int res);

class ScriptEngine {
public:
    ScriptEngine(int (*errhandler)(lua_State*));
    ~ScriptEngine();

    void set_path(const std::filesystem::path& path);
    void bind_func(std::string_view name, lua_CFunction func);
    
    bool get_error_state();
    void set_error_state(bool error);
    
    void dofile(const std::filesystem::path& fname, std::string_view namespc=""sv, const auto&... args) {
        if (get_error_state()) {
            return;
        }

        const auto resolved = fname.is_absolute() ? fname : (script_path / fname);
        constexpr auto nargs = (int)sizeof...(args);
        constexpr auto nret = 0;
        log("script dofile:", resolved);

        // load file
        if (const auto res = luaL_loadfilex(lua, (resolved.string() + ".luac").data(), nullptr); res != LUA_OK) {
            // loadfile error
            log("execute loadfile error: ", error_str(res), lua_tostring(lua, -1));
            lua_pop(lua, 1);

            if (res == LUA_ERRFILE) {
                // try and load string version
                if (const auto res = luaL_loadfilex(lua, (resolved.string() + ".lua").data(), nullptr); res != LUA_OK) {
                    log("execute loadfile fallback error: ", error_str(res), lua_tostring(lua, -1));
                    lua_pop(lua, 1);
                    return;
                }
            } else {
                return;
            }
        }

        // make namespace
        if (namespc.size()) {
            lua_getglobal(lua, namespc.data());
            if (lua_type(lua, -1) != LUA_TTABLE) {
                lua_pop(lua, 1);
                make_namespace(lua, namespc.data());
                lua_getglobal(lua, namespc.data());
            }
        } else {
            const auto ns = convert_chars(fname.stem().c_str());
            make_namespace(lua, ns);
            lua_getglobal(lua, ns.c_str());
        }

        #ifdef LUABIND_USE_LUAJIT
        // use setfenv
        if (!lua_setfenv(lua, -2)) {
            log("warning: environment namespace failed (setfenv)");
        }
        #else
        // use _ENV upvalue
        if (!lua_setupvalue(lua, -2, 1)) {
            log("warning: environment namespace failed (_ENV upvalue)");
            lua_pop(lua, 1);
        }
        #endif

        // push arguments ...
        (lpush(lua, args), ...);

        if (const auto res = lua_pcall(lua, nargs, nret, errpos); res != LUA_OK) {
            // pcall error
            log<true, false>(Colors::FG_RED, error_str(res), Colors::FG_RED2, lua_tostring(lua, -1), Colors::FG_DEFAULT);
            lua_pop(lua, 1);
        }
    }

    template<typename... return_types>
    std::tuple<return_types...> call(const char* ns, const char* name, const auto&... args) {
        if (get_error_state()) {
            return {};
        }

        using return_type = std::tuple<return_types...>;
        constexpr auto nargs = (int)sizeof...(args);
        constexpr auto nret = (int)sizeof...(return_types);
        
        // push namespace
        lua_getglobal(lua, ns);
        if (!lua_istable(lua, -1)) {
            log("not a namespace:", ns);
            lua_pop(lua, 1);
            return {};
        }

        // push func
        lua_getfield(lua, -1, name);
        if (!lua_isfunction(lua, -1)) {
            log("not a function", ns, name);
            lua_pop(lua, 2); // func and namespace
            return {};
        }
        
        // push arguments ...
        (lpush(lua, args), ...);

        // call func
        if (const auto res = lua_pcall(lua, nargs, nret, errpos); res != LUA_OK) {
            // an error occurred; log it and pop it; error handler already added traceback
            log<true, false>(Colors::FG_RED, error_str(res), Colors::FG_RED2, lua_tostring(lua, -1), Colors::FG_DEFAULT);
            lua_pop(lua, 1); // pop namespace
            return {};
        }
        
        // pop returns ...
        auto retval = lb_rets<return_types...>(lua);

        lua_pop(lua, 1); // pop namespace

        return retval;
    }

    template<typename T> T get(const char* ns, const char* name) {
        if (get_error_state()) {
            return T{};
        }

        lua_getglobal(lua, ns);
        if (!lua_istable(lua, -1)) {
            log("not a namespace:", ns);
            lua_pop(lua, 1);
            return T{};
        }

        lua_getfield(lua, -1, name);
        const auto retval = lto<T>(lua, -1);
        lua_pop(lua, 2); // field, namespace

        return retval;
    }

    template<typename T> void set(const char* ns, const char* name, const T& value) {
        if (get_error_state()) {
            return;
        }

        lua_getglobal(lua, ns);
        if (!lua_istable(lua, -1)) {
            log("not a namespace: ", ns);
            lua_pop(lua, 1);
            return;
        }

        lpush(lua, value);
        lua_setfield(lua, -2, name);
        lua_pop(lua, 1); // namespace
    }

    inline lua_State* get_lua() { return lua; }

private:
    lua_State* lua;
    std::filesystem::path script_path;
    int errpos; // error handler here always
    bool error_state;
};


#ifdef LUABIND_IMPLEMENTATION
// #if 1

#include <filesystem>

void ScriptEngine::set_error_state(bool error) {
    error_state = error;
}
bool ScriptEngine::get_error_state() {
    return error_state;
}

static int panic_func(lua_State* lua) {
    log<true, false>(Colors::FG_RED, "LUA PANIC!", Colors::FG_DEFAULT);
    return 0;
}
static int default_error_handler(lua_State* lua) {
    luaL_traceback(lua, lua, (std::string(lua_tostring(lua, -1)) + Colors::FG_YELLOW2).c_str(), 0);
    return 1;
}
static int lua_exception_wrapper(lua_State* lua, lua_CFunction f) {
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
std::string convert_chars(const char* c) { return std::string { c }; }
std::string convert_chars(const wchar_t* c) { const auto w = std::wstring { c }; return std::string(w.begin(), w.end()); }

// make new namespace object and leave it on top of the stack
void make_namespace(lua_State* lua, std::string_view name) {
    // _G[name] = setmetatable({}, { __index = _G })

    lua_newtable(lua);
    lua_newtable(lua);
    // stack:
    // {}
    // {}
    #ifdef LUABIND_USE_LUAJIT
    lua_pushvalue(lua, LUA_GLOBALSINDEX);
    #else
    lua_pushglobaltable(lua);
    #endif
    // stack:
    // _G
    // {}
    // {}

    lua_setfield(lua, -2, "__index");
    // stack:
    // { __index = _G }
    // {}
    lua_setmetatable(lua, -2);
    // stack:
    // {} // metatable { __index = _G }
    lua_setglobal(lua, name.data());
    // stack:
    // empty; _G[name] = {} // metatable = { __index = _G }
}

ScriptEngine::ScriptEngine(int (*errhandler)(lua_State*)) {
    error_state = false;
    lua = luaL_newstate();
    lua_atpanic(lua, panic_func);
    lua_pushcfunction(lua, errhandler ? errhandler : default_error_handler);
    errpos = lua_gettop(lua);
    luaL_openlibs(lua);

    #ifdef LUABIND_USE_LUAJIT
        luaJIT_setmode(lua, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);
    //     lua_pushlightuserdata(lua, (void*)lua_exception_wrapper);
    //     luaJIT_setmode(lua, -1, LUAJIT_MODE_WRAPCFUNC|LUAJIT_MODE_ON);
    //     lua_pop(lua, 1);
    #endif
}

ScriptEngine::~ScriptEngine() {
    // pop error handler
    lua_pop(lua, 1);
    lua_close(lua);
}

void ScriptEngine::set_path(const std::filesystem::path& path) {
    script_path = path;
    lua_getglobal(lua, "package");
        lua_getfield(lua, -1, "path");
            auto current_path = std::string { lua_tostring(lua, -1) };
            lua_pop(lua, 1);
        current_path = current_path + ";" + path.string() + "/?.lua"
                                    + ";" + path.string() + "/?.luac";
        lua_pushstring(lua, current_path.data());
            lua_setfield(lua, -2, "path");
    lua_pop(lua, 1); // pop package
}

void ScriptEngine::bind_func(std::string_view name, lua_CFunction func) {
    lua_register(lua, name.data(), func);
}

#endif

