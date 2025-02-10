#pragma once

/// Set 1 - Scene information

/// Set 2 - Rendering data

// layout(std140, set = 2, binding = 0) uniform Camera
layout(std140, set = 0, binding = 0) uniform Camera
{
    mat4 viewproj;
    mat4 view;
    mat4 proj;
}
u_Camera;
