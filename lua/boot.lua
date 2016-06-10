package.path = package.path..";../lua/?.lua;../lua/lib/?.lua;../lua/lib/?/init.lua"

local ffi = require"ffi"

ffi.cdef(io.open("ffi/ffi_defs.h","r"):read("*a"))

ffi.cdef[[
int main(int argc, char *argv[]);
void initOGL();
]]
ffi.cdef[[
void Sleep(int ms);
int poll(struct pollfd *fds, unsigned long nfds, int timeout);
]]


DEBUG = true and ffi.os == 'Linux'

local md
if ffi.os == 'Linux' then
	md = require("mobdebug")
end

setmetatable(_G, {
	__newindex = function (_, n)
		error("Attempt to write global variable '"..n.."' (use rawset)", 2)
	end,
	__index = function (_, n)
		error("Attempt to read global variable '"..n.."' (use rawget)", 2)
	end
})

require("init")
