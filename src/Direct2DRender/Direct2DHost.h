#pragma once

#include <Windows.h>
#include <dxgi.h>
#include <d3d11.h>
#include <d2d1.h>
#include <d2d1_3.h>
#include <d2d1helper.h>
#include "IDirect2DHost.h"
#include "Direct2DViewPort.h"
#include <atlbase.h>
#include <vector>
#include <atomic>
#include <mutex>

class Direct2DHost :public IDirect2DHost
{
public:
    Direct2DHost() {}
    ~Direct2DHost() {}
    virtual bool initialize(HWND hWnd, float bgR, float bgG, float bgB) override;
    virtual void resize(int width, int height) override;
    virtual void release() override;
    virtual void addDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort) override;
    virtual void removeDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort) override;
    virtual void renderOnce() override;

    virtual ID2D1RenderTarget* getWicBitmapRenderTarget() override;
    virtual ID2D1RenderTarget* getDxgiSurfaceRenderTarget() override;
    virtual ID2D1SolidColorBrush* getBrush() override;

private:
    DXGI_SWAP_CHAIN_DESC createSwapChain(HWND hWnd);
    HRESULT CreateDeviceAndSwapChain(HWND hWnd, ID3D11Device** d3DDevice, IDXGISwapChain** swapChain);
    bool resizeSwapChainBuffers();
    bool updateOrCreateDxgiSurfaceRenderTarget();
    bool updateOrCreateYuvRenderBitmap();
    bool updateOrCreateWicBitmapRenderTarget();
    bool updateOrCreateMaskRenderBitmap();
    void onRender();

private:
    ID2D1Factory1* pD2dFactory = nullptr;
    IWICImagingFactory* pWICImagingFactory = nullptr;
    IDXGISwapChain* pSwapChain = nullptr;
    ID2D1RenderTarget* pWicBitmapRenderTarget = nullptr;
    ID2D1RenderTarget* pDxgiSurfaceRenderTarget = nullptr;
    ID2D1SolidColorBrush* pBrush = nullptr;

    int mLastWidth = 0;
    int mLastHeight = 0;
    bool mIsNeedResize = false;
    ID2D1Bitmap* pYuvRenderBitmap = nullptr;
    IWICBitmap* pMaskRenderBitmap = nullptr;

    D2D1_COLOR_F mBackgroundColor = { 0, 0, 0, 0 };
    std::vector<std::shared_ptr<Direct2DViewPort>> mVecViewPort;
};

