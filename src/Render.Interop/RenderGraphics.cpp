#include "RenderGraphics.h"
#include "IDirect2DHost.h"
#include <functional>

RenderGraphics::RenderGraphics()
{
    mRenderFlowCache = gcnew List<Tuple<RenderMethod, array<Object^>^>^>();
}
RenderGraphics::~RenderGraphics()
{
}

void RenderGraphics::DrawRectangle(float x, float y, float w, float h, DColor32F color, float strokeWidth)
{
    mRenderFlowCache->Add(gcnew Tuple<RenderMethod, array<Object^>^>(RenderMethod::DrawRectangle, gcnew array<Object^>{x, y, w, h, color.A, color.R, color.G, color.B, strokeWidth}));
}

void RenderGraphics::FillRectangle(float x, float y, float w, float h, DColor32F color)
{
    mRenderFlowCache->Add(gcnew Tuple<RenderMethod, array<Object^>^>(RenderMethod::FillRectangle, gcnew array<Object^>{x, y, w, h, color.A, color.R, color.G, color.B}));
}

void RenderGraphics::DrawEllipse(float x, float y, float w, float h, DColor32F color, float strokeWidth)
{
    mRenderFlowCache->Add(gcnew Tuple<RenderMethod, array<Object^>^>(RenderMethod::DrawEllipse, gcnew array<Object^>{x, y, w, h, color.A, color.R, color.G, color.B, strokeWidth}));
}

void RenderGraphics::FillEllipse(float x, float y, float w, float h, DColor32F color)
{
    mRenderFlowCache->Add(gcnew Tuple<RenderMethod, array<Object^>^>(RenderMethod::FillEllipse, gcnew array<Object^>{x, y, w, h, color.A, color.R, color.G, color.B}));
}

void RenderGraphics::DrawRectangleInternal(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth)
{
    auto renderTarget = IDirect2DHost::getInstance()->getDxgiSurfaceRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r,g,b,a });
    renderTarget->DrawRectangle({ x, y, x + w, y + h }, brush, strokeWidth);
}

void RenderGraphics::FillRectangleInternal(float x, float y, float w, float h, float a, float r, float g, float b)
{
    auto renderTarget = IDirect2DHost::getInstance()->getDxgiSurfaceRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r,g,b,a });
    renderTarget->FillRectangle({ x, y, x + w, y + h }, brush);
}

void RenderGraphics::DrawEllipseInternal(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth)
{
    auto renderTarget = IDirect2DHost::getInstance()->getDxgiSurfaceRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r,g,b,a });
    renderTarget->DrawEllipse({ x + w / 2, y + h / 2, w / 2, h / 2 }, brush, strokeWidth);
}

void RenderGraphics::FillEllipseInternal(float x, float y, float w, float h, float a, float r, float g, float b)
{
    auto renderTarget = IDirect2DHost::getInstance()->getDxgiSurfaceRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r,g,b,a });
    renderTarget->FillEllipse({ x + w / 2 , y + h / 2 , w / 2, h / 2 }, brush);
}

void RenderGraphics::SuspendRender()
{
    mRenderFlowCache->Clear();
}

void RenderGraphics::ResumeRender()
{
    for (int i = 0; i < mRenderFlowCache->Count; i++) {
        Tuple<RenderMethod, array<Object^>^>^ item = mRenderFlowCache[i];
        RenderMethod name = item->Item1;
        array<Object^>^ args = item->Item2;
        switch (name) {
        case RenderMethod::DrawRectangle:
            DrawRectangleInternal((float)args[0], (float)args[1], (float)args[2], (float)args[3], (float)args[4], (float)args[5], (float)args[6], (float)args[7], (float)args[8]);
            break;
        case RenderMethod::FillRectangle:
            FillRectangleInternal((float)args[0], (float)args[1], (float)args[2], (float)args[3], (float)args[4], (float)args[5], (float)args[6], (float)args[7]);
            break;
        case RenderMethod::DrawEllipse:
            DrawEllipseInternal((float)args[0], (float)args[1], (float)args[2], (float)args[3], (float)args[4], (float)args[5], (float)args[6], (float)args[7], (float)args[8]);
            break;
        case RenderMethod::FillEllipse:
            FillEllipseInternal((float)args[0], (float)args[1], (float)args[2], (float)args[3], (float)args[4], (float)args[5], (float)args[6], (float)args[7]);
            break;
        default:
            break;
        }
    }
}