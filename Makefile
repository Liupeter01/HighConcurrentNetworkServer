BUILD_DIR := build
LIB_DIR := lib
CXX :=clang++
CXXFLAGS := -O0 -Wall -Wextra -g -lpthread -std=c++11
LDFLAGS := -shared
ARFLAGS := rcs

HCNSMEMORY_BUILD_DIR := $(LIB_DIR)/HCNSMemory
#HCNSMEMORY_BUILD_TARGET := $(HCNSMEMORY_BUILD_DIR)/libhcnsmemory.a
HCNSMEMORY_BUILD_TARGET := $(HCNSMEMORY_BUILD_DIR)/libhcnsmemory.so
HCNSMEMORY_CXX_FILES := $(wildcard HCNSMemory/src/*.cpp)
HCNSMEMORY_OBJ_FILES := $(HCNSMEMORY_CXX_FILES:HCNSMemory/src/%.cpp=$(HCNSMEMORY_BUILD_DIR)/%hcnsmem_cpp.o)
HCNSMEMORY_LIBS := -L$(HCNSMEMORY_BUILD_DIR) -lhcnsmemory

SERVER_BUILD_DIR := $(BUILD_DIR)/Server
SERVER_BUILD_TARGET := $(SERVER_BUILD_DIR)/Server
SERVER_CXX_FILES := $(wildcard server/src/*.cpp)
SERVER_OBJ_FILES := $(SERVER_CXX_FILES:server/src/%.cpp=$(SERVER_BUILD_DIR)/%server_cpp.o)

CLIENT_BUILD_DIR := $(BUILD_DIR)/Client
CLIENT_BUILD_TARGET := $(CLIENT_BUILD_DIR)/Client
CLIENT_CXX_FILES := $(wildcard client/src/*.cpp)
CLIENT_OBJ_FILES := $(CLIENT_CXX_FILES:client/src/%.cpp=$(CLIENT_BUILD_DIR)/%client_cpp.o)

.PHONY: clean all build
all: $(SERVER_BUILD_TARGET) $(CLIENT_BUILD_TARGET)

#generate a Library .so file
$(HCNSMEMORY_BUILD_DIR)/%hcnsmem_cpp.o: HCNSMemory/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -IHCNSMemory/include -c $< -o $@

$(HCNSMEMORY_BUILD_TARGET):$(HCNSMEMORY_OBJ_FILES)
	mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -fPIC $^ -o $@
#	$(AR) $(ARFLAGS) $^ -o $@
 
#compile cpp files and generate obj files
$(CLIENT_BUILD_DIR)/%client_cpp.o: client/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iclient/include -c $< -o $@

$(SERVER_BUILD_DIR)/%server_cpp.o: server/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iserver/include -c $< -o $@

#link obj files to excutable file
$(SERVER_BUILD_TARGET): $(SERVER_OBJ_FILES) $(HCNSMEMORY_BUILD_TARGET)
	$(CXX) $(CXXFLAGS) $(HCNSMEMORY_LIBS) $^ -o $@

$(CLIENT_BUILD_TARGET): $(CLIENT_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)
