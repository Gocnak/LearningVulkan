# Vulkan MDL Renderer
Very rudimentary Vulkan MDL renderer.

## Dependencies
You will need:
 * [Visual Studio 2017](https://docs.microsoft.com/en-us/visualstudio/install/install-visual-studio?view=vs-2017)
 * [Latest Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (Make sure to install it in `C:/VulkanSDK/`)

## Setup

Open `LearningVulkan.sln` and compile under the `x64` platform.

If there are linker issues, you may need to fix the include directory to find your version of Vulkan. To do that:
 * Right click LearningVulkan in the Solution Explorer, then select Properties
 * Go to `Linker -> General -> Additional Link Directories`
 * Correct the path in there to your installed VulkanSDK path (might just be the version number)
 * Go to `C/C++ -> General -> Additional Include Directories` and correct the path there to the correct Vulkan SDK path/number

## Run
You should be able to just run it with CTRL+F5 (or just F5 for debugging) through Visual Studio.