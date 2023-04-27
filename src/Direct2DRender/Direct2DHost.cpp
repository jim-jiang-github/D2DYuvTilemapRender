#include "pch.h"
#include "Direct2DHost.h"

IDirect2DHost* IDirect2DHost::getInstance()
{
    static Direct2DHost instance;
    return &instance;
}
bool Direct2DHost::initialize(HWND hWnd, float bgR, float bgG, float bgB)
{
    std::lock_guard<std::mutex> guard(mRenderMutex);
    RECT rc;
    GetClientRect(hWnd, &rc);
    mLastWidth = rc.right - rc.left;
    mLastHeight = rc.bottom - rc.top;

    mBackgroundColor = { bgR, bgG, bgB, 1 };

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2; //Double buffer
    swapDesc.BufferDesc.Width = mLastWidth;
    swapDesc.BufferDesc.Height = mLastHeight;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.Windowed = TRUE; // Winodws mode
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    // Create D3D11 device and swap chain
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    HRESULT hr = S_OK;
    // on low-end pc, it may takes more than 10 seconds when system is busy
    //DL_ENTER_THIS_SCOPE_I("D3D11CreateDeviceAndSwapChain");
    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
    };
    UINT countOfDriverTypes = ARRAYSIZE(driverTypes);

    for (UINT driverTypeIndex = 0; driverTypeIndex < countOfDriverTypes; driverTypeIndex++) {
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,       // use default adapter
            driverTypes[driverTypeIndex],
            nullptr,       // no external software rasterizer
            createDeviceFlags,
            &featureLevel,       // use default set of feature levels
            1,
            D3D11_SDK_VERSION,
            &swapDesc,
            &pSwapChain,
            &pD3dDevice,
            nullptr,       // do not care about what feature level is chosen
            nullptr        // do not retain D3D device context
        );

        if (SUCCEEDED(hr))
            break;
        else
            LOG_W("hr:{:x}", hr);
    }

    if (!pD3dDevice)
    {
        LOG_E("pD3dDevice is null");
        return false;
    }
    hr = pD3dDevice->QueryInterface(&pDxgiDevice);
    if (FAILED(hr))
    {
        LOG_E("pD3dDevice->QueryInterface hr:{}", hr);
        return false;
    }

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory);
    if (FAILED(hr))
    {
        LOG_E("D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory) hr:{}", hr);
        return false;
    }

    hr = pFactory->CreateDevice(pDxgiDevice, &pD2dDevice);
    if (FAILED(hr))
    {
        LOG_E("pFactory->CreateDevice(pDxgiDevice, &pD2dDevice) hr:{}", hr);
        return false;
    }
    // 获取D2D设备上下文
    CComPtr<IDXGISurface> dxgiSurface;
    hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
    if (FAILED(hr))
    {
        return false;
    }
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE),
        0,
        0,
        D2D1_RENDER_TARGET_USAGE_NONE,
        D2D1_FEATURE_LEVEL_DEFAULT);

    hr = pFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &props, &pD2dRenderTarget);
    if (FAILED(hr))
    {
        return false;
    }

    pD2dRenderTarget->CreateSolidColorBrush(mBackgroundColor, &pBrush);

    return true;
}

void Direct2DHost::resize(int width, int height)
{
    std::lock_guard<std::mutex> guard(mRenderMutex);
    if (!pSwapChain || !pD2dRenderTarget)
    {
        return;
    }
    if (mLastWidth != width ||
        mLastHeight != height)
    {
        mLastWidth = width;
        mLastHeight = height;
        // Release the old render target and create a new one
        pD2dRenderTarget->Release();
        HRESULT hr = pSwapChain->ResizeBuffers(
            2,                // Double buffer
            width,         // New width
            height,        // New height
            DXGI_FORMAT_R8G8B8A8_UNORM, // New format
            0                 // No flags
        );
        if (FAILED(hr))
        {
            return;
        }
        CComPtr<IDXGISurface> dxgiSurface;
        hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
        if (FAILED(hr))
        {
            return;
        }
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_IGNORE),
            0,
            0,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT);
        hr = pFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &props, &pD2dRenderTarget);
        if (FAILED(hr))
        {
            return;
        }
        pD2dRenderTarget->CreateSolidColorBrush(mBackgroundColor, &pBrush);
        renderOnce();
    }

}

void Direct2DHost::release()
{
    if (pFactory)
    {
        pFactory->Release();
        pFactory = nullptr;
    }
    if (pD3dDevice)
    {
        pD3dDevice->Release();
        pD3dDevice = nullptr;
    }
    if (pD3dDeviceContext)
    {
        pD3dDeviceContext->Release();
        pD3dDeviceContext = nullptr;
    }
    if (pD2dDevice)
    {
        pD2dDevice->Release();
        pD2dDevice = nullptr;
    }
    if (pD2dDeviceContext)
    {
        pD2dDeviceContext->Release();
        pD2dDeviceContext = nullptr;
    }
    if (pDxgiDevice)
    {
        pDxgiDevice->Release();
        pDxgiDevice = nullptr;
    }
    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }
    if (pD2dRenderTarget)
    {
        pD2dRenderTarget->Release();
        pD2dRenderTarget = nullptr;
    }
    if (pBrush)
    {
        pBrush->Release();
        pBrush = nullptr;
    }
}

void Direct2DHost::addDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort)
{
    auto iter = std::find(mVecViewPort.begin(), mVecViewPort.end(), viewPort);
    if (iter == mVecViewPort.end())
    {
        mVecViewPort.push_back(viewPort);
    }
}

void Direct2DHost::removeDirect2DViewPort(std::shared_ptr<Direct2DViewPort> viewPort)
{
    auto iter = std::find(mVecViewPort.begin(), mVecViewPort.end(), viewPort);
    if (iter != mVecViewPort.end())
    {
        mVecViewPort.erase(iter);
    }
}

void Direct2DHost::renderOnce()
{
    onRender();
}

ID2D1RenderTarget* Direct2DHost::getRenderTarget()
{
    return pD2dRenderTarget;
}

ID2D1SolidColorBrush* Direct2DHost::getBrush()
{
    return pBrush;
}

void Direct2DHost::onRender()
{
    std::lock_guard<std::mutex> guard(mRenderMutex);
    if (!pD2dRenderTarget)
    {
        return;
    }
    // 开始绘制
    pD2dRenderTarget->BeginDraw();

    // 清空背景色
    pD2dRenderTarget->Clear(mBackgroundColor);
    for (auto iter : mVecViewPort)
    {
        iter->OnRender(pD2dRenderTarget);
    }

    // 结束绘制
    pD2dRenderTarget->EndDraw();

    // Present the back buffer to the screen
    pSwapChain->Present(1, 0);
}