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
        private YuvFile yuvFile = YuvFile.Load("480x270.yuv", 480, 270);
        public RenderViewPortImpl()
        {
            //YuvLoader yuvLoader = new YuvLoader(YuvFile.Load(720, 640), 30);
            //YuvLoader yuvLoader = new YuvLoader(YuvFile.Load("320x180.yuv", 320, 180), 30);
            YuvLoader yuvLoader = new YuvLoader(yuvFile, 30, _random.Next(0, 30));
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
        private const int VIEWPORT_COUNT = 25;
        private readonly RenderViewPortImpl[] _renderViewPortImpls = new RenderViewPortImpl[VIEWPORT_COUNT];
        public Form1()
        {
            InitializeComponent();
            WindowState = FormWindowState.Maximized;
            //FormBorderStyle = FormBorderStyle.None;
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
                    Thread.Sleep(40);
                }
            })
            { IsBackground = true }.Start();
        }

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
                int w = ClientSize.Width / 5;
                int h = ClientSize.Height / 5;
                int x = i % 5 * w;
                int y = i / 5 * h;
                renderViewPort.SetBounds(x, y, w, h);
            }
        }
    }
}
