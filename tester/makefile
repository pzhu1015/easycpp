SERVICE = main.cpp


SERVICE_BIN = tester.exe
INCLUDES = -I$(CURDIR)
INCLUDES += -I$(CURDIR)/..
INCLUDES += -I$(CURDIR)/../../cpp-httplib
INCLUDES += -I$(CURDIR)/../../picojson
INCLUDES += -I$(CURDIR)/../../json/single_include
INCLUDES += -I$(CURDIR)/../../inja/single_include

CC = g++
#debug -g添加调试信息
#release -O2编译优化, 并移除-g
#CCFLAGS = -Wall -lpthread -fPIC -m64 -g -std=c++20 -lstdc++ -pipe
#-Wsign-compare -Wfloat-equal -Wpointer-arith -Wcast-align
DYNAMIC_LIBS = -L/usr/local/lib64 -lamqpcpp -Wl,-rpath=/usr/local/lib64 -lpthread -lstdc++ -pipe -lssl -lcrypto -ldl -lz -lev

MACROS = -DPICOJSON_USE_INT64 -DEASYCPP_LOGGING
CHECKS = -Wpointer-arith -Wcast-align
CCFLAGS = -g $(CHECKS) $(MACROS) -fPIC -m64 -std=c++17

all:$(SERVICE_BIN)

$(SERVICE_BIN):$(SERVICE) $(TARGET)
	$(CC) $(SERVICE) -o $@  $(CCFLAGS) $(INCLUDES) -Wl,--start-group $(TARGET) -Wl,--end-group $(DYNAMIC_LIBS)

clean:
	rm -rf $(SERVICE_BIN)
