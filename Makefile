PRO = libalsa_if.so
OBJ = alsa_if.o

MAINPRO = demo
SRVPRO  = server
CLTPRO  = client

LD  = ld
CC  = gcc
CXX = g++

PTHREAD 	= -lpthread
SHARE 		= -fPIC -shared
ALSAIF   	= -lalsa_if
RUNTIME_DIR = -Wl,-rpath=./
LIB_DIR     = -L./
ALSA        = -lasound

CXXFLAGS    = $(SHARE) $(PTHREAD)


VPATH 		= ./


$(PRO) : $(OBJ)
	$(LD) -fPIC -shared -o $@ $^ $(CXXFLAGS) $(ALSA)

%.o : %.cpp
	$(CXX) -fPIC -o $@ -c $< $(CXXFLAGS)

.PHONY:clean
clean:
	rm -rf $(OBJ)

.PHONY:dstclean
dstclean:
	rm -rf $(PRO) $(OBJ) $(MAINPRO) $(SRVPRO) $(CLTPRO) ./record.pcm

.PHONY:main
main:
	$(CXX) -o $(MAINPRO) ./main.cpp $(LIB_DIR) $(RUNTIME_DIR) $(ALSAIF)

.PHONY:server
server:
	$(CXX) -o $(SRVPRO) ./server.cpp $(LIB_DIR) $(RUNTIME_DIR) $(ALSAIF) $(PTHREAD) $(ALSA)

.PHONY:client
client:
	$(CXX) -o $(CLTPRO) ./client.cpp $(LIB_DIR) $(RUNTIME_DIR) $(ALSAIF) $(PTHREAD) $(ALSA)

.PHONY:all
all:
	make
	make main
	make server
	make client
	