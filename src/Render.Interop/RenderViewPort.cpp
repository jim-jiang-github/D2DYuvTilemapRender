#include "pch.h"
#include "RenderViewPort.h"

RenderViewPort::RenderViewPort()
{
    pRenderGraphics = gcnew RenderGraphics();
    System::Action^ callback = gcnew System::Action(this, &RenderViewPort::OnRenderedInternal);
    RenderedCallback nativeCallback = static_cast<RenderedCallback>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(callback).ToPointer());

    pDirect2DViewPort = new Direct2DViewPort(nativeCallback);
}

RenderViewPort::~RenderViewPort()
{
    if (pDirect2DViewPort)
    {
        pDirect2DViewPort = nullptr;
    }
}

void RenderViewPort::SetBounds(float x, float y, float w, float h)
{
    mX = x;
    mY = y;
    mW = w;
    mH = h;
    if (pDirect2DViewPort)
    {
        pDirect2DViewPort->SetBounds(x, y, w, h);
    }
}

void RenderViewPort::OnRendered(RenderGraphics^ g, float clientWidth, float clientHeight)
{

}

void RenderViewPort::OnRenderedInternal()
{
    OnRendered(pRenderGraphics, mW, mH);
}

Direct2DViewPort* RenderViewPort::getDirect2DViewPort()
{
    return pDirect2DViewPort;
}