TARGET:= Server
SRC:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRC))
CLANG:=

$(TARGET):$(OBJS)
	$(CLANG) $^ -o $@
all:
	echo $(SRC)

.PHONY:
clean:
	$(RM) $(OBJS) $(TARGET)