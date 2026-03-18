
cmake_minimum_required(VERSION 3.10)

project(LogCatching C CXX)

# -------------------------------
# Compiler standard
# -------------------------------
if(MSVC)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------
# Source files
# -------------------------------
set(SRC
    ../deps/SystemCore/Processing.cpp
    ../deps/SystemCore/Log.cpp
    ../deps/SystemCore/SystemDebugging.cpp
    ../deps/SystemCore/SystemCommanding.cpp
    ../deps/SystemCore/TcpListening.cpp
    ../deps/SystemCore/TcpTransfering.cpp
    main.cpp
    LcSupervising.cpp
    LogCatching.cpp
)

# -------------------------------
# Include directories
# -------------------------------
include_directories(
    ../deps/SystemCore
    ../deps/LibNaegCommon
)

# -------------------------------
# Compiler flags
# -------------------------------
if(MSVC)
    add_compile_definitions(APP_HAS_TCLAP=1) # falls tclap genutzt wird
    add_compile_options(/std:c++17 /W4)
else()
    add_compile_options(
        -std=gnu++11
        -DCONFIG_PROC_HAVE_LOG=1
        -DCONFIG_PROC_HAVE_CORE_LOG=0
    )

    # GCC warnings
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wall -Wextra -Wpedantic -Werror -Wfatal-errors
            -Wreorder -Wswitch-enum -Wuseless-cast -Wparentheses
            -Wshift-overflow -Wsign-compare -Wzero-as-null-pointer-constant
            -Wcast-align -Wcast-qual -Wcatch-value -Wchar-subscripts
            -Wswitch-default -Wctor-dtor-privacy -Wduplicated-branches
            -Wduplicated-cond -Wempty-body -Wextra-semi -Wfloat-equal
            -Wformat -Wformat-extra-args -Wimplicit-fallthrough
            -Wmissing-field-initializers -Wnull-dereference -Wshadow
        )
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(
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

    # Platform-specific defines
    if(WIN32)
        add_compile_definitions(_WIN32_WINNT=_WIN32_WINNT_WIN10)
        add_compile_definitions(WINVER=_WIN32_WINNT_WIN10)
    endif()
endif()

# -------------------------------
# Dependencies
# -------------------------------
find_package(Threads REQUIRED)

if(WIN32 AND NOT MSVC)
    # Link ws2_32 on MinGW/Clang on Windows
    set(PLATFORM_LIBS ws2_32)
endif()

# -------------------------------
# Executable
# -------------------------------
set(EXE_NAME app)
if(WIN32)
    set(EXE_NAME App)
endif()

add_executable(${EXE_NAME} ${SRC})

target_link_libraries(${EXE_NAME} PRIVATE Threads::Threads ${PLATFORM_LIBS})

