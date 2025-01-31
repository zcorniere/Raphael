#pragma once

#include "Engine/Math/ViewPoint.hxx"

namespace ecs
{

template <typename T>
struct TCameraComponent {
    RTTI_DECLARE_TYPEINFO_MINIMAL(TCameraComponent);

public:
    bool bIsActive = false;
    Math::TViewPoint<T> ViewPoint;
};

using FCameraComponent = TCameraComponent<float>;

}    // namespace ecs
