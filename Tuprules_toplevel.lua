C = tup.getconfig
LIB_DIR = tup.nodevariable("./lib")

function lib_path(libs)
	for k,v in pairs(libs) do
		libs[k] = LIB_DIR .. "/lib" .. v .. ".a"
	end
	return libs
end

function lib_group(libs)
	for k,v in pairs(libs) do
		libs[k] = LIB_DIR .. "/<lib_" .. v .. ">"
	end
	return libs
end

function LIB(name)
	return LIB_DIR .. "/lib" .. name .. ".a"
end

function LIB_GROUP()
	return LIB_DIR .. "/<lib>"
end

function compile_clib(name)
	local objs = compile_c()
	local libname = name or tup.getdirectory()
	link_lib(objs, libname, LIB(libname))
end

function compile_c()
	return tup.foreach_rule(
		"*.c", 
		"@(CC) %f -c -o %o", 
		{"%B.o"}
	)
end

function link_lib(objs, name, file)
	tup.rule(
		objs,
		"@(AR) rcs %o %f",
		{
			file,
			LIB_GROUP()
		}
	)
end

function link_bin(objs, output)
	tup.rule(
		objs,
		"@(CC) %f -o %o",
		output
	)
end
