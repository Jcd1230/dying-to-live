package.path = package.path..";../lua/?.lua;../lua/lib/?.lua"

local ffi = require"ffi"
ffi.cdef(io.open("ffi/ffi_defs.h","r"):read("*a"))
DEBUG = true and ffi.os == 'Linux'

--if DEBUG then else
	setmetatable(_G, {
		__newindex = function (_, n)
			error("Attempt to write global variable '"..n.."' (use rawset)", 2)
		end,
		__index = function (_, n)
			error("Attempt to read global variable '"..n.."' (use rawget)", 2)
		end
	})
-- end
require("init")
