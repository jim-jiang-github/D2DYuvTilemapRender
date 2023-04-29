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
            YuvLoader yuvLoader = new YuvLoader(YuvFile.Load("320x180.yuv", 320, 180), 30);
            //YuvLoader yuvLoader = new YuvLoader(YuvFile.Load("480x270.yuv", 480, 270), 30);
            yuvLoader.YuvFrameChanged += (yData, uData, vData, yStride, uStride, vStride, width, height) =>
            {
                OnFrame(yData, uData, vData, yStride, uStride, vStride, width, height);
            };
            yuvLoader.Play();
            //new Thread(() =>
            //{
            //    int width = 1920 / 5;
            //    int height = 1080 / 5;
            //    //int width = 2400 / 5;
            //    //int height = 1600 / 5;
            //    int yStride = width;
            //    int uStride = width / 2;
            //    int vStride = width / 2;

            //    int ySize = yStride * height;
            //    int uSize = uStride * height / 2;
            //    int vSize = vStride * height / 2;

            //    byte[] yuvData = new byte[ySize + uSize + vSize];
            //    while (true)
            //    {
            //        _random.NextBytes(yuvData);
            //        IntPtr yIntPtr = Marshal.AllocHGlobal(yuvData.Length);
            //        Marshal.Copy(yuvData, 0, yIntPtr, yuvData.Length);
            //        OnFrame(yIntPtr, yIntPtr + ySize, yIntPtr + ySize + uSize, yStride, uStride, vStride, width, height);
            //        Marshal.FreeHGlobal(yIntPtr);
            //        Thread.Sleep(33);
            //    }
            //})
            //{ IsBackground = true }.Start();
        }

        public override void OnRendered(RenderGraphics g, float clientWidth, float clientHeight)
        {
            base.OnRendered(g, clientWidth, clientHeight);
            //g.DrawRectangle(0, 0, clientWidth, clientHeight, new DColor32F(1, 0, 0, 0), 2);
            //g.FillRectangle(0, clientHeight - 45, clientWidth / 2, 45, new DColor32F(0.7F, 0, 0, 0));
            //float avatarX = (clientWidth - clientWidth / 3) / 2;
            //float avatarY = (clientHeight - clientWidth / 3) / 2;
            //float avatarW = clientWidth / 3;
            //float avatarH = clientWidth / 3;
            //g.FillEllipse(avatarX, avatarY, avatarW, avatarH, new DColor32F(1, 1, 1, 1));
            //g.FillEllipse(avatarX + 3, avatarY + 3, avatarW - 6, avatarH - 6, new DColor32F(1, 0.4F, 0.7F, 0.8F));
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
            _renderHost = RenderHost.Load(Handle, new DColor24(0, 0, 0));
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

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            if (_renderHost == null)
            {
                return;
            }
            _renderHost.Resize(ClientSize.Width, ClientSize.Height);
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
        }
    }
}
