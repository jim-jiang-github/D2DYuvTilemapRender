#include "RenderShapeDescription.h"

RenderShapeDescription::RenderShapeDescription(RenderType renderType, std::vector<std::any> arguments)
    : mRenderType(renderType), mArguments(arguments)
{

}

RenderType RenderShapeDescription::GetRenderType() const {
    return mRenderType;
}

std::vector<std::any> RenderShapeDescription::GetArguments() const {
    return mArguments;
}