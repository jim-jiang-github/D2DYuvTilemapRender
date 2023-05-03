#include "IDirect2DHost.h"
#include "RenderViewPort.h"
#include "DColor.h"
#include <list> 
using namespace System::Collections::Generic;

public ref class RenderHost
{
private:
    RenderHost(IDirect2DHost* direct2DHost);
public:
    static RenderHost^ Load(System::IntPtr handle, DColor24F bgColor)
    {
        HWND hWnd = reinterpret_cast<HWND>(handle.ToPointer());
        IDirect2DHost* direct2DHost = IDirect2DHost::getInstance();
        if (direct2DHost->initialize(hWnd, bgColor.R, bgColor.G, bgColor.B))
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