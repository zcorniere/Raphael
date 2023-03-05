#pragma once

#include "Engine/Renderer/RHI/RHIDefinitions.hxx"

struct RasterizerCreateInfo {
    ERasterizerFillMode FillMode = ERasterizerFillMode::Solid;
    ERasterizerCullMode CullMode = ERasterizerCullMode::ClockWise;
    ERasterizerDepthClipMode ClipMode = ERasterizerDepthClipMode::DepthClip;
    float DepthBias = 0.0f;

    bool AllowMSAA = false;
};
