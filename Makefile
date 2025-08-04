CFLAGS = -std=c++17

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Use DEBUG=1 to disable NDEBUG
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0
else
    CFLAGS += -DNDEBUG -O2
endif

# Find all .cpp files recursively starting from current directory
SRCS := $(shell find . -name '*.cpp')

# Create corresponding object files replacing .cpp with .o
OBJS := $(SRCS:.cpp=.o)

TARGET = App

all: shaders $(TARGET)

$(TARGET): $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile rule for .o from .cpp
%.o: %.cpp
	@echo "Compiling $< ..."
	g++ $(CFLAGS) -c $< -o $@

.PHONY: all test clean shaders

shaders:
	cd shaders && ./compile.sh

test: all
	./$(TARGET)

clean: 
	rm -f $(OBJS) $(TARGET)
	rm -f shaders/*.spv


