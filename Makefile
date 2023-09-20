BUILD_DIR:=build
CXXFLAGS:= -O0 -Wall -Wextra -g -lpthread -std=c++11

HCNSMEMORY_BUILD_DIR := $(BUILD_DIR)/server/include/ 
HCNSMEMORY_BUILD_TARGET := $(HCNSMEMORY_BUILD_DIR)/HCNSMemory.lib
HCNSMEMORY_CXX_FILES := $(wildcard HCNSMemory/src/*.cpp)
HCNSMEMORY_OBJ_FILES := $(HCNSMEMORY_CXX_FILES:HCNSMemory/src/%.cpp=$(HCNSMEMORY_BUILD_DIR)/%hcnsmem_cpp.o)

SERVER_BUILD_DIR := $(BUILD_DIR)/Server/
SERVER_BUILD_TARGET := $(SERVER_BUILD_DIR)/Server
SERVER_CXX_FILES := $(wildcard server/src/*.cpp)
SERVER_OBJ_FILES := $(SERVER_CXX_FILES:server/src/%.cpp=$(SERVER_BUILD_DIR)/%server_cpp.o)

CLIENT_BUILD_DIR := $(BUILD_DIR)/Client/
CLIENT_BUILD_TARGET := $(CLIENT_BUILD_DIR)/Client
CLIENT_CXX_FILES := $(wildcard client/src/*.cpp)
CLIENT_OBJ_FILES := $(CLIENT_CXX_FILES:client/src/%.cpp=$(CLIENT_BUILD_DIR)/%client_cpp.o)

.PHONY: clean all build
all: build

#generate a static library file
$(HCNSMEMORY_BUILD_DIR)/%memory_cpp.o : HCNSMemory/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -IHCNSMemory/include -shared -fPIC -c $< -o $@

#compile cpp files and generate obj files
$(CLIENT_BUILD_DIR)/%client_cpp.o: client/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iclient/include -c $< -o $@

$(SERVER_BUILD_DIR)/%server_cpp.o: server/src/%.cpp
	mkdir =p $(@D)
	$(CXX) $(CXXFLAGS) -Iserver/include -c $< -o $@

#link obj files to excutable file
server/include: $(HCNSMEMORY_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o HCNSMemory.lib

build/Server: $(SERVER_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $(SERVER_BUILD_TARGET)

build/Client: $(CLIENT_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $(CLIENT_BUILD_TARGET)

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
