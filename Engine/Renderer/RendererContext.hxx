#pragma once

namespace Raphael
{

class RendererContext : public RObject
{
public:
    virtual ~RendererContext()
    {
    }
    virtual void Init() = 0;

    static Ref<RendererContext> Create();
};

}    // namespace Raphael
