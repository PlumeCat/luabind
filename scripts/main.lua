-- main.lua

-- dofile "../../scripts/foo.lua"

function import(def)
	local ffi = require("ffi")
	local name = def[1]
	ffi.cdef(io.open("../../src/"..name..".h"):read('a'))
	local methods = {}
	for i, v in ipairs(def) do
		if i > 1 then
			methods[v] = ffi.C[name .. "_" .. v]
		end
	end
	_G[name:sub(1,1):upper() .. name:sub(2)] = ffi.metatype(name, {
		__index = methods
	})
end

import { "circle", "area" }

function bar(...)
	print("main foo2")
	foo2()
	-- local circle = Circle(10, 20, 3)
	-- local area = circle:area()
	-- print("circle: ", circle.x, circle.y, circle.radius)
	-- print("area = ", area)
	Circle(0, 2, 3):area()
	-- return 1, 2, "hello"
end
