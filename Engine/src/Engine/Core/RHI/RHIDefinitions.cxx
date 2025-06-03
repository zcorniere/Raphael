#include "Engine/Core/RHI/RHIDefinitions.hxx"

uint32 GetSizeOfElementType(EVertexElementType Type)
{
    switch (Type)
    {
        case EVertexElementType::Float1:
            return sizeof(float);
        case EVertexElementType::Float2:
            return sizeof(float) * 2;
        case EVertexElementType::Float3:
            return sizeof(float) * 3;
        case EVertexElementType::Float4:
            return sizeof(float) * 4;

        case EVertexElementType::Uint1:
            return sizeof(uint32);
        case EVertexElementType::Uint2:
            return sizeof(uint32) * 2;
        case EVertexElementType::Uint3:
            return sizeof(uint32) * 3;
        case EVertexElementType::Uint4:
            return sizeof(uint32) * 4;

        case EVertexElementType::Int1:
            return sizeof(int32);
        case EVertexElementType::Int2:
            return sizeof(int32) * 2;
        case EVertexElementType::Int3:
            return sizeof(int32) * 3;
        case EVertexElementType::Int4:
            return sizeof(int32) * 4;
    }
    checkNoEntry();
    return 0;
}
