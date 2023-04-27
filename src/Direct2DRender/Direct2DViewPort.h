#pragma once

#include <d2d1.h>
#include <memory>
#include <atomic>
#include <vector>

typedef void (*RenderedCallback)();

struct Frame {
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

class Direct2DViewPort :public std::enable_shared_from_this<Direct2DViewPort>
{
public:
    Direct2DViewPort(RenderedCallback renderedCallback);
    ~Direct2DViewPort();
    void OnFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height);
    void SetBounds(float x, float y, float w, float h);
    void OnRender(ID2D1RenderTarget* renderTarget);
    void RegisterRenderedCallback(void (*onRenderedCallback)(ID2D1RenderTarget*));
private:
    void (*pOnRenderedCallback)(ID2D1RenderTarget*);
    float mX = 0;
    float mY = 0;
    float mW = 0;
    float mH = 0;
    D2D1_COLOR_F mColor = { 0, 0, 0, 1 };
    RenderedCallback mRenderedCallback = nullptr;

    size_t capacity_ = 5;
    std::vector<Frame> buffer_;
    std::atomic<size_t> readPos_;
    std::atomic<size_t> writePos_;
};

