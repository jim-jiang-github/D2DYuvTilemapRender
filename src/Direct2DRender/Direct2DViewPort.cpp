#include "logging.h"
#include "Direct2DViewPort.h"
#include "Direct2DHost.h"
#include "libyuv.h"
#include <algorithm>

Direct2DViewPort::Direct2DViewPort(YuvRenderedCallback yuvRenderedCallback) :
    mBuffer(5),
    mReadPos(0),
    mWritePos(0)
{
    mYuvRenderedCallback = yuvRenderedCallback;
}

Direct2DViewPort::~Direct2DViewPort()
{
}

void Direct2DViewPort::onYuvFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height)
{
    size_t nextWritePos = (mWritePos.load(std::memory_order_acquire) + 1) % mYuvBufferCapacity;

    // If the buffer is full, discard the oldest frame
    if (nextWritePos == mReadPos.load(std::memory_order_acquire)) {
        size_t nextReadPos = (mReadPos.load(std::memory_order_relaxed) + 1) % mYuvBufferCapacity;
        mReadPos.store(nextReadPos, std::memory_order_release);
    }

    size_t uDataOffset = width * height;
    size_t vDataOffset = uDataOffset + (width * height) / 4;
    size_t dataSize = vDataOffset + (width * height) / 4;

    auto& frame = mBuffer[mWritePos.load(std::memory_order_relaxed)];

    if (!frame.data || frame.dataSize != dataSize) {
        frame.data.reset(new unsigned char[dataSize]);
        frame.dataSize = dataSize;
    }

    frame.uDataOffset = uDataOffset;
    frame.vDataOffset = vDataOffset;
    frame.yStride = yStride;
    frame.uStride = uStride;
    frame.vStride = vStride;
    frame.width = width;
    frame.height = height;

    // Copy yData, uData and vData separately
    memcpy(frame.data.get(), yData, uDataOffset);
    memcpy(frame.data.get() + uDataOffset, uData, (vDataOffset - uDataOffset));
    memcpy(frame.data.get() + vDataOffset, vData, (dataSize - vDataOffset));

    mWritePos.store(nextWritePos, std::memory_order_release);
}

void Direct2DViewPort::onRenderYuv(ID2D1Bitmap* yuvRenderBitmap, ID2D1RenderTarget* dxgiSurfaceRenderTarget)
{
    if (!yuvRenderBitmap)
    {
        return;
    }
    if (!dxgiSurfaceRenderTarget)
    {
        return;
    }
    YuvFrame frame;
    bool isFrameChanged = false;
    if (!tryGetNextFrame(frame, isFrameChanged)) {
        return;
    }
    if (isFrameChanged)
    {
        if (mLastFrameWidth != frame.width || mLastFrameHeight != frame.height)
        {
            mLastFrameWidth = frame.width;
            mLastFrameHeight = frame.height;
            size_t rgbSize = mLastFrameWidth * mLastFrameHeight * 4;
            pRgbFrame = new uint8_t[rgbSize];
        }

        useYuvFrameToUpdateFrameRgb(frame);

        D2D1_RECT_U rcDest = D2D1::RectU(mX, mY, mX + mLastFrameWidth, mY + mLastFrameHeight);
        auto hr = yuvRenderBitmap->CopyFromMemory(&rcDest, pRgbFrame, mLastFrameWidth * 4);
    }

    double w = mLastFrameWidth;
    double h = mLastFrameHeight;
    double scaleX = mWidth / w;
    double scaleY = mHeight / h;
    double scale = scaleX < scaleY ? scaleX : scaleY;
    w = w * scale;
    h = h * scale;

    double x = (mWidth - w) / 2;
    double y = (mHeight - h) / 2;

    D2D1_RECT_F srcRect = D2D1::RectF(mX, mY, mX + mLastFrameWidth, mY + mLastFrameHeight);
    D2D1_RECT_F destRect = D2D1::RectF(x, y, w, h);
    dxgiSurfaceRenderTarget->DrawBitmap(yuvRenderBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
}

void Direct2DViewPort::onRenderMask(ID2D1Bitmap* maskRenderBitmap, ID2D1RenderTarget* dxgiSurfaceRenderTarget)
{
    if (!maskRenderBitmap)
    {
        return;
    }
    if (!dxgiSurfaceRenderTarget)
    {
        return;
    }

    double w = mLastFrameWidth;
    double h = mLastFrameHeight;
    double scaleX = mWidth / w;
    double scaleY = mHeight / h;
    double scale = scaleX < scaleY ? scaleX : scaleY;
    w = w * scale;
    h = h * scale;

    double x = (mWidth - w) / 2;
    double y = (mHeight - h) / 2;

    D2D1_RECT_F srcMaskRect = D2D1::RectF(mX, mY, mX + mWidth, mY + mHeight);
    D2D1_RECT_F destMaskRect = D2D1::RectF(0, 0, mWidth, mHeight);
    dxgiSurfaceRenderTarget->DrawBitmap(maskRenderBitmap, destMaskRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcMaskRect);
}

float Direct2DViewPort::getX()
{
    return mX;
}

float Direct2DViewPort::getY()
{
    return mY;
}

float Direct2DViewPort::getWidth()
{
    return mWidth;
}

float Direct2DViewPort::getHeight()
{
    return mHeight;
}

void Direct2DViewPort::setBounds(float x, float y, float w, float h)
{
    // Set the position and size of the control
    mX = x;
    mY = y;
    mWidth = w;
    mHeight = h;
}

YuvRenderedCallback Direct2DViewPort::getYuvRenderedCallback()
{
    return mYuvRenderedCallback;
}

void Direct2DViewPort::clearRenderShapeDescriptionCache()
{
    mLastRenderShapeDescriptionCache.clear();
    mCurrentRenderShapeDescriptionCache.clear();
}

void Direct2DViewPort::startRecordRenderShape()
{
    mCurrentRenderShapeDescriptionCache.clear();
}

void Direct2DViewPort::addRenderShape(std::shared_ptr<RenderShapeDescription> renderShapeDescription)
{
    mCurrentRenderShapeDescriptionCache.push_back(renderShapeDescription);
}

std::vector<std::shared_ptr<RenderShapeDescription>> Direct2DViewPort::endRecordRenderShape()
{
    bool isEqual = true;
    if (mLastRenderShapeDescriptionCache.size() != mCurrentRenderShapeDescriptionCache.size()) {
        isEqual = false;
    }
    else
    {
        for (size_t i = 0; i < mLastRenderShapeDescriptionCache.size(); i++) {
            RenderType lastRenderType = mLastRenderShapeDescriptionCache[i]->GetRenderType();
            RenderType currentRenderType = mCurrentRenderShapeDescriptionCache[i]->GetRenderType();
            std::vector<std::any> lastArguments = mLastRenderShapeDescriptionCache[i]->GetArguments();
            std::vector<std::any> currentArguments = mCurrentRenderShapeDescriptionCache[i]->GetArguments();
            if (lastRenderType != currentRenderType) {
                isEqual = false;
                break;
            }

            if (lastArguments.size() != currentArguments.size()) {
                isEqual = false;
                break;
            }

            for (size_t i = 0; i < lastArguments.size(); i++) {
                if (!Utils::anyEqual(lastArguments[i], currentArguments[i])) {
                    isEqual = false;
                    break;
                }
            }
        }
    }
    if (isEqual)
    {
        return {};
    }
    mLastRenderShapeDescriptionCache = mCurrentRenderShapeDescriptionCache;
    return mLastRenderShapeDescriptionCache;
}

void Direct2DViewPort::Release()
{

}

bool Direct2DViewPort::tryGetNextFrame(YuvFrame& frame, bool& isFrameChanged)
{
    if (mReadPos.load(std::memory_order_acquire) == mWritePos.load(std::memory_order_acquire)) {
        size_t lastReadPos = (mReadPos.load(std::memory_order_relaxed) + mYuvBufferCapacity - 1) % mYuvBufferCapacity;
        if (mBuffer[lastReadPos].data) {
            frame = mBuffer[lastReadPos];
            isFrameChanged = false;
        }
        else
        {
            return false;
        }
    }
    else
    {
        frame = mBuffer[mReadPos.load(std::memory_order_relaxed)];
        isFrameChanged = true;
        size_t nextReadPos = (mReadPos.load(std::memory_order_relaxed) + 1) % mYuvBufferCapacity;
        mReadPos.store(nextReadPos, std::memory_order_release);
    }
    return true;
}

void Direct2DViewPort::useYuvFrameToUpdateFrameRgb(YuvFrame& frame)
{
    uint8_t* dataY = (uint8_t*)frame.data.get();
    uint8_t* dataU = (uint8_t*)frame.data.get() + (frame.yStride * frame.height);
    uint8_t* dataV = dataU + (frame.uStride * (frame.height >> 1));
    libyuv::I420ToARGB(dataY,
        frame.yStride,
        dataU,
        frame.uStride,
        dataV,
        frame.vStride,
        pRgbFrame,
        frame.width * 4,
        frame.width,
        frame.height);
}