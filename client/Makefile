TARGET:= Client
SRC:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRC))

$(TARGET):$(OBJS)
	$(CXX) $^ -o $@

all:
	echo $(SRC)

.PHONY:
clean:
	$(RM) $(OBJS) $(TARGET)