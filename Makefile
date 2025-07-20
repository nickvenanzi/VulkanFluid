CFLAGS = -std=c++17

LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

# Use DEBUG=1 to disable NDEBUG
ifeq ($(DEBUG),1)
    CFLAGS += -g -O0
else
    CFLAGS += -DNDEBUG -O2
endif

VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp $(LDFLAGS)

.PHONY: test clean shaders

shaders:
	cd shaders && ./compile.sh

test: shaders VulkanTest
	./VulkanTest

clean: 
	rm -f VulkanTest
	rm -f shaders/*.spv


