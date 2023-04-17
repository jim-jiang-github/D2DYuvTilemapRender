#include "pch.h"
#include "RenderGraphics.h"
#include "Direct2DViewPort.h"

public ref class RenderViewPort
{
public:
    RenderViewPort();
    ~RenderViewPort();

    property int X
    {
        int get() { return mX; }
    }
    property int Y
    {
        int get() { return mY; }
    }
    property int Width
    {
        int get() { return mW; }
    }
    property int Height
    {
        int get() { return mH; }
    }
    property int Left
    {
        int get() { return X; }
    }
    property int Top
    {
        int get() { return Y; }
    }
    property int Right
    {
        int get() { return X + Width; }
    }
    property int Bottom
    {
        int get() { return Y + Height; }
    }
    void SetBounds(float x, float y, float w, float h);
    virtual void OnRendered(RenderGraphics^ g, float clientWidth, float clientHeight);
internal:
    void OnRenderedInternal();
    Direct2DViewPort* getDirect2DViewPort();
private:
    RenderGraphics^ pRenderGraphics;
    Direct2DViewPort* pDirect2DViewPort = nullptr;
    float mX = 0;
    float mY = 0;
    float mW = 0;
    float mH = 0;
};