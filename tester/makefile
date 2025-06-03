SERVICE = main.cpp

TARGET = ../../licensecc/install/lib/licensecc/DEFAULT/liblicensecc_static.a

SERVICE_BIN = tester.exe
INCLUDES = -I$(CURDIR)
INCLUDES += -I$(CURDIR)/..
INCLUDES += -I$(CURDIR)/../../cpp-httplib
INCLUDES += -I$(CURDIR)/../../picojson
INCLUDES += -I$(CURDIR)/../../json/single_include
INCLUDES += -I$(CURDIR)/../../inja/single_include
INCLUDES += -I$(CURDIR)/../../licensecc/install/include
INCLUDES += -I$(CURDIR)/../../licensecc/install/include/licensecc/DEFAULT

CC = g++
#debug -g添加调试信息
#release -O2编译优化, 并移除-g
#CCFLAGS = -Wall -lpthread -fPIC -m64 -g -std=c++20 -lstdc++ -pipe
#-Wsign-compare -Wfloat-equal -Wpointer-arith -Wcast-align
DYNAMIC_LIBS = -lpthread -lstdc++ -pipe -lssl -lcrypto -ldl -lz

MACROS = -DPICOJSON_USE_INT64 -DENABLE_EASYCPP_LOG
CHECKS = -Wpointer-arith -Wcast-align
CCFLAGS = -g -O2 $(CHECKS) $(MACROS) -fPIC -m64 -std=c++17

all:$(SERVICE_BIN)

$(SERVICE_BIN):$(SERVICE) $(TARGET)
	$(CC) $(SERVICE) -o $@  $(CCFLAGS) $(INCLUDES) -Wl,--start-group $(TARGET) -Wl,--end-group $(DYNAMIC_LIBS)

clean:
	rm -rf $(SERVICE_BIN)
