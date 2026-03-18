
project(
	'TCP Echo Server',
	'c',
	'cpp',
	default_options : ['cpp_std=c++11'],
)

# Dependencies

deps = []
deps += dependency('threads')

if host_machine.system() == 'windows'
	deps += cxx.find_library('ws2_32')
endif

# Application

myApp = executable(
	'app',
	[
		'../deps/SystemCore/Processing.cpp',
		'../deps/SystemCore/Log.cpp',
		'../deps/SystemCore/TcpListening.cpp',
		'../deps/SystemCore/TcpTransfering.cpp',
		'../deps/SystemCore/SystemCommanding.cpp',
		'../deps/SystemCore/SystemDebugging.cpp',
		'main.cpp',
		'Supervising.cpp',
		'UserInteracting.cpp',
	],
	include_directories : include_directories([
		'../deps/SystemCore',
	]),
	dependencies : [
		deps,
	],
	cpp_args : [
		'-std=gnu++11',
		'-DCONFIG_PROC_HAVE_LOG=1',
		'-DCONFIG_PROC_HAVE_CORE_LOG=1',
	],
)

