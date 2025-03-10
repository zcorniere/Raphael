#pragma once

#include "Engine/Math/Transform.hxx"

class RSceneComponent : public RObject
{
    RTTI_DECLARE_TYPEINFO(RSceneComponent, RObject)
public:
    RSceneComponent() = default;
    virtual ~RSceneComponent() = default;

    void SetRelativeLocation(const FVector3& Location);
    void SetRelativeRotation(const FQuaternion& Rotation);
    void SetRelativeScale(const FVector3& Scale);
    void SetRelativeTransform(const FTransform& Transform);

    const FTransform& GetRelativeTransform() const;
    bool IsTransformDirty() const;
    bool IsRenderStateDirty() const;

    void MarkRenderStateDirty();
    void ClearRenderStateDirtyFlag();

    void MarkTransformDirty();
    void ClearDirtyTransformFlag();

private:
    FTransform RelativeTransform;

    uint8 bTransformDirty : 1 = false;
    uint8 bRenderStateDirty : 1 = false;
};

inline void RSceneComponent::SetRelativeLocation(const FVector3& Location)
{
    RelativeTransform.SetLocation(Location);
    bTransformDirty = true;
}

inline void RSceneComponent::SetRelativeRotation(const FQuaternion& Rotation)
{
    RelativeTransform.SetRotation(Rotation);
    bTransformDirty = true;
}

inline void RSceneComponent::SetRelativeScale(const FVector3& Scale)
{
    RelativeTransform.SetScale(Scale);
    bTransformDirty = true;
}

inline void RSceneComponent::SetRelativeTransform(const FTransform& Transform)
{
    RelativeTransform = Transform;
    bTransformDirty = true;
}

inline const FTransform& RSceneComponent::GetRelativeTransform() const
{
    return RelativeTransform;
}

inline void RSceneComponent::MarkRenderStateDirty()
{
    bRenderStateDirty = true;
}

inline void RSceneComponent::MarkTransformDirty()
{
    bTransformDirty = true;
}

inline bool RSceneComponent::IsTransformDirty() const
{
    return bTransformDirty;
}

inline bool RSceneComponent::IsRenderStateDirty() const
{
    return bRenderStateDirty;
}

inline void RSceneComponent::ClearDirtyTransformFlag()
{
    bTransformDirty = false;
}

inline void RSceneComponent::ClearRenderStateDirtyFlag()
{
    bRenderStateDirty = false;
}
