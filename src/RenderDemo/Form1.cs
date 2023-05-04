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
        private const int INPUT_FPS = 30;

        //Also can use yuv file
        //private YuvFile _yuvFile = YuvFile.Load("480x270.yuv", 480, 270);
        private YuvFile _yuvFile = YuvFile.Load(480, 270);

        public RenderViewPortImpl()
        {
            var startIndex = _random.Next(0, 30);
            YuvLoader yuvLoader = new YuvLoader(_yuvFile, INPUT_FPS, startIndex);
            yuvLoader.YuvFrameChanged += (yData, uData, vData, yStride, uStride, vStride, width, height) =>
            {
                OnFrame(yData, uData, vData, yStride, uStride, vStride, width, height);
            };
            yuvLoader.Play();
        }

        public override void OnRendered(RenderGraphics g, float clientWidth, float clientHeight)
        {
            base.OnRendered(g, clientWidth, clientHeight);
            g.DrawRectangle(0, 0, clientWidth, clientHeight, new DColor32F(1, 0, 0, 0), 2);
            g.FillRectangle(0, clientHeight - 45, clientWidth / 2, 45, new DColor32F(0.7F, 0, 0, 0));
            float avatarCenterX = clientWidth / 2;
            float avatarCenterY = clientHeight / 2;
            float avatarRadiusX = clientWidth / 3 / 2;
            float avatarRadiusY = clientWidth / 3 / 2;
            g.FillEllipse(avatarCenterX, avatarCenterY, avatarRadiusX, avatarRadiusY, new DColor32F(1, 1, 1, 1));
            g.FillEllipse(avatarCenterX, avatarCenterY, avatarRadiusX - 6, avatarRadiusY - 6, new DColor32F(1, 0.4F, 0.7F, 0.8F));
        }
    }
    public partial class Form1 : Form
    {
        private RenderHost _renderHost;
        private const int OUTPUT_FPS = 25;
        private const int VIEWPORT_COUNT = 25;
        private readonly RenderViewPortImpl[] _renderViewPortImpls = new RenderViewPortImpl[VIEWPORT_COUNT];
        public Form1()
        {
            InitializeComponent();
            WindowState = FormWindowState.Maximized;
            FormBorderStyle = FormBorderStyle.None;
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            _renderHost = RenderHost.Load(Handle, new DColor24F(50F / 255F, 50F / 255F, 50F / 255F));
            for (int i = 0; i < _renderViewPortImpls.Length; i++)
            {
                RenderViewPortImpl renderViewPort = new RenderViewPortImpl();
                _renderHost.AddRenderViewPort(renderViewPort);
                _renderViewPortImpls[i] = renderViewPort;
            }
            ResizeHost();
            new Thread(() =>
            {
                while (true)
                {
                    _renderHost.RenderOnce();
                    Thread.Sleep(1000 / OUTPUT_FPS);
                }
            })
            { IsBackground = true }.Start();
            //new Thread(() =>
            //{
            //    int x = 0;
            //    while (true)
            //    {
            //        if (_renderViewPortImpls.Length > 0)
            //        {
            //            var renderViewPort = _renderViewPortImpls[0];

            //            var l = renderViewPort.Left;
            //            var t = renderViewPort.Top;
            //            var r = renderViewPort.Right;
            //            var b = renderViewPort.Bottom;

            //            //renderViewPort.SetBounds((l + x++) % ClientSize.Width, t, r, b);
            //            renderViewPort.SetBounds(200, t, r, b);
            //        }
            //        Thread.Sleep(1000000);
            //    }
            //})
            //{ IsBackground = true }.Start();
        }

        //protected override void OnMouseDown(MouseEventArgs e)
        //{
        //    base.OnMouseDown(e);
        //    var renderViewPort = _renderViewPortImpls[0];

        //    var l = renderViewPort.Left;
        //    var t = renderViewPort.Top;
        //    var r = renderViewPort.Right;
        //    var b = renderViewPort.Bottom;

        //    //renderViewPort.SetBounds((l + x++) % ClientSize.Width, t, r, b);
        //    renderViewPort.SetBounds(200, t, r, b);
        //}

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            ResizeHost();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);
        }

        private void ResizeHost()
        {
            if (_renderHost == null)
            {
                return;
            }
            _renderHost.Resize(ClientSize.Width, ClientSize.Height);
            for (int i = 0; i < _renderViewPortImpls.Length; i++)
            {
                var renderViewPort = _renderViewPortImpls[i];
                var splitCount = (int)Math.Sqrt(VIEWPORT_COUNT);
                int w = ClientSize.Width / splitCount;
                int h = ClientSize.Height / splitCount;
                int x = i % splitCount * w;
                int y = i / splitCount * h;
                renderViewPort.SetBounds(x, y, w, h);
            }
        }
    }
}
