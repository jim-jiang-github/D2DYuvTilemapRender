#include "logging.h"
#include "Direct2DHost.h"
#include <wincodec.h>
#include <any>
#include <comdef.h>

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
    if (!updateOrCreateBitmapRenderTarget())
    {
        LOG_E("updateOrCreateBitmapRenderTarget false");
        return false;
    }

    pDxgiSurfaceRenderTarget->CreateSolidColorBrush(mBackgroundColor, &pBrush);

    return true;
}

void Direct2DHost::resize(int width, int height)
{
    if (mLastWidth != width ||
        mLastHeight != height)
    {
        mLastWidth = width;
        mLastHeight = height;
        mIsNeedResize = true;
    }
}

void Direct2DHost::release()
{
    SafeRelease(pD2dFactory);
    SafeRelease(pWICImagingFactory);
    SafeRelease(pSwapChain);
    SafeRelease(pBitmapRenderTarget);
    SafeRelease(pDxgiSurfaceRenderTarget);
    SafeRelease(pBrush);
    SafeRelease(pYuvRenderBitmap);
    SafeRelease(pMaskRenderBitmap);
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
        for (auto iter : mVecViewPort)
        {
            iter->clearRenderShapeDescriptionCache();
        }

       /* if (!resizeSwapChain())
        {
            LOG_E("resizeSwapChain false");
            return;
        }*/
        if (!updateOrCreateDxgiSurfaceRenderTarget())
        {
            LOG_E("updateOrCreateDxgiSurfaceRenderTarget false");
            return;
        }
        if (!updateOrCreateYuvRenderBitmap())
        {
            LOG_E("updateOrCreateYuvRenderBitmap false");
            return;
        }
        if (!updateOrCreateBitmapRenderTarget())
        {
            LOG_E("updateOrCreateBitmapRenderTarget false");
            return;
        }
    }
    onRender();
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

bool Direct2DHost::resizeSwapChain()
{
    if (!pSwapChain)
    {
        return false;
    }
    HRESULT hr = pSwapChain->ResizeBuffers(
        2,
        mLastWidth,
        mLastHeight,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        0);
    if (FAILED(hr))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        return false;
    }
}

bool Direct2DHost::updateOrCreateDxgiSurfaceRenderTarget()
{
    if (!pSwapChain)
    {
        return false;
    }
    if (!pD2dFactory)
    {
        return false;
    }
    SafeRelease(pDxgiSurfaceRenderTarget);
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
    if (!pDxgiSurfaceRenderTarget)
    {
        return false;
    }
    SafeRelease(pYuvRenderBitmap);
    HRESULT hr = pDxgiSurfaceRenderTarget->CreateBitmap(D2D1::SizeU(mLastWidth, mLastHeight),
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        &pYuvRenderBitmap);
    if (FAILED(hr))
    {
        return false;
    }
    SafeRelease(pMaskRenderBitmap);
    hr = pDxgiSurfaceRenderTarget->CreateBitmap(D2D1::SizeU(mLastWidth, mLastHeight),
        D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
        &pMaskRenderBitmap);
    if (FAILED(hr))
    {
        return false;
    }
    return true;
}

bool Direct2DHost::updateOrCreateBitmapRenderTarget()
{
    if (!pDxgiSurfaceRenderTarget)
    {
        return false;
    }
    SafeRelease(pBitmapRenderTarget);
    HRESULT hr = pDxgiSurfaceRenderTarget->CreateCompatibleRenderTarget(&pBitmapRenderTarget);
    if (FAILED(hr))
    {
        return false;
    }
    hr = pBitmapRenderTarget->GetBitmap(&pMaskRenderBitmap);
    if (FAILED(hr))
    {
        return false;
    }
    auto a = pMaskRenderBitmap->GetSize();
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
    bool bitmapRenderTargetBeginDraw = false;
    for (auto iter : mVecViewPort)
    {
        if (iter->getWidth() == 0 || iter->getHeight() == 0)
        {
            continue;
        }
        auto matrix = D2D1::Matrix3x2F::Translation(iter->getX(), iter->getY());
        pDxgiSurfaceRenderTarget->SetTransform(matrix);
        pBitmapRenderTarget->SetTransform(matrix);

        iter->onRenderYuv(pYuvRenderBitmap, pDxgiSurfaceRenderTarget);
        auto yuvRenderedCallback = iter->getYuvRenderedCallback();
        if (yuvRenderedCallback)
        {
            iter->startRecordRenderShape();
            yuvRenderedCallback();

            auto renderShapeDescriptions = iter->endRecordRenderShape();
            if (renderShapeDescriptions.size() > 0)
            {
                if (!bitmapRenderTargetBeginDraw)
                {
                    bitmapRenderTargetBeginDraw = true;
                    pBitmapRenderTarget->BeginDraw();
                }
                excuteRecordedRenderShapes(renderShapeDescriptions);
            }
        }
        iter->onRenderMask(pMaskRenderBitmap, pDxgiSurfaceRenderTarget);
        pBitmapRenderTarget->SetTransform(mOriginalMatrix);
        pDxgiSurfaceRenderTarget->SetTransform(mOriginalMatrix);
    }
    if (bitmapRenderTargetBeginDraw)
    {
        pBitmapRenderTarget->EndDraw();
    }
     for (auto iter : mVecViewPort)
     {
         auto matrix = D2D1::Matrix3x2F::Translation(iter->getX(), iter->getY());
         pBitmapRenderTarget->SetTransform(matrix);
         iter->onRenderMask(pMaskRenderBitmap, pDxgiSurfaceRenderTarget);
         pBitmapRenderTarget->SetTransform(mOriginalMatrix);
     }
    //pDxgiSurfaceRenderTarget->DrawBitmap(pMaskRenderBitmap);
    pDxgiSurfaceRenderTarget->EndDraw();
    pSwapChain->Present(1, 0);
}

void Direct2DHost::excuteRecordedRenderShapes(std::vector<std::shared_ptr<RenderShapeDescription>> renderShapeDescriptions)
{
    if (!pBitmapRenderTarget)
    {
        return;
    }
    if (!pBrush)
    {
        return;
    }
    for (int i = 0; i < renderShapeDescriptions.size(); i++) {
        auto renderShapeDescription = renderShapeDescriptions[i];
        RenderType renderType = renderShapeDescription->GetRenderType();
        std::vector<std::any> args = renderShapeDescription->GetArguments();
        switch (renderType) {
        case RenderType::DrawRectangle:
        {
            float x = std::any_cast<float>(args[0]);
            float y = std::any_cast<float>(args[1]);
            float w = std::any_cast<float>(args[2]);
            float h = std::any_cast<float>(args[3]);
            float a = std::any_cast<float>(args[4]);
            float r = std::any_cast<float>(args[5]);
            float g = std::any_cast<float>(args[6]);
            float b = std::any_cast<float>(args[7]);
            float strokeWidth = std::any_cast<float>(args[8]);
            pBrush->SetColor({ r, g, b, a });
            pBitmapRenderTarget->DrawRectangle({ x, y, x + w, y + h }, pBrush, strokeWidth);
        }
        break;
        case RenderType::FillRectangle:
        {
            float x = std::any_cast<float>(args[0]);
            float y = std::any_cast<float>(args[1]);
            float w = std::any_cast<float>(args[2]);
            float h = std::any_cast<float>(args[3]);
            float a = std::any_cast<float>(args[4]);
            float r = std::any_cast<float>(args[5]);
            float g = std::any_cast<float>(args[6]);
            float b = std::any_cast<float>(args[7]);
            pBrush->SetColor({ r, g, b, a });
            pBitmapRenderTarget->FillRectangle({ x, y, x + w, y + h }, pBrush);
        }
        break;
        case RenderType::DrawEllipse:
        {
            float centerX = std::any_cast<float>(args[0]);
            float centerY = std::any_cast<float>(args[1]);
            float radiusX = std::any_cast<float>(args[2]);
            float radiusY = std::any_cast<float>(args[3]);
            float a = std::any_cast<float>(args[4]);
            float r = std::any_cast<float>(args[5]);
            float g = std::any_cast<float>(args[6]);
            float b = std::any_cast<float>(args[7]);
            float strokeWidth = std::any_cast<float>(args[8]);
            pBrush->SetColor({ r, g, b, a });
            pBitmapRenderTarget->DrawEllipse({ centerX, centerY, radiusX, radiusY }, pBrush, strokeWidth);
        }
        break;
        case RenderType::FillEllipse:
        {
            float centerX = std::any_cast<float>(args[0]);
            float centerY = std::any_cast<float>(args[1]);
            float radiusX = std::any_cast<float>(args[2]);
            float radiusY = std::any_cast<float>(args[3]);
            float a = std::any_cast<float>(args[4]);
            float r = std::any_cast<float>(args[5]);
            float g = std::any_cast<float>(args[6]);
            float b = std::any_cast<float>(args[7]);
            pBrush->SetColor({ r, g, b, a });
            pBitmapRenderTarget->FillEllipse({ centerX, centerY, radiusX, radiusY }, pBrush);
        }
        break;
        default:
            break;
        }
    }
}