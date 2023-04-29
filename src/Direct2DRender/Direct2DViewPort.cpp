#include "logging.h"
#include "Direct2DViewPort.h"
#include "Direct2DHost.h"
#include "libyuv.h"

Direct2DViewPort::Direct2DViewPort(RenderedCallback renderedCallback) :
    mBuffer(5),
    mReadPos(0),
    mWritePos(0)
{
    mRenderedCallback = renderedCallback;
}

Direct2DViewPort::~Direct2DViewPort()
{
}

void Direct2DViewPort::onYuvFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height)
{
    size_t nextWritePos = (mWritePos.load(std::memory_order_acquire) + 1) % mCapacity;

    // If the buffer is full, discard the oldest frame
    if (nextWritePos == mReadPos.load(std::memory_order_acquire)) {
        size_t nextReadPos = (mReadPos.load(std::memory_order_relaxed) + 1) % mCapacity;
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

void Direct2DViewPort::onRender(ID2D1Bitmap* renderBitmap, ID2D1RenderTarget* renderTarget)
{
    YuvFrame frame;
    if (!tryGetNextFrame(frame)) {
        return;
    }
    if (mLastFrameWidth != frame.width || mLastFrameHeight != frame.height)
    {
        mLastFrameWidth = frame.width;
        mLastFrameHeight = frame.height;
        size_t rgbSize = mLastFrameWidth * mLastFrameHeight * 3;
        pRgbFrame = new uint8_t[rgbSize];
    }

    useYuvFrameToUpdateFrameRgb(frame);

    D2D1_RECT_U rcDest = D2D1::RectU(mX, mY, mX + mWidth, mY + mHeight);
    auto hr = renderBitmap->CopyFromMemory(&rcDest, pRgbFrame, frame.width);
    // Fill the rectangle using the mColor variable
    auto brush = Direct2DHost::getInstance()->getBrush();
    if (!renderTarget || !brush)
    {
        return;
    }

     D2D1_RECT_F srcRect = D2D1::RectF(0, 0, 800, 800);
     D2D1_RECT_F destRect = D2D1::RectF(0, 0, 800, 800);
     renderTarget->DrawBitmap(renderBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
     // Call the callback function
    if (mRenderedCallback)
    {
        mRenderedCallback();
    }
}

void Direct2DViewPort::registerRenderedCallback(void (*onRenderedCallback)(ID2D1RenderTarget*))
{
    pOnRenderedCallback = onRenderedCallback;
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

bool Direct2DViewPort::tryGetNextFrame(YuvFrame& frame)
{
    if (mReadPos.load(std::memory_order_acquire) == mWritePos.load(std::memory_order_acquire)) {
        size_t lastReadPos = (mReadPos.load(std::memory_order_relaxed) + mCapacity - 1) % mCapacity;
        if (mBuffer[lastReadPos].data) {
            frame = mBuffer[lastReadPos];
        }
        else
        {
            return false;
        }
    }
    else
    {
        frame = mBuffer[mReadPos.load(std::memory_order_relaxed)];
    }
    size_t nextReadPos = (mReadPos.load(std::memory_order_relaxed) + 1) % mCapacity;
    mReadPos.store(nextReadPos, std::memory_order_release);
    return true;
}

void Direct2DViewPort::useYuvFrameToUpdateFrameRgb(YuvFrame& frame)
{
    uint8_t* dataY = (uint8_t*)frame.data.get();
    uint8_t* dataU = (uint8_t*)frame.data.get() + (frame.yStride * frame.height);
    uint8_t* dataV = dataU + (frame.uStride * (frame.height >> 1));
    libyuv::I420ToRGB24(dataY,
        frame.yStride,
        dataU,
        frame.uStride,
        dataV,
        frame.vStride,
        pRgbFrame,
        frame.width * 3,
        frame.width,
        frame.height);
}