using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace RenderDemo
{
    public class YuvLoader
    {
        public delegate void YuvFrameDelegate(IntPtr yData, IntPtr uData, IntPtr vData, int yStride, int uStride, int vStride, int width, int height);

        public event YuvFrameDelegate YuvFrameChanged;

        private YuvFile _videoFile;
        private readonly int _fps = 25;
        private bool _isPlaying = false;
        private Thread _thread;
        private int _startIndex = 0;

        public YuvLoader(YuvFile videoFile, int fps = 25, int startIndex = 0)
        {
            _videoFile = videoFile;
            _fps = fps;
            _startIndex = startIndex;
        }

        public void Play()
        {
            _isPlaying = true;
            int interval = (int)(1000F / _fps);
            int yStride = _videoFile.FrameWidth;
            int uStride = _videoFile.FrameWidth / 2;
            int vStride = _videoFile.FrameWidth / 2;

            int ySize = _videoFile.FrameWidth * _videoFile.FrameHeight;
            int uSize = _videoFile.FrameWidth * _videoFile.FrameHeight / 4;
            _thread = new Thread(() =>
            {
                for (; _startIndex < _videoFile.FrameCount; _startIndex++)
                {
                    if (_isPlaying)
                    {
                        IntPtr yuvDta = _videoFile.GetFrame(_startIndex);
                        IntPtr y = yuvDta;
                        IntPtr u = y + ySize;
                        IntPtr v = u + uSize;
                        YuvFrameChanged?.Invoke(y, u, v, yStride, uStride, vStride, _videoFile.FrameWidth, _videoFile.FrameHeight);
                        Thread.Sleep(interval);
                        if (_startIndex == _videoFile.FrameCount - 1)
                        {
                            _startIndex = 0;
                        }
                    }
                }
            })
            { IsBackground = true };
            _thread.Start();
        }

        public void Stop()
        {
            _isPlaying = false;
        }
    }
}
