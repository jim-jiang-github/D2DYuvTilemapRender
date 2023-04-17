#include "pch.h"
#include "RenderHost.h"

RenderHost::RenderHost(IDirect2DHost* direct2DHost)
{
    _renderViewPortList = gcnew List<RenderViewPort^>();
    pDirect2DHost = direct2DHost;
}

RenderHost::~RenderHost()
{
}

void RenderHost::Resize(int width, int height)
{
    if (pDirect2DHost)
    {
        pDirect2DHost->resize(width, height);
    }
}

void RenderHost::Release()
{
    if (_renderViewPortList)
    {
        _renderViewPortList->Clear();
        _renderViewPortList = nullptr;
    }
    if (pDirect2DHost)
    {
        pDirect2DHost->release();
        pDirect2DHost = nullptr;
    }
}

void RenderHost::AddRenderViewPort(RenderViewPort^ renderViewPort)
{
    if (pDirect2DHost)
    {
        pDirect2DHost->addDirect2DViewPort((std::shared_ptr<Direct2DViewPort>)renderViewPort->getDirect2DViewPort());
    }
    if (_renderViewPortList)
    {
        _renderViewPortList->Add(renderViewPort);
    }
}

void RenderHost::RemoveRenderViewPort(RenderViewPort^ renderViewPort)
{
    if (_renderViewPortList)
    {
        _renderViewPortList->Remove(renderViewPort);
    }
    if (pDirect2DHost)
    {
        pDirect2DHost->removeDirect2DViewPort((std::shared_ptr<Direct2DViewPort>)renderViewPort->getDirect2DViewPort());
    }
}

void RenderHost::RenderOnce()
{
    if (pDirect2DHost)
    {
        pDirect2DHost->renderOnce();
    }
}