SERVICE = main.cpp

TARGET = 

SERVICE_BIN = tester.exe
INCLUDES = -I$(CURDIR)/..
INCLUDES += -I$(CURDIR)/../../cpp-httplib
INCLUDES += -I$(CURDIR)/../../picojson
INCLUDES += -I$(CURDIR)/../../ctemplate/src

CC = g++
#debug -g添加调试信息
#release -O2编译优化, 并移除-g
#CCFLAGS = -Wall -lpthread -fPIC -m64 -g -std=c++20 -lstdc++ -pipe
#-Wsign-compare -Wfloat-equal -Wpointer-arith -Wcast-align
DYNAMIC_LIBS = -lpthread -lstdc++ -pipe -lssl -lcrypto -ldl -lz -lctemplate
MACROS = -DPICOJSON_USE_INT64
CHECKS = -Wpointer-arith -Wcast-align
CCFLAGS = -g -O2 $(CHECKS) $(MACROS) -fPIC -m64 -std=c++11 -L/usr/local/lib

all:$(SERVICE_BIN)

$(SERVICE_BIN):$(SERVICE) $(TARGET)
	$(CC) $(SERVICE) -o $@  -Wl,--start-group $(TARGET) -Wl,--end-group $(CCFLAGS) $(DYNAMIC_LIBS) $(INCLUDES)

clean:
	rm -rf $(SERVICE_BIN)
