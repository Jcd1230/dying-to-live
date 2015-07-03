local ffi = require("ffi")
local km = ffi.load("./libkazmath.so")
local _M = {}

local vec3 = {
	add = km.kmVec3Add,
	sub = km.kmVec3Subtract,
	zero = km.kmVec3Zero,
	scale = km.kmVec3Scale,
	quatForwardVec3RH = km.kmQuaternionGetForwardVec3RH,
	quatUpVec3 = km.kmQuaternionGetUpVec3,
	quatRightVec3 = km.kmQuaternionGetRightVec3
}
setmetatable(vec3, {
	__call = function(self, size)
		--return ffi.new("kmVec3[1]")
		return ffi.cast("kmVec3 *",ffi.C.malloc(ffi.sizeof("kmVec3")*(size or 1)))
	end
})
_M.vec3 = vec3
--_M.vec3 = ffi.metatype("kmVec3", vec3)

local mat3 = {	
	mul = km.kmMat3Multiply,
	iden = km.kmMat3Identity,
	inverse = km.kmMat3Inverse,
	transpose = km.kmMat3Transpose,
	assignMat4 = km.kmMat3AssignMat4
}
setmetatable(mat3, {
	__call = function(self, size)
		return ffi.cast("kmMat3 *",ffi.C.malloc(ffi.sizeof("kmMat3")*(size or 1)))
	end
})
_M.mat3 = mat3

local mat4 = {	
	mul = km.kmMat4Multiply,
	translation = km.kmMat4Translation,
	scaling = km.kmMat4Scaling,
	rotationYPR = km.kmMat4RotationYawPitchRoll,
	lookAt = km.kmMat4LookAt,
	perspectiveProjection = km.kmMat4PerspectiveProjection,
	iden = km.kmMat4Identity,
	inverse = km.kmMat4Inverse,
	transpose = km.kmMat4Transpose
}
setmetatable(mat4, {
	__call = function(self, size)
		return ffi.cast("kmMat4 *",ffi.C.malloc(ffi.sizeof("kmMat4")*(size or 1)))
	end
})
_M.mat4 = mat4

local quat = {
	rotationPYR = km.kmQuaternionRotationPitchYawRoll,
}
setmetatable(quat, {
	__call = function(self, size)
		return ffi.cast("kmQuaternion *",ffi.C.malloc(ffi.sizeof("kmQuaternion")*(size or 1)))
	end
})
_M.quat = quat

return _M