#include "logging.h"
#include "Direct2DHost.h"
#include <wincodec.h>

IDirect2DHost* IDirect2DHost::getInstance()
{
    static Direct2DHost instance;
    return &instance;
}
bool Direct2DHost::initialize(HWND hWnd, float bgR, float bgG, float bgB)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    mLastWidth = rc.right - rc.left;
    mLastHeight = rc.bottom - rc.top;

    mBackgroundColor = { bgR, bgG, bgB, 1 };

    CComPtr<ID3D11Device> d3DDevice;
    HRESULT hr = CreateDeviceAndSwapChain(hWnd, &d3DDevice, &pSwapChain);
    if (FAILED(hr))
    {
        LOG_E("CreateDeviceAndSwapChain hr:{}", hr);
        return false;
    }

    if (!d3DDevice)
    {
        LOG_E("pD3dDevice is null");
        return false;
    }

    CComPtr<IDXGIDevice1> dxgiDevice;
    hr = d3DDevice->QueryInterface(&dxgiDevice);

    if (FAILED(hr))
    {
        LOG_E("pD3dDevice->QueryInterface hr:{}", hr);
        return false;
    }

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pD2dFactory);
    if (FAILED(hr))
    {
        LOG_E("D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory) hr:{}", hr);
        return false;
    }

    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICImagingFactory));
    if (FAILED(hr))
    {
        LOG_E("CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICImagingFactory) hr:{}", hr);
        return false;
    }

    if (!updateOrCreateDxgiSurfaceRenderTarget())
    {
        LOG_E("pD2dRenderTarget create false");
        return false;
    }
    if (!updateOrCreateYuvRenderBitmap())
    {
        LOG_E("pRenderBitmap create false");
        return false;
    }
    if (!updateOrCreateMaskRenderBitmap())
    {
        LOG_E("updateOrCreateMaskRenderBitmap false");
        return false;
    }
    if (!updateOrCreateWicBitmapRenderTarget())
    {
        LOG_E("updateOrCreateWicBitmapRenderTarget false");
        return false;
    }

    pDxgiSurfaceRenderTarget->CreateSolidColorBrush(mBackgroundColor, &pBrush);

    return true;
}

void Direct2DHost::resize(int width, int height)
{
    if (!pSwapChain || !pDxgiSurfaceRenderTarget)
    {
        return;
    }
    if (mLastWidth != width ||
        mLastHeight != height)
    {
        mLastWidth = width;
        mLastHeight = height;
        mIsNeedResize = true;
        renderOnce();
    }
}

void Direct2DHost::release()
{
    if (pD2dFactory)
    {
        pD2dFactory->Release();
        pD2dFactory = nullptr;
    }
    if (pWICImagingFactory)
    {
        pWICImagingFactory->Release();
        pWICImagingFactory = nullptr;
    }
    if (pSwapChain)
    {
        pSwapChain->Release();
        pSwapChain = nullptr;
    }
    if (pWicBitmapRenderTarget)
    {
        pWicBitmapRenderTarget->Release();
        pWicBitmapRenderTarget = nullptr;
    }
    if (pDxgiSurfaceRenderTarget)
    {
        pDxgiSurfaceRenderTarget->Release();
        pDxgiSurfaceRenderTarget = nullptr;
    }
    if (pBrush)
    {
        pBrush->Release();
        pBrush = nullptr;
    }
    if (pYuvRenderBitmap != nullptr)
    {
        pYuvRenderBitmap->Release();
        pYuvRenderBitmap = nullptr;
    }
    if (pMaskRenderBitmap != nullptr)
    {
        pMaskRenderBitmap->Release();
        pMaskRenderBitmap = nullptr;
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
    if (mIsNeedResize)
    {
        mIsNeedResize = false;
        if (!resizeSwapChainBuffers())
        {
            LOG_E("resizeSwapChainBuffers false");
            return;
        }
        if (!updateOrCreateDxgiSurfaceRenderTarget())
        {
            LOG_E("updateOrCreateRenderTarget false");
            return;
        }
        if (!updateOrCreateYuvRenderBitmap())
        {
            LOG_E("updateOrCreateRenderBitmap false");
            return;
        }
        if (!updateOrCreateMaskRenderBitmap())
        {
            LOG_E("updateOrCreateRenderBitmap false");
            return;
        }
        if (!updateOrCreateWicBitmapRenderTarget())
        {
            LOG_E("updateOrCreateRenderTarget false");
            return;
        }
    }
    onRender();
}

ID2D1RenderTarget* Direct2DHost::getWicBitmapRenderTarget()
{
    return pWicBitmapRenderTarget;
}

ID2D1RenderTarget* Direct2DHost::getDxgiSurfaceRenderTarget()
{
    return pDxgiSurfaceRenderTarget;
}

ID2D1SolidColorBrush* Direct2DHost::getBrush()
{
    return pBrush;
}

DXGI_SWAP_CHAIN_DESC Direct2DHost::createSwapChain(HWND hWnd)
{
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
    return swapDesc;
}

HRESULT Direct2DHost::CreateDeviceAndSwapChain(HWND hWnd, ID3D11Device** d3DDevice, IDXGISwapChain** swapChain)
{
    DXGI_SWAP_CHAIN_DESC swapDesc = createSwapChain(hWnd);

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
            *&swapChain,
            *&d3DDevice,
            nullptr,       // do not care about what feature level is chosen
            nullptr        // do not retain D3D device context
        );

        if (SUCCEEDED(hr))
            break;
        else
            LOG_W("hr:{:x}", hr);
    }
    return hr;
}

bool Direct2DHost::resizeSwapChainBuffers()
{
    if (!pSwapChain)
    {
        LOG_E("pSwapChain is null");
        return false;
    }
    HRESULT hr = pSwapChain->ResizeBuffers(
        2,                // Double buffer
        mLastWidth,         // New width
        mLastHeight,        // New height
        DXGI_FORMAT_R8G8B8A8_UNORM, // New format
        0                 // No flags
    );
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool Direct2DHost::updateOrCreateDxgiSurfaceRenderTarget()
{
    if (!pD2dFactory)
    {
        return false;
    }
    if (pDxgiSurfaceRenderTarget)
    {
        // Release the old render target and create a new one
        pDxgiSurfaceRenderTarget->Release();
    }
    CComPtr<IDXGISurface> dxgiSurface;
    HRESULT hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiSurface));
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
    hr = pD2dFactory->CreateDxgiSurfaceRenderTarget(dxgiSurface, &props, &pDxgiSurfaceRenderTarget);
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool Direct2DHost::updateOrCreateYuvRenderBitmap()
{
    if (pYuvRenderBitmap != nullptr)
    {
        pYuvRenderBitmap->Release();
        pYuvRenderBitmap = nullptr;
    }
    HRESULT hr = pDxgiSurfaceRenderTarget->CreateBitmap(D2D1::SizeU(mLastWidth, mLastHeight),
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        &pYuvRenderBitmap);
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool Direct2DHost::updateOrCreateWicBitmapRenderTarget()
{
    if (pWicBitmapRenderTarget)
    {
        // Release the old render target and create a new one
        pWicBitmapRenderTarget->Release();
    }
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED);
    HRESULT hr = pD2dFactory->CreateWicBitmapRenderTarget(pMaskRenderBitmap, props, &pWicBitmapRenderTarget);
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool Direct2DHost::updateOrCreateMaskRenderBitmap()
{
    if (!pWICImagingFactory)
    {
        return false;
    }
    if (pMaskRenderBitmap != nullptr)
    {
        pMaskRenderBitmap->Release();
        pMaskRenderBitmap = nullptr;
    }
    HRESULT hr = pWICImagingFactory->CreateBitmap(mLastWidth, mLastHeight, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &pMaskRenderBitmap);

    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

void Direct2DHost::onRender()
{
    if (!pDxgiSurfaceRenderTarget)
    {
        return;
    }
    pDxgiSurfaceRenderTarget->BeginDraw();
    pDxgiSurfaceRenderTarget->Clear(mBackgroundColor);

    for (auto iter : mVecViewPort)
    {
        //offset render position.
        D2D1_MATRIX_3X2_F originalMatrix;
        pDxgiSurfaceRenderTarget->GetTransform(&originalMatrix);
        auto matrix = D2D1::Matrix3x2F::Translation(iter->getX(), iter->getY());
        pDxgiSurfaceRenderTarget->SetTransform(matrix);

        iter->onRender(pYuvRenderBitmap, pDxgiSurfaceRenderTarget);

        pDxgiSurfaceRenderTarget->SetTransform(originalMatrix);
    }
    //pDxgiSurfaceRenderTarget->DrawBitmap(pYuvRenderBitmap);

    pDxgiSurfaceRenderTarget->EndDraw();
    pSwapChain->Present(1, 0);
}