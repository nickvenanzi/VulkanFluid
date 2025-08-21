# GPU-Accelerated Fluid Simulation (Navier–Stokes on Raspberry Pi 4 with Vulkan)

This project is a performance-tuned fluid dynamics simulation that discretizes the **Navier–Stokes equations** and accelerates computations using a **GPU** via **Vulkan**.  

Originally developed on macOS, the codebase has been ported and optimized to run on a **Raspberry Pi 4 Model B**, leveraging the **Broadcom BCM2711 SoC** (ARM Cortex-A72 CPU + VideoCore VI GPU).  

The goal is to demonstrate efficient GPU-based numerical methods for real-time or near real-time fluid simulation on low-power embedded hardware.

---

## Features
- ✅ **Navier–Stokes discretization** for incompressible fluids  
- ✅ **GPU acceleration** via Vulkan compute pipelines  
- ✅ **Optimized data structures** for memory bandwidth and cache efficiency  
- ✅ **Ported for Raspberry Pi 4 Model B** (BCM2711 SoC)  
- ✅ **Configurable resolution & time-step** for performance benchmarking  
- ✅ **Cross-platform C++17 codebase**  

---

## Demo Video
Here’s a preview of the simulation running on Raspberry Pi 4:

<video src="LiquidDropAnimation.mp4" controls autoplay loop muted width="600"></video>

---

## Installation

### Requirements
- Raspberry Pi 4 Model B (4GB+ recommended)  
- Vulkan driver support (`mesa-vulkan-drivers`)  
- C++17 compatible compiler (e.g., `g++` 10+)  
- CMake 3.16+  

### Setup
```bash
# Update & install dependencies
sudo apt update && sudo apt install -y build-essential cmake git mesa-vulkan-drivers vulkan-tools

# Clone repository
git clone https://github.com/nickvenanzi/VulkanFluid.git
cd VulkanFluid

# Build
make test
