#pragma once
#include <vector>
#include <any>

enum class RenderType
{
    DrawRectangle,
    FillRectangle,
    DrawEllipse,
    FillEllipse
};

class RenderShapeDescription
{
public:
    RenderShapeDescription(RenderType renderType, std::vector<std::any> arguments);

    RenderType GetRenderType() const;
    std::vector<std::any> GetArguments() const;

private:
    RenderType mRenderType;
    std::vector<std::any> mArguments;
};
