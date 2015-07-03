local args = { ... }
local inputF = io.open(args[1])
local outputF = io.open(args[2], "w")
local input = inputF:read("*a")

outputF:write("local _M = {}\n\n")
for id, value in input:gmatch("#define GL_([A-Z0-9_]+) ([x0-9A-Fa-f]+)") do
	outputF:write(string.format("_M.%s = %s\n",id, value))
end
outputF:write([[

setmetatable(_M, {
	__index = function(_, n)
		error("Attempted to read nil GL constant "..n, 2)
	end
})

return _M
]])

inputF:close()
outputF:close()
