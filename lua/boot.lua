package.path = package.path..";../lua/?.lua;../lua/lib/?.lua"

local ffi = require"ffi"
ffi.cdef(io.open("ffi/ffi_defs.h","r"):read("*a"))
DEBUG = false
--debug = debug and ffi.os == 'Linux'

if DEBUG then
else
	setmetatable(_G, {
		__newindex = function (_, n)
			error("Attempt to write to global variable (use rawset) '"..n.."'", 2)
		end,
		__index = function (_, n)
			error("Attempt to read global variable (use rawget) '"..n.."'", 2)
		end,
	})
end
require("init")