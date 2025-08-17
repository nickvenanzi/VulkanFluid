CFLAGS = -std=c++17 -Iinclude

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Use DEBUG=1 to disable NDEBUG
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0
else
    CFLAGS += -DNDEBUG -O2
endif

# Directories
SRCDIR := src
INCDIR := include
OBJDIR := build

# Find all .cpp files in /src
SRCS := $(shell find $(SRCDIR) -name '*.cpp')

# Create object file list in $(OBJDIR), preserving subdirectory structure
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

TARGET = App

all: shaders $(TARGET)

$(TARGET): $(OBJS)
	g++ $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile rule for .o from .cpp
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $< ..."
	g++ $(CFLAGS) -c $< -o $@

.PHONY: all test clean shaders

shaders:
	cd shaders && ./compile.sh

test: all
	./$(TARGET)

clean: 
	rm -rf $(OBJDIR) $(TARGET)
	rm -f shaders/*.spv


