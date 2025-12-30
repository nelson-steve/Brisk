# Brisk Engine
An open-source, C++20 game engine focused on modern rendering techniques and GPU optimization.

## Features
- **Infinite Perspective Matrix**
- **Clustered Lighting**
- **Deferred Rendering**
- **Meshlet Rendering (Indirect Draw)**
- **Mesh Processing**: Using `meshoptimizer`
- **Cascaded Shadow Maps**

## Graphics APIs
- **Vulkan** (Functional)
- **DirectX12** (Still in early development)

## Platforms
- **Windows**

# GPU used in development
- AMD Radeon RX 7900 GRE

## Requirements
- `VulkanSDK>=1.4`
  - Download from https://vulkan.lunarg.com/ and follow install instruction.
- Visual Studio 2022
## Build Instructions
- Clone the repo
```bash
git clone https://github.com/nelson-steve/Brisk.git
```
OR
- Clone without commit history for faster clone
```bash
git clone --depth=1 https://github.com/nelson-steve/Brisk.git
```
- Run `GenerateProjects-Windows.bat` in Brisk/Scripts directory
- Open Brisk.sln
- Build `Ctrl + Shift + B`
- BriskEditor should be startup project
- Run `F5` or press Local Windows Debugger

Helpful links:
- https://www.youtube.com/watch?v=BR2my8OE1Sc&list=PL0JVLUVCkk-l7CWCn3-cdftR0oajugYvd
- https://www.aortiz.me/2018/12/21/CG.html
- https://github.com/SaschaWillems/Vulkan
- https://learnopengl.com
