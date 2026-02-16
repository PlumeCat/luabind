-- main.lua

-- dofile "../../scripts/foo.lua"
local ffi = require "ffi"

function import(def)
	local name = def[1]
	ffi.cdef(io.open("../../src/"..name..".h"):read('a'))
	if #def > 1 then
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
end

-- import { "circle", "area" }

-- import { "../../glfw/include/GLFW/glfw3" }
-- import { "glfw3" }
-- local glfw = ffi.load("../../../glfw/glfw-3.4.bin.MACOS/lib-arm64/libglfw.3.dylib")

function main(...)
	
end
