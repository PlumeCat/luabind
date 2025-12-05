cmake_minimum_required(VERSION 3.23 FATAL_ERROR)
project(luabind)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(USE_LUAJIT ON)

file(GLOB_RECURSE source_files CONFIGURE_DEPENDS src/*.cpp)
add_executable(luabind_test ${source_files})

target_include_directories(luabind_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/include/")
target_include_directories(luabind_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../jlib")

if (USE_LUAJIT)
	target_compile_definitions(luabind_test PRIVATE LUABIND_USE_LUAJIT)
	target_include_directories(luabind_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../luajit/src/")
	target_link_libraries(luabind_test "${CMAKE_CURRENT_LIST_DIR}/../luajit/src/libluajit.a")
else()
	target_include_directories(luabind_test PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../lua-5.4.0/src/")
	target_link_libraries(luabind_test "${CMAKE_CURRENT_LIST_DIR}/../lua-5.4.0/src/liblua.a")
endif()