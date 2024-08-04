# Raphael
Raphael is a game engine focused on letting me having fun. If you see something weird going on in the code, it is probably on purpose 'cause I find it funny.

## Feature
- Rendering backend throught a RHI
  - Currently only support Vulkan, but in theory it should be possible to port it to DX12
- Support Linux and Windows
- Custom implementation of an RTTI (cause why not)
- Custom Vector implementation (called Array)

## Build
### Linux
```bash
git submodule update --init --recursive
mkdir build && cd build && cmake .. -GNinja
ninja
```

### Window
Run CMake, and you can build it through you favorite IDE (probably).

## Inspiration
A lot of the code / architecture / ideas mainly come from Unreal Engine and Hazel Engine, so credit where credit is due.
