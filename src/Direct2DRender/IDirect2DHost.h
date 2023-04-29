#pragma once

#include <d2d1.h>
#include "Direct2DViewPort.h"

class IDirect2DHost
{
public:
    static IDirect2DHost* getInstance();
    virtual bool initialize(HWND hWnd, float bgR, float bgG, float bgB) = 0;
    virtual void resize(int width, int height) = 0;
    virtual void release() = 0;
    virtual void addDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort) = 0;
    virtual void removeDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort) = 0;
    virtual void renderOnce() = 0;
    virtual ID2D1RenderTarget* getWicBitmapRenderTarget() = 0;
    virtual ID2D1RenderTarget* getDxgiSurfaceRenderTarget() = 0;
    virtual ID2D1SolidColorBrush* getBrush() = 0;
};

