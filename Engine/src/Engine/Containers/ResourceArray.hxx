#pragma once

/// Interface used to type erase an Array<T> in an RHI buffer context
/// where we don't care that much about the type. We care much more about the size of the data and the data itself.
class IResourceArrayInterface
{

public:
    virtual ~IResourceArrayInterface()
    {
    }

    /// @return the raw pointer to the data
    virtual const void* GetData() const = 0;
    /// @return the size (in bytes) of the data in the array
    virtual uint32 GetByteSize() const = 0;
    /// @return the size (in bytes) of a item in the array
    ///
    /// Usually just a return sizeof(T) will suffice here
    virtual uint32 GetTypeSize() const = 0;
};

/// Array subtype used in the context of the RHI
template <typename Type>
class TResourceArray : public IResourceArrayInterface, public TArray<Type>
{
public:
    using TArray<Type>::TArray;
    TResourceArray() = default;
    TResourceArray(TResourceArray&&) = default;
    TResourceArray(const TResourceArray&) = default;
    TResourceArray& operator=(TResourceArray&&) = default;
    TResourceArray& operator=(const TResourceArray&) = default;

    virtual ~TResourceArray() = default;

    const void* GetData() const override
    {
        return this->Raw();
    }
    uint32 GetByteSize() const override
    {
        if (this->Size() > std::numeric_limits<uint32>::max() / sizeof(Type)) [[unlikely]] {
            checkNoEntry();
            return 0;
        }
        return this->ByteSize();
    }
    uint32 GetTypeSize() const override
    {
        return sizeof(Type);
    }
};
