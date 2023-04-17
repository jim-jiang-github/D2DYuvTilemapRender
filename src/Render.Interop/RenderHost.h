#include "IDirect2DHost.h"
#include "RenderViewPort.h"
#include <list> 
using namespace System::Collections::Generic;

public ref class RenderHost
{
private:
    RenderHost(IDirect2DHost* direct2DHost);
public:
    static RenderHost^ Load(System::IntPtr handle, int bgR, int bgG, int bgB)
    {
        HWND hWnd = reinterpret_cast<HWND>(handle.ToPointer());
        IDirect2DHost* direct2DHost = IDirect2DHost::getInstance();
        if (direct2DHost->initialize(hWnd, bgR, bgG, bgB))
        {
            return gcnew RenderHost(direct2DHost);
        }
        else
        {
            direct2DHost->release();
            return nullptr;
        }
    }
    ~RenderHost();
    void Resize(int width, int height);
    void Release();
    void AddRenderViewPort(RenderViewPort^ renderViewPort);
    void RemoveRenderViewPort(RenderViewPort^ renderViewPort);
    void RenderOnce();
private:
    IDirect2DHost* pDirect2DHost = nullptr;
    List<RenderViewPort^>^ _renderViewPortList = gcnew List<RenderViewPort^>();
};