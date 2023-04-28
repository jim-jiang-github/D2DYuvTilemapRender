#include "pch.h"
#include "Direct2DViewPort.h"
#include "Direct2DHost.h"
#include "libyuv.h"
#include <random>

Direct2DViewPort::Direct2DViewPort(RenderedCallback renderedCallback) :
    buffer_(5),
    readPos_(0),
    writePos_(0)
{
    mRenderedCallback = renderedCallback;
}

Direct2DViewPort::~Direct2DViewPort()
{
}

void Direct2DViewPort::OnFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height)
{
    size_t nextWritePos = (writePos_.load(std::memory_order_acquire) + 1) % capacity_;

    // If the buffer is full, discard the oldest frame
    if (nextWritePos == readPos_.load(std::memory_order_acquire)) {
        size_t nextReadPos = (readPos_.load(std::memory_order_relaxed) + 1) % capacity_;
        readPos_.store(nextReadPos, std::memory_order_release);
    }

    size_t uDataOffset = width * height;
    size_t vDataOffset = uDataOffset + (width * height) / 4;
    size_t dataSize = vDataOffset + (width * height) / 4;

    auto& frame = buffer_[writePos_.load(std::memory_order_relaxed)];

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

    writePos_.store(nextWritePos, std::memory_order_release);
}

void Direct2DViewPort::SetBounds(float x, float y, float w, float h)
{
    // Set the position and size of the control
    mX = x;
    mY = y;
    mW = x + w;
    mH = y + h;
}

void Direct2DViewPort::OnRender(ID2D1RenderTarget* renderTarget)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    int r = dis(gen);
    int g = dis(gen);
    int b = dis(gen);
    mColor = { r / (float)255, g / (float)255, b / (float)255, 255 };
    try {
        Frame frame;
        if (readPos_.load(std::memory_order_acquire) == writePos_.load(std::memory_order_acquire)) {
            size_t lastReadPos = (readPos_.load(std::memory_order_relaxed) + capacity_ - 1) % capacity_;
            if (buffer_[lastReadPos].data) {
                frame = buffer_[lastReadPos];
            }
            else
            {
                return;
            }
        }
        else
        {
            frame = buffer_[readPos_.load(std::memory_order_relaxed)];
        }
        size_t nextReadPos = (readPos_.load(std::memory_order_relaxed) + 1) % capacity_;
        readPos_.store(nextReadPos, std::memory_order_release);

        uint8_t* dataY = (uint8_t*)frame.data.get();
        uint8_t* dataU = (uint8_t*)frame.data.get() + (frame.yStride * frame.height);
        uint8_t* dataV = dataU + (frame.uStride * (frame.height >> 1));
        size_t argb_size = frame.width * frame.height * 4;
        uint8_t* datargb = new uint8_t[argb_size];
        libyuv::I420ToARGB(dataY, frame.yStride, dataU, frame.uStride, dataV, frame.vStride, datargb, frame.width * 4, frame.width, frame.height);
        delete[] datargb;
        // Fill the rectangle using the mColor variable
        auto brush = Direct2DHost::getInstance()->getBrush();
        if (!renderTarget || !brush)
        {
            return;
        }
        D2D1_MATRIX_3X2_F originalMatrix;
        renderTarget->GetTransform(&originalMatrix);
        // Set the translation matrix
        auto matrix = D2D1::Matrix3x2F::Translation(mX, mY);
        renderTarget->SetTransform(matrix);
        // Draw the black border
        D2D1_COLOR_F color = { 0, 0, 0, 1 };
        brush->SetColor(color);
        renderTarget->FillRectangle({ 0, 0, mW - mX,  mH - mY }, brush);
        brush->SetColor(mColor);
        renderTarget->FillRectangle({ 2, 2, mW - mX - 4,  mH - mY - 4 }, brush);
        // Call the callback function
        if (mRenderedCallback)
        {
            mRenderedCallback();
        }
        // Save the original transformation matrix
        renderTarget->SetTransform(originalMatrix);
    }
    catch (std::exception& e) {
    }
}

void Direct2DViewPort::RegisterRenderedCallback(void (*onRenderedCallback)(ID2D1RenderTarget*))
{
    pOnRenderedCallback = onRenderedCallback;
}