BUILD_DIR:=build
TARGET:= $(BUILD_DIR)/Server
CXXFLAGS:= -O0 -Wall -Wextra -g -lpthread -Iinclude -std=c++11

CXX_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(CXX_FILES:src/%.cpp=$(BUILD_DIR)/%_cpp.o)

.PHONY: clean all build
all: build

#compile cpp files and generate obj files
build/%_cpp.o: src/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

#link obj files to excutable file
build: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)

clean:
	@echo "Cleaning up..."
	rm -rf $(BUILD_DIR)