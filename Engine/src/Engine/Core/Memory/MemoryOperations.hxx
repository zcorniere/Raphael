#pragma once

template <typename ElementType, typename SizeType>
FORCEINLINE void ConstructItems(ElementType* Ptr, SizeType Count)
{
    checkSlow(Count >= 0);
    checkSlow(Ptr);

    std::memset(Ptr, 0, Count * sizeof(ElementType));
    // Nothing to do if the type is trivially destructible
    if constexpr (std::is_trivially_default_constructible_v<ElementType>) {
        return;
    }
    if constexpr (std::is_default_constructible_v<ElementType>) {
        while (Count) {
            new (Ptr) ElementType();
            ++Ptr;
            --Count;
        }
    }
}

template <typename ElementType, typename SizeType>
FORCEINLINE void DestructItems(ElementType* Ptr, SizeType Count)
{
    checkSlow(Count >= 0);
    checkSlow(Ptr);

    // Nothing to do if the type is trivially destructible
    if constexpr (std::is_trivially_destructible_v<ElementType>) {
        return;
    }
    while (Count) {
        Ptr->ElementType::~ElementType();
        ++Ptr;
        --Count;
    }
}

template <typename ElementType, typename SizeType>
FORCEINLINE void MoveItems(ElementType* Destination, ElementType* Source, SizeType Count)
{
    checkSlow(Count >= 0);
    checkSlow(Destination != nullptr);
    checkSlow(Source != nullptr);

    // Nothing to do if the type is trivially destructible
    if constexpr (std::is_trivially_copy_constructible_v<ElementType>) {
        std::memmove(Destination, Source, Count * sizeof(ElementType));
    } else {
        while (Count) {
            new (Destination) ElementType(*Source);
            ++Destination;
            (Source++)->ElementType::~ElementType();
            --Count;
        }
    }
}

template <typename ElementType, typename SizeType>
/// @brief Move items from Source to Destination
/// Destination must be preallocated
FORCEINLINE void CopyItems(ElementType* Destination, const ElementType* Source, SizeType Count)
{
    checkSlow(Count >= 0);
    checkSlow(Destination != nullptr);
    checkSlow(Source != nullptr);

    // Nothing to do if the type is trivially destructible
    if constexpr (std::is_trivially_copyable_v<ElementType>) {
        std::memmove((void*)Destination, Source, Count * sizeof(ElementType));
    } else {
        while (Count) {
            new (Destination) ElementType(*Source);
            ++Source;
            ++Destination;
            --Count;
        }
    }
}
