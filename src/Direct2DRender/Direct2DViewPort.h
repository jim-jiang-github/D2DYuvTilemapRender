#pragma once

#include <d2d1.h>
#include <memory>
#include <atomic>
#include <vector>
#include "RenderShapeDescription.h"
#include "Utils.h"

typedef void (*YuvRenderedCallback)();

class Direct2DViewPort :public std::enable_shared_from_this<Direct2DViewPort>
{
public:
    Direct2DViewPort(YuvRenderedCallback yuvRenderedCallback);
    ~Direct2DViewPort();
    void onYuvFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height);
    void onRenderYuv(ID2D1Bitmap* yuvRenderBitmap, ID2D1RenderTarget* dxgiSurfaceRenderTarget);
    void onRenderMask(ID2D1Bitmap* maskRenderBitmap, ID2D1RenderTarget* dxgiSurfaceRenderTarget);
    float getX();
    float getY();
    float getWidth();
    float getHeight();
    void setBounds(float x, float y, float w, float h);

    YuvRenderedCallback getYuvRenderedCallback();
    void clearRenderShapeDescriptionCache();
    void startRecordRenderShape();
    void addRenderShape(std::shared_ptr<RenderShapeDescription> renderShapeDescription);
    std::vector<std::shared_ptr<RenderShapeDescription>> endRecordRenderShape();

    void Release();
private:
    struct YuvFrame {
        std::shared_ptr<unsigned char[]> data;
        size_t dataSize;
        size_t uDataOffset;
        size_t vDataOffset;
        int yStride;
        int uStride;
        int vStride;
        int width;
        int height;
    };
    bool tryGetNextFrame(YuvFrame& frame, bool& isFrameChanged);
    void useYuvFrameToUpdateFrameRgb(YuvFrame& frame);
private:
    float mX = 0;
    float mY = 0;
    float mWidth = 0;
    float mHeight = 0;
    size_t mYuvBufferCapacity = 5;
    std::vector<YuvFrame> mBuffer;
    std::atomic<size_t> mReadPos;
    std::atomic<size_t> mWritePos;
    int mLastFrameWidth = 0;
    int mLastFrameHeight = 0;
    YuvRenderedCallback mYuvRenderedCallback = nullptr;
    uint8_t* pRgbFrame;
    std::vector<std::shared_ptr<RenderShapeDescription>> mLastRenderShapeDescriptionCache;
    std::vector<std::shared_ptr<RenderShapeDescription>> mCurrentRenderShapeDescriptionCache;
};

