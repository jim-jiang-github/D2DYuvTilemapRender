#include "pch.h"
#include "RenderGraphics.h"
#include "IDirect2DHost.h"

RenderGraphics::RenderGraphics()
{
}

void RenderGraphics::DrawRectangle(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth)
{
    auto renderTarget = IDirect2DHost::getInstance()->getRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r, g, b, a });
    renderTarget->DrawRectangle({ x, y, x + w, y + h }, brush, strokeWidth);
}

void RenderGraphics::FillRectangle(float x, float y, float w, float h, float a, float r, float g, float b)
{
    auto renderTarget = IDirect2DHost::getInstance()->getRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r, g, b, a });
    renderTarget->FillRectangle({ x, y, x + w, y + h }, brush);
}

void RenderGraphics::DrawEllipse(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth)
{
    auto renderTarget = IDirect2DHost::getInstance()->getRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r, g, b, a });
    renderTarget->DrawEllipse({ x + w / 2, y + h / 2, w / 2, h / 2 }, brush, strokeWidth);
}

void RenderGraphics::FillEllipse(float x, float y, float w, float h, float a, float r, float g, float b)
{
    auto renderTarget = IDirect2DHost::getInstance()->getRenderTarget();
    if (!renderTarget)
    {
        return;
    }
    auto brush = IDirect2DHost::getInstance()->getBrush();
    brush->SetColor({ r, g, b, a });
    renderTarget->FillEllipse({ x + w / 2 , y + h / 2 , w / 2, h / 2 }, brush);
}