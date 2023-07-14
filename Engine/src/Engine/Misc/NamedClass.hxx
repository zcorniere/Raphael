#pragma once

/// Classes that implement this interface can be named, for debugging purposes
class NamedClass
{
public:
    /// Give the object a debug name
    virtual void SetName(std::string_view InName)
    {
        Name = InName;
    }

    /// Return the debug name of the object
    FORCEINLINE std::string_view GetName() const
    {
        return Name;
    }

private:
    std::string Name;
};

template <class T>
/// Easy access to the typename of the current class
class NamedClassWithTypeName : public NamedClass
{
public:
    std::string_view GetTypeName() const
    {
        return GetTypeName_Internal<T>();
    }

protected:
    /// Return the typename
    virtual std::string_view GetTypeName_Internal() const
    {
        return type_name<T>();
    }
};
