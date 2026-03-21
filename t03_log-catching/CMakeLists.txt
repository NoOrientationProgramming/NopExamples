
cmake_minimum_required(VERSION 3.10)

project(ApplicationLogCatcher LANGUAGES CXX)

# =========================
# Sources
# =========================
set(SRCS
    ../deps/SystemCore/Processing.cpp
    ../deps/SystemCore/Log.cpp
    ../deps/SystemCore/SystemDebugging.cpp
    ../deps/SystemCore/SystemCommanding.cpp
    ../deps/SystemCore/TcpListening.cpp
    ../deps/SystemCore/TcpTransfering.cpp
    ../deps/LibNaegCommon/LibFilesys.cpp
    ../deps/LibNaegCommon/LibDspc.cpp
    ../deps/LibNaegCommon/Res.cpp
    main.cpp
    Supervising.cpp
    LogCatching.cpp
)

# =========================
# Base compile definitions
# =========================
add_compile_definitions(
    dPackageName="${PROJECT_NAME}"
    dAppName="app"
    dVersion="app-v1.26.3.1"
    CONFIG_PROC_HAVE_LOG=1
    CONFIG_PROC_HAVE_CORE_LOG=0
)

# =========================
# Threads
# =========================
find_package(Threads REQUIRED)

# =========================
# Executable name
# =========================
if (WIN32)
    set(EXE_NAME App)
else()
    set(EXE_NAME app)
endif()

add_executable(${EXE_NAME} ${SRCS})

# =========================
# Include directories
# =========================
target_include_directories(${EXE_NAME} PRIVATE
    ../deps/SystemCore
    ../deps/LibNaegCommon
)

# =========================
# Compiler / platform logic
# =========================
if (WIN32)
    target_compile_definitions(${EXE_NAME} PRIVATE
        _WIN32_WINNT=_WIN32_WINNT_WIN10
        WINVER=_WIN32_WINNT_WIN10
    )

    if (NOT MSVC)
        target_compile_definitions(${EXE_NAME} PRIVATE
            __STDCPP_THREADS__=1
        )
    endif()
else()
    target_compile_options(${EXE_NAME} PRIVATE -std=gnu++11)
endif()

# =========================
# Compiler-specific flags
# =========================
if (MSVC)
    target_compile_options(${EXE_NAME} PRIVATE /std:c++17)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${EXE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Werror -Wfatal-errors
        -Wreorder -Wswitch-enum -Wuseless-cast -Wparentheses
        -Wshift-overflow -Wsign-compare -Wzero-as-null-pointer-constant
        -Wcast-align -Wcast-qual -Wcatch-value -Wchar-subscripts
        -Wswitch-default -Wctor-dtor-privacy -Wduplicated-branches
        -Wduplicated-cond -Wempty-body -Wextra-semi -Wfloat-equal
        -Wformat -Wformat-extra-args -Wimplicit-fallthrough
        -Wmissing-field-initializers -Wnull-dereference -Wshadow
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${EXE_NAME} PRIVATE
        -Wall -Wextra -Wpedantic -Werror
        -Wno-gnu-zero-variadic-macro-arguments
        -Wshadow -Wsign-conversion -Wnull-dereference
        -Wdouble-promotion -Wimplicit-fallthrough
        -Wcast-qual -Wcast-align -Wnon-virtual-dtor
        -Woverloaded-virtual -Wfloat-equal -Wswitch-enum
        -Wmissing-declarations -Wunreachable-code
        -Wdocumentation -Wthread-safety
    )
endif()

# =========================
# Dependencies
# =========================
target_link_libraries(${EXE_NAME} PRIVATE Threads::Threads)

if (WIN32)
    target_link_libraries(${EXE_NAME} PRIVATE ws2_32)
endif()

# =========================
# Optional Tclap submodule
# =========================
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../deps/tclap_loc/include")
    message(STATUS "submodule tclap found")

    target_include_directories(${EXE_NAME} SYSTEM PRIVATE
        ../deps/tclap_loc/include
    )

    target_compile_definitions(${EXE_NAME} PRIVATE APP_HAS_TCLAP=1)
else()
    message(WARNING "submodule tclap not found. Limited CLI options")
endif()

