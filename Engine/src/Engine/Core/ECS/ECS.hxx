#pragma once

namespace ecs
{

class World;
using Entity = uint64;

/// Create a new world
[[nodiscard]]
Ref<World> CreateWorld();

/// Destroy the given world
/// Note: The world may not be deleted right away but it will start the deletion process. The Ref given as argument is
/// expected to be the last one.
void DestroyWorld(Ref<World>& World);

}    // namespace ecs
