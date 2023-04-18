using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RenderDemo
{
    public class RenderViewPortImpl : RenderViewPort
    {
        private readonly Random _random = new Random();
        public RenderViewPortImpl()
        {
            new Thread(() =>
            {
                int width = 1080;
                int height = 720;
                int yStride = width;
                int uStride = width / 2;
                int vStride = width / 2;

                int ySize = yStride * height;
                int uSize = uStride * height / 2;
                int vSize = vStride * height / 2;

                byte[] yData = new byte[ySize];
                byte[] uData = new byte[uSize];
                byte[] vData = new byte[vSize];
                while (true)
                {
                    PushOnFrame(yData, uData, vData, yStride, uStride, vStride, width, height);
                    Thread.Sleep(33);
                }
            })
            { IsBackground = true }.Start();
        }

        private void PushOnFrame(byte[] yData, byte[] uData, byte[] vData, int yStride, int uStride, int vStride, int width, int height)
        {
            _random.NextBytes(yData);
            _random.NextBytes(uData);
            _random.NextBytes(vData);

            IntPtr yIntPtr = Marshal.AllocHGlobal(yData.Length);
            IntPtr uIntPtr = Marshal.AllocHGlobal(uData.Length);
            IntPtr vIntPtr = Marshal.AllocHGlobal(vData.Length);

            Marshal.Copy(yData, 0, yIntPtr, yData.Length);
            Marshal.Copy(uData, 0, uIntPtr, uData.Length);
            Marshal.Copy(vData, 0, vIntPtr, vData.Length);

            OnFrame(yIntPtr, uIntPtr, vIntPtr, yStride, uStride, vStride, width, height);

            Marshal.FreeHGlobal(yIntPtr);
            Marshal.FreeHGlobal(uIntPtr);
            Marshal.FreeHGlobal(vIntPtr);
        }

        public override void OnRendered(RenderGraphics g, float clientWidth, float clientHeight)
        {
            base.OnRendered(g, clientWidth, clientHeight);
            g.DrawRectangle(0, 0, 50, 50, 1, 0.5F, 0.5F, 0.5F, 2);
        }
    }
    public partial class Form1 : Form
    {
        private RenderHost _renderHost;
        public Form1()
        {
            InitializeComponent();
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            RenderViewPortImpl renderViewPort = new RenderViewPortImpl();
            renderViewPort.SetBounds(0, 0, 100, 100);
            _renderHost = RenderHost.Load(Handle, 0, 0, 0);
            _renderHost.AddRenderViewPort(renderViewPort);
            new Thread(() =>
            {
                while (true)
                {
                    _renderHost.RenderOnce();
                    Thread.Sleep(33);
                }
            })
            { IsBackground = true }.Start();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
        }
    }
}
