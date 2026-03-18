
CXX = g++
CXXFLAGS = -std=gnu++11 -Wall -Wextra \
    -DCONFIG_PROC_HAVE_LOG=1 \
    -DCONFIG_PROC_HAVE_CORE_LOG=1 \
    -I../../deps/SystemCore

LDFLAGS = -pthread

SRC = \
    ../../deps/SystemCore/Processing.cpp \
    ../../deps/SystemCore/Log.cpp \
    ../../deps/SystemCore/TcpListening.cpp \
    ../../deps/SystemCore/TcpTransfering.cpp \
    ../../deps/SystemCore/SystemCommanding.cpp \
    ../../deps/SystemCore/SystemDebugging.cpp \
    ../main.cpp \
    ../Supervising.cpp \
    ../UserInteracting.cpp

OBJ = $(notdir $(SRC:.cpp=.o))

TARGET = app

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

%.o: ../../deps/SystemCore/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: ../%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf deps *.o app

