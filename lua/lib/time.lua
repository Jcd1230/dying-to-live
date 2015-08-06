local class = require("30log")
local ffi = require("ffi")

local _M = {}

if ffi.os == "Windows" then
	local _pfreq = ffi.new("uint64_t[1]")
	ffi.cdef[[
	int QueryPerformanceCounter(uint64_t *lpPerformanceCount);
	int QueryPerformanceFrequency(uint64_t *lpPerformanceCount);
	]]
	ffi.C.QueryPerformanceFrequency(_pfreq)
	local _freq = _pfreq[0]
	_M.now = function()
		local now = ffi.new("uint64_t[1]")
		ffi.C.QueryPerformanceCounter(now)
		return now
	end
	
	_M.diff = function(to, from)
		local diff = to[0] - from[0]
		return (tonumber(diff)/tonumber(_freq))*1000
	end
elseif ffi.os == "Linux" then
	_M.now = function()
		local now = ffi.new("struct timespec[1]")
		ffi.C.clock_gettime(1 --[[CLOCK_MONOTONIC]], now)
		return now
	end
	
	_M.diff = function(to, from)
		local diffs = to[0].tv_sec - from[0].tv_sec
		local diffns = to[0].tv_nsec - from[0].tv_nsec
		return tonumber(diffs*1000) + tonumber(diffns)/1000000
	end
else
	error("Time module does not have an implementation for OS = '"..ffi.os.."'")
end

return _M