#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>
#include "DColor.h";

using namespace System::Collections::Generic;
using namespace System;

public ref class RenderGraphics
{
internal:
    RenderGraphics();
    ~RenderGraphics();
public:
    void DrawRectangle(float x, float y, float w, float h, DColor32F color, float strokeWidth);
    void FillRectangle(float x, float y, float w, float h, DColor32F color);
    void DrawEllipse(float x, float y, float w, float h, DColor32F color, float strokeWidth);
    void FillEllipse(float x, float y, float w, float h, DColor32F color);
private:
    enum class RenderMethod
    {
        DrawRectangle,
        FillRectangle,
        DrawEllipse,
        FillEllipse
    };

    void DrawRectangleInternal(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth);
    void FillRectangleInternal(float x, float y, float w, float h, float a, float r, float g, float b);
    void DrawEllipseInternal(float x, float y, float w, float h, float a, float r, float g, float b, float strokeWidth);
    void FillEllipseInternal(float x, float y, float w, float h, float a, float r, float g, float b);
internal:
    void SuspendRender();
    void ResumeRender();
private:
    bool isNeedRefresh = false;
    int mRenderFlowCacheIndex = 0;
    List<Tuple<RenderMethod, array<Object^>^>^>^ mRenderFlowCache;
};

