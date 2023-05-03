#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>
#include "DColor.h";
#include "Direct2DViewPort.h"

public ref class RenderGraphics
{
internal:
    RenderGraphics(Direct2DViewPort* direct2DViewPort);
    ~RenderGraphics();
public:
    void DrawRectangle(float x, float y, float w, float h, DColor32F color, float strokeWidth);
    void FillRectangle(float x, float y, float w, float h, DColor32F color);
    void DrawEllipse(float centerX, float centerY, float radiusX, float radiusY, DColor32F color, float strokeWidth);
    void FillEllipse(float centerX, float centerY, float radiusX, float radiusY, DColor32F color);

private:
    Direct2DViewPort* mDirect2DViewPort = nullptr;
};

