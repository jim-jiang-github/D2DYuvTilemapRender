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
                int width = 1920 / 5;
                int height = 1080 / 5;
                int yStride = width;
                int uStride = width / 2;
                int vStride = width / 2;

                int ySize = yStride * height;
                int uSize = uStride * height / 2;
                int vSize = vStride * height / 2;

                byte[] yuvData = new byte[ySize + uSize + vSize];
                while (true)
                {
                    _random.NextBytes(yuvData);
                    IntPtr yIntPtr = Marshal.AllocHGlobal(yuvData.Length);
                    Marshal.Copy(yuvData, 0, yIntPtr, yuvData.Length);
                    OnFrame(yIntPtr, yIntPtr + ySize, yIntPtr + ySize + uSize, yStride, uStride, vStride, width, height);
                    Marshal.FreeHGlobal(yIntPtr);
                    Thread.Sleep(33);
                }
            })
            { IsBackground = true }.Start();
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
            WindowState = FormWindowState.Maximized;
            FormBorderStyle = FormBorderStyle.None;
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            _renderHost = RenderHost.Load(Handle, 0, 0, 0);
            int count = 25;
            RenderViewPortImpl[] renderViewPortImpls = new RenderViewPortImpl[count];
            for (int i = 0; i < count; i++)
            {
                int w = 2400 / 5;
                int h = 1600 / 5;
                int x = i % 5 * w;
                int y = i / 5 * h;
                RenderViewPortImpl renderViewPort = new RenderViewPortImpl();
                renderViewPort.SetBounds(x, y, w, h);
                _renderHost.AddRenderViewPort(renderViewPort);
                renderViewPortImpls[i] = renderViewPort;
            }
            new Thread(() =>
            {
                while (true)
                {
                    _renderHost.RenderOnce();
                    Thread.Sleep(40);
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
