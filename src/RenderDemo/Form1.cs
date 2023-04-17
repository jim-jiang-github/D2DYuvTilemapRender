using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RenderDemo
{
    public class RenderViewPortImpl : RenderViewPort
    {
        public override void OnRendered(RenderGraphics g, float clientWidth, float clientHeight)
        {
            base.OnRendered(g, clientWidth, clientHeight);
            g.DrawRectangle(0, 0, 50, 50, 1, 0.5F, 0.5F, 0.5F, 2);
        }
    }
    public partial class Form1 : Form
    {
        private System.Threading.Timer _timer;
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
            _timer = new System.Threading.Timer(TimerCallback, null, 0, 40);
        }

        private void TimerCallback(object state)
        {
            _renderHost.RenderOnce();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            _timer.Dispose();
            base.OnClosing(e);
        }
    }
}
