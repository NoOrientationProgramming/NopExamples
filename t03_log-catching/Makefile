
CXX = g++
CXXFLAGS = -std=gnu++11 -Wall -Wextra \
    -DdPackageName="\"ApplicationLogCatcher\"" \
    -DdAppName="\"app\"" \
    -DdVersion="\"app-v1.26.3.1\"" \
    -DCONFIG_PROC_HAVE_LOG=1 \
    -DCONFIG_PROC_HAVE_CORE_LOG=1 \
    -I../../deps/SystemCore \
    -I../../deps/LibNaegCommon \

ifeq ($(OS),Windows_NT)
    LDLIBS += -lws2_32
else
    LDLIBS += -pthread
endif

SRC = \
    ../../deps/SystemCore/Processing.cpp \
    ../../deps/SystemCore/Log.cpp \
    ../../deps/SystemCore/SystemDebugging.cpp \
    ../../deps/SystemCore/SystemCommanding.cpp \
    ../../deps/SystemCore/TcpListening.cpp \
    ../../deps/SystemCore/TcpTransfering.cpp \
    ../../deps/LibNaegCommon/LibFilesys.cpp \
    ../../deps/LibNaegCommon/LibDspc.cpp \
    ../../deps/LibNaegCommon/Res.cpp \
    ../main.cpp \
    ../Supervising.cpp \
    ../LogCatching.cpp \

OBJ = $(notdir $(SRC:.cpp=.o))

TCLAP_DIR := ../../deps/tclap_loc/include/tclap

ifneq ("$(wildcard $(TCLAP_DIR)/CmdLine.h)","")
    $(info Submodule tclap found)

    CXXFLAGS += -isystem../../deps/tclap_loc/include
    CXXFLAGS += -DAPP_HAS_TCLAP=1
else
    $(warning Submodule tclap not found. Limited CLI options)
endif

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $@ $(LDLIBS)

%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../../deps/SystemCore/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../../deps/LibNaegCommon/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf deps *.o app

