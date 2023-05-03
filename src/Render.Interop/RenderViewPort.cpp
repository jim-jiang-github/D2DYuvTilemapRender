#include "RenderViewPort.h"

RenderViewPort::RenderViewPort()
{
    mYuvRenderedCallbackWarpper = gcnew System::Action(this, &RenderViewPort::OnRenderedInternal);
    YuvRenderedCallback yuvRenderedCallbackNative = static_cast<YuvRenderedCallback>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(mYuvRenderedCallbackWarpper).ToPointer());

    pDirect2DViewPort = new Direct2DViewPort(yuvRenderedCallbackNative);
    pRenderGraphics = gcnew RenderGraphics(pDirect2DViewPort);
}

RenderViewPort::~RenderViewPort()
{
    if (pDirect2DViewPort)
    {
        pDirect2DViewPort = nullptr;
    }
    if (mYuvRenderedCallbackWarpper)
    {
        mYuvRenderedCallbackWarpper = nullptr;
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
        pDirect2DViewPort->setBounds(x, y, w, h);
    }
}

void RenderViewPort::OnFrame(System::IntPtr yData, System::IntPtr uData, System::IntPtr vData, int yStride, int uStride, int vStride, int width, int height)
{
    if (pDirect2DViewPort)
    {
        unsigned char* pYData = reinterpret_cast<unsigned char*>(yData.ToPointer());
        unsigned char* pUData = reinterpret_cast<unsigned char*>(uData.ToPointer());
        unsigned char* pVData = reinterpret_cast<unsigned char*>(vData.ToPointer());
        pDirect2DViewPort->onYuvFrame(pYData, pUData, pVData, yStride, uStride, vStride, width, height);
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