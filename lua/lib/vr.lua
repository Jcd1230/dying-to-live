local ffi = require("ffi")
local openvr
local _M = {}

if ffi.os == "Windows" then
  openvr = ffi.load("./openvr_api.dll")
else
  openvr = ffi.load("./libopenvr_api.so")
end

_M.isHmdPresent = function()
  return openvr.VR_IsHmdPresent()
end

return _M
