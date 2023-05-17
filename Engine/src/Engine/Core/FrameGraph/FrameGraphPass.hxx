#pragma once

class FrameGraphPassResources;
class FrameGraphBuilder;

template <typename T, typename Data>
concept CValidFrameGraphExecutionLambda = std::is_invocable_r_v<void, T, const Data &, FrameGraphPassResources &> &&
                                          sizeof(T) < 10'000;    // Shouldn't be larger that 10 ko

template <typename T, typename Data>
concept CValidFrameGraphSetupLambda = std::is_invocable_r_v<void, T, FrameGraphBuilder &, Data &> && sizeof(T) < 10'000;

class FrameGraphPass : public RObject
{
    RPH_NONCOPYABLE(FrameGraphPass)
public:
    FrameGraphPass() = default;
    virtual ~FrameGraphPass() = default;

    virtual void operator()(FrameGraphPassResources &) = 0;
};

template <typename Data, typename Execute>
    requires CValidFrameGraphExecutionLambda<Execute, Data>
class TFrameGraphPass : public FrameGraphPass
{
public:
    TFrameGraphPass() = delete;
    explicit TFrameGraphPass(Execute &&Exec, Data InData = {}): ExecFunction(Exec), m_Data(InData) {}

    virtual void operator()(FrameGraphPassResources &Resource) override { ExecFunction(m_Data, Resource); }

private:
    Execute ExecFunction;
    Data m_Data;

    friend class FrameGraph;
};
