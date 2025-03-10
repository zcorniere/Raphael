#pragma once

template <typename TFunctionSignature>
class TDelegate
{
private:
    class ILamdaInterface
    {
    public:
        virtual ~ILamdaInterface() = default;
        virtual std::function<TFunctionSignature> GetFunction() const = 0;
    };

    template <typename T>
    class TWeakLambda : public ILamdaInterface
    {
    public:
        TWeakLambda(WeakRef<T> InWeakPtr, std::function<TFunctionSignature>&& InFunction)
            : WeakPtr(InWeakPtr), Function(std::move(InFunction))
        {
        }
        virtual ~TWeakLambda() = default;

        virtual std::function<TFunctionSignature> GetFunction() const override
        {
            if (!WeakPtr.IsValid()) {
                return {};
            }
            return Function;
        }

    private:
        WeakRef<T> WeakPtr = nullptr;
        std::function<TFunctionSignature> Function;
    };

    class TLambda : public ILamdaInterface
    {
    public:
        TLambda(std::function<TFunctionSignature>&& InFunction): Function(std::move(InFunction))
        {
        }
        virtual ~TLambda() = default;

        virtual std::function<TFunctionSignature> GetFunction() const override
        {
            return Function;
        }

    private:
        std::function<TFunctionSignature> Function;
    };

public:
    TDelegate() = default;
    ~TDelegate() = default;

    void Add(std::function<TFunctionSignature>&& Function)
    {
        Functions.Add(std::make_unique<TLambda>(std::move(Function)));
    }

    template <typename T>
    void Add(WeakRef<T> OwnerPtr, std::function<TFunctionSignature>&& Function)
    {
        Functions.Add(std::make_unique<TWeakLambda<T>>(OwnerPtr, std::move(Function)));
    }
    template <typename T>
    void Add(T* OwnerPtr, std::function<TFunctionSignature>&& Function)
    {
        Functions.Add(std::make_unique<TWeakLambda<T>>(OwnerPtr, std::move(Function)));
    }

    void AddUnique(std::function<TFunctionSignature>&& Function)
    {
        if (std::find(Function->begin(), Function->end(), Function) == Function->end()) {
            Functions.Add(std::make_unique<TLambda>(std::move(Function)));
        }
    }

    template <typename... TFunctionArgs>
    requires std::is_invocable_v<TFunctionSignature, TFunctionArgs&...>
    void Broadcast(TFunctionArgs&&... Args)
    {
        for (std::unique_ptr<ILamdaInterface>& Function: Functions) {
            std::function<TFunctionSignature> LambdaFunction = Function->GetFunction();
            if (LambdaFunction) {
                LambdaFunction(Args...);
            }
        }
    }

private:
    TArray<std::unique_ptr<ILamdaInterface>> Functions;
};
