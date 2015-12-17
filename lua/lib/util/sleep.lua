local ffi = require("ffi")

local sleep

if ffi.os == "Windows" then
  sleep = function (s)
    ffi.C.Sleep(s*1000)
  end
else
  sleep = function (s)
    ffi.C.poll(nil, 0, s*1000)
  end
end

return sleep