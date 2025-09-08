SERVICE = main.cpp ../../pugixml/src/pugixml.cpp


SERVICE_BIN = tester.exe
INCLUDES = -I$(CURDIR)
INCLUDES += -I$(CURDIR)/..
INCLUDES += -I$(CURDIR)/../../cpp-httplib
INCLUDES += -I$(CURDIR)/../../picojson
INCLUDES += -I$(CURDIR)/../../json/single_include
INCLUDES += -I$(CURDIR)/../../inja/single_include
INCLUDES += -I$(CURDIR)/../../aho_corasick/src
INCLUDES += -I$(CURDIR)/../../pugixml/src
INCLUDES += -I$(CURDIR)/../../cppjieba/include
INCLUDES += -I$(CURDIR)/../../cppjieba/deps/limonp/include

CC = g++
#debug -g添加调试信息
#release -O2编译优化, 并移除-g
#CCFLAGS = -Wall -lpthread -fPIC -m64 -g -std=c++20 -lstdc++ -pipe
#-Wsign-compare -Wfloat-equal -Wpointer-arith -Wcast-align
DYNAMIC_LIBS = -lpthread -lstdc++ -pipe -lssl -lcrypto -ldl -lz -lev -llzma
DYNAMIC_LIBS += -L/usr/local/lib64 -lamqpcpp -Wl,-rpath=/usr/local/lib64 
DYNAMIC_LIBS += -L/usr/local/lib -lfasttext -Wl,-rpath=/usr/local/lib 


MACROS = -DEASYCPP_LOGGING #开启调试日志
MACROS += -DPHONEDATA_GZIP #PhoneData使用gzip库解压缩
CHECKS = -Wpointer-arith -Wcast-align
CCFLAGS = -O2 $(CHECKS) $(MACROS) -fPIC -m64 -std=c++17

all:$(SERVICE_BIN)

$(SERVICE_BIN):$(SERVICE) $(TARGET)
	$(CC) $(SERVICE) -o $@  $(CCFLAGS) $(INCLUDES) -Wl,--start-group $(TARGET) -Wl,--end-group $(DYNAMIC_LIBS)

clean:
	rm -rf $(SERVICE_BIN)
