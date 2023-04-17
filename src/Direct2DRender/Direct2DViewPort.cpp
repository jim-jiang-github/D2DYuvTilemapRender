#include "pch.h"
#include "Direct2DViewPort.h"
#include "Direct2DHost.h"
#ifdef _DEBUG
#include <random>
#endif

Direct2DViewPort::Direct2DViewPort(RenderedCallback renderedCallback)
{
    mRenderedCallback = renderedCallback;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    int r = dis(gen);
    int g = dis(gen);
    int b = dis(gen);
    mColor = { r / (float)255, g / (float)255, b / (float)255, 255 };
}

Direct2DViewPort::~Direct2DViewPort()
{
}

void Direct2DViewPort::OnFrame(unsigned char* yData, unsigned char* uData, unsigned char* vData, int yStride, int uStride, int vStride, int width, int height)
{

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
    try {
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