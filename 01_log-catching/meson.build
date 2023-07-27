
project(
	'Log-Catching',
	'c',
	'cpp',
	default_options : ['cpp_std=c++11'],
)

fs = import('fs')

srcsCore = [
	'Processing.cpp',
	'Log.cpp',
	'TcpListening.cpp',
	'TcpTransfering.cpp',
	'SystemDebugging.cpp',
	'SystemCommanding.cpp',
]

srcsCommon = [
	'LibFilesys.cpp',
]

srcsSpecific = [
	'main.cpp',
	'LcSupervising.cpp',
	'LogCatching.cpp',
]

srcs = []
srcs += srcsSpecific

foreach f : srcsCore
	srcs += join_paths('../ProcessingCore', f)
endforeach

foreach f : srcsCommon
	srcs += join_paths('../ProcessingCommon', f)
endforeach

incs = [
	'../ProcessingCore',
	'../ProcessingCommon',
]

incsAbs = []
foreach p : incs
	incsAbs += include_directories(p)
endforeach

deps = []
deps += dependency('tclap')

cppArgs = [
	'-DdPackageName="' + meson.project_name() + '"',
	'-DdAppName="log-catching"',
	'-DHAVE_SSTREAM',
	'-DCONFIG_PROC_HAVE_LOG=1',
	'-DCONFIG_PROC_SHOW_ADDRESS_IN_ID=0',
	'-DCONFIG_PROC_INFO_BUFFER_SIZE=1024',
	'-DCONFIG_PROC_DISABLE_TREE_DEFAULT=0',
	'-std=gnu++11',
]

cppArgs += '-g'

# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
warnings = [
	'-Wall',
	'-Wextra',
	'-Wpedantic',
	'-Werror',
	'-Wfatal-errors',
	'-Wreorder',
	'-Wswitch-enum',
	'-Wuseless-cast',
	'-Wparentheses',
	'-Wshift-overflow',
	'-Wsign-compare',
	'-Wzero-as-null-pointer-constant',
	'-Wcast-align',
	'-Wcast-qual',
	'-Wcatch-value',
	'-Wchar-subscripts',
	'-Wswitch-default',
	'-Wctor-dtor-privacy',
	'-Wduplicated-branches',
	'-Wduplicated-cond',
	'-Wempty-body',
	'-Wextra-semi',
	'-Wfloat-equal',
	'-Wformat',
	'-Wformat-extra-args',
	'-Wimplicit-fallthrough',
	'-Wmissing-field-initializers',
	'-Wnull-dereference',
	'-Wshadow',
]

cppArgs += warnings

myApp = executable(
	'log-catching',
	srcs,
	include_directories : [
		incsAbs,
	],
	dependencies : [
		deps,
	],
	cpp_args : [
		cppArgs,
	],
)

