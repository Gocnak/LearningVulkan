VULKAN_SDK_PATH = /home/nick/Documents/Vulkan/1.1.85.0/x86_64
THIRD_PARTY_INCLUDE_PATH = /home/nick/Documents/Vulkan/LearningVulkan/LearningVulkan/inc
CFLAGS = -std=c++17 -I$(VULKAN_SDK_PATH)/include -I$(THIRD_PARTY_INCLUDE_PATH)
LDFLAGS = -L$(VULKAN_SDK_PATH)/lib `pkg-config --static --libs glfw3` -lvulkan

VulkanTest: main.cpp
	g++ $(CFLAGS) -o VulkanTest main.cpp MDLReader.h MDLReader.cpp Vertex.h $(LDFLAGS)

.PHONY: test clean

test: VulkanTest
	LD_LIBRARY_PATH=$(VULKAN_SDK_PATH)/lib VK_LAYER_PATH=$(VULKAN_SDK_PATH)/etc/explicit_layer.d ./VulkanTest

clean:
	rm -f VulkanTest
