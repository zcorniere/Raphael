#pragma once

namespace ecs
{

class RWorld;
using FEntity = uint64;

/// Create a new world
[[nodiscard]]
Ref<RWorld> CreateWorld();

/// Destroy the given world
/// Note: The world may not be deleted right away but it will start the deletion process. The Ref given as argument is
/// expected to be the last one.
void DestroyWorld(Ref<RWorld>& World);

}    // namespace ecs
