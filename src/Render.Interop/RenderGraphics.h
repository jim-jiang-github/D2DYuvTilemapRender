#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>

public ref class RenderGraphics
{
internal:
    RenderGraphics();
public:
    void DrawRectangle(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth);
    void FillRectangle(float x, float y, float w, float h, float a, float r, float g, float b);
    void DrawEllipse(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth);
    void FillEllipse(float x, float y, float w, float h, float a, float r, float g, float b);
};

