BUILD_DIR := build
CXX :=clang++
CXXFLAGS := -O0 -Wall -Wextra -g -lpthread -stdlib=libc++ -std=c++14
LDFLAGS := -shared -fPIC

MEMOBJPOOL_BUILD_DIR := $(BUILD_DIR)/MemoryObjectPool
MEMOBJPOOL_GENERATE_PATH := server/lib
MEMOBJPOOL_GENERATE_TARGET := $(MEMOBJPOOL_GENERATE_PATH)/libmemobjpool.so
MEMOBJPOOL_CXX_FILES := server/src/HCNSMemoryAllocator.cpp \
						server/src/HCNSMemoryPool.cpp \
						server/src/HCNSObjectPoolAllocator.cpp

MEMOBJPOOL_OBJ_FILES := $(MEMOBJPOOL_CXX_FILES:server/src/HCNSMemoryAllocator.cpp \
											   server/src/HCNSObjectPoolAllocator.cpp = \
						$(MEMOBJPOOL_BUILD_DIR)/HCNSMemoryAllocator_cpp.o \
						$(MEMOBJPOOL_BUILD_DIR)/HCNSObjectPoolAllocator_cpp.o)

CLIENT_BUILD_DIR := $(BUILD_DIR)/Client
CLIENT_BUILD_TARGET := $(CLIENT_BUILD_DIR)/Client
CLIENT_CXX_FILES := $(wildcard client/src/*.cpp)
CLIENT_OBJ_FILES := $(CLIENT_CXX_FILES:client/src/%.cpp=$(CLIENT_BUILD_DIR)/%client_cpp.o)

SERVER_BUILD_DIR := $(BUILD_DIR)/Server
SERVER_BUILD_TARGET := $(SERVER_BUILD_DIR)/Server
SERVER_CXX_FILES := $(wildcard server/src/*.cpp)
SERVER_OBJ_FILES := $(SERVER_CXX_FILES:server/src/%.cpp=$(SERVER_BUILD_DIR)/%server_cpp.o)

.PHONY: clean all build
all: $(SERVER_BUILD_TARGET) $(CLIENT_BUILD_TARGET) $(MEMOBJPOOL_GENERATE_TARGET)

$(CLIENT_BUILD_DIR)/%client_cpp.o: client/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iclient/include -c $< -o $@

$(MEMOBJPOOL_BUILD_DIR)/HCNSMemoryAllocator_cpp.o:server/src/HCNSMemoryAllocator.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iserver/include -c server/src/HCNSMemoryAllocator.cpp -o $(MEMOBJPOOL_BUILD_DIR)/HCNSMemoryAllocator_cpp.o

$(MEMOBJPOOL_BUILD_DIR)/HCNSObjectPoolAllocator_cpp.o:server/src/HCNSObjectPoolAllocator.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iserver/include -c server/src/HCNSObjectPoolAllocator.cpp -o $(MEMOBJPOOL_BUILD_DIR)/HCNSObjectPoolAllocator_cpp.o


$(SERVER_BUILD_DIR)/%server_cpp.o: server/src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -Iserver/include -c $< -o $@

$(CLIENT_BUILD_TARGET): $(CLIENT_OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(MEMOBJPOOL_GENERATE_TARGET): $(MEMOBJPOOL_BUILD_DIR)/HCNSMemoryAllocator_cpp.o $(MEMOBJPOOL_BUILD_DIR)/HCNSObjectPoolAllocator_cpp.o $(MEMOBJPOOL_BUILD_DIR)/HCNSMemoryAllocator_cpp.o
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(SERVER_BUILD_TARGET): $(SERVER_OBJ_FILES) $(MEMOBJPOOL_GENERATE_TARGET)
	$(CXX) $(CXXFLAGS) -L$(MEMOBJPOOL_GENERATE_PATH) -lmemobjpool $^ -o $@

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR) $(MEMOBJPOOL_GENERATE_TARGET)
