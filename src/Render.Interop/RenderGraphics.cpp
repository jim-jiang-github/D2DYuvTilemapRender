#include "RenderGraphics.h"
#include "IDirect2DHost.h"
#include <functional>
#include <any>

RenderGraphics::RenderGraphics(Direct2DViewPort* direct2DViewPort)
{
    mDirect2DViewPort = direct2DViewPort;
}
RenderGraphics::~RenderGraphics()
{
    mDirect2DViewPort = nullptr;
}

void RenderGraphics::DrawRectangle(float x, float y, float w, float h, DColor32F color, float strokeWidth)
{
    std::vector<std::any> arguments = {
        x,
        y,
        w,
        h,
        color.A,
        color.R,
        color.G,
        color.B,
        strokeWidth
    };
    std::shared_ptr<RenderShapeDescription> renderShapeDescription = std::make_shared<RenderShapeDescription>(RenderType::DrawRectangle, arguments);
    mDirect2DViewPort->addRenderShape(renderShapeDescription);
}

void RenderGraphics::FillRectangle(float x, float y, float w, float h, DColor32F color)
{
    std::vector<std::any> arguments = {
        x,
        y,
        w,
        h,
        color.A,
        color.R,
        color.G,
        color.B
    };
    std::shared_ptr<RenderShapeDescription> renderShapeDescription = std::make_shared<RenderShapeDescription>(RenderType::FillRectangle, arguments);
    mDirect2DViewPort->addRenderShape(renderShapeDescription);
}

void RenderGraphics::DrawEllipse(float centerX, float centerY, float radiusX, float radiusY, DColor32F color, float strokeWidth)
{
    std::vector<std::any> arguments = {
        centerX,
        centerY,
        radiusX,
        radiusY,
        color.A,
        color.R,
        color.G,
        color.B,
        strokeWidth
    };
    std::shared_ptr<RenderShapeDescription> renderShapeDescription = std::make_shared<RenderShapeDescription>(RenderType::DrawEllipse, arguments);
    mDirect2DViewPort->addRenderShape(renderShapeDescription);
}

void RenderGraphics::FillEllipse(float centerX, float centerY, float radiusX, float radiusY, DColor32F color)
{
    std::vector<std::any> arguments = {
        centerX,
        centerY,
        radiusX,
        radiusY,
        color.A,
        color.R,
        color.G,
        color.B
    };
    std::shared_ptr<RenderShapeDescription> renderShapeDescription = std::make_shared<RenderShapeDescription>(RenderType::FillEllipse, arguments);
    mDirect2DViewPort->addRenderShape(renderShapeDescription);
}