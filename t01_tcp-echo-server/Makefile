
CXX = g++
CXXFLAGS = -std=gnu++11 -Wall -Wextra \
    -DCONFIG_PROC_HAVE_LOG=1 \
    -DCONFIG_PROC_HAVE_CORE_LOG=1 \
    -I../../deps/SystemCore \
    -I../../deps/LibNaegCommon \

LDFLAGS = -pthread

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
    ../UserInteracting.cpp

OBJ = $(notdir $(SRC:.cpp=.o))

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../../deps/SystemCore/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../../deps/LibNaegCommon/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf deps *.o app

