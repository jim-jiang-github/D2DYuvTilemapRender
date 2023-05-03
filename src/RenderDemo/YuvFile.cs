using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace RenderDemo
{
    public class YuvFile
    {
        //Random byte to mock yuv data.
        public static YuvFile Load(int frameWidth, int frameHeight)
        {
            return new YuvFile(frameWidth, frameHeight);
        }

        public static YuvFile Load(string yuvPath, int frameWidth, int frameHeight)
        {
            if (File.Exists(yuvPath))
            {
                try
                {
                    return new YuvFile(yuvPath, frameWidth, frameHeight);
                }
                catch
                {
                    return null;
                }
            }
            else
            {
                return null;
            }
        }

        private readonly IntPtr _backBuffer;

        public int FrameWidth { get; private set; }
        public int FrameHeight { get; private set; }
        public int FrameCount { get; private set; }

        private YuvFile(string yuvPath, int frameWidth, int frameHeight)
        {
            FrameWidth = frameWidth;
            FrameHeight = frameHeight;
            int _frameSize = FrameWidth * FrameHeight * 3 / 2;
            using var fileStream = new FileStream(yuvPath, FileMode.Open);
            using var binaryReader = new BinaryReader(fileStream);
            _backBuffer = Marshal.AllocCoTaskMem((int)fileStream.Length);
            FrameCount = (int)(fileStream.Length / _frameSize);
            Marshal.Copy(binaryReader.ReadBytes((int)fileStream.Length), 0, _backBuffer, (int)fileStream.Length);
        }

        private YuvFile(int frameWidth, int frameHeight)
        {
            FrameWidth = frameWidth;
            FrameHeight = frameHeight;
            FrameCount = 30;
            int _frameSize = FrameWidth * FrameHeight * 3 / 2;
            long bufferLength = FrameCount * _frameSize;
            byte[] yuvData = new byte[bufferLength];
            Random _random = new Random();
            _random.NextBytes(yuvData);
            _backBuffer = Marshal.AllocCoTaskMem((int)bufferLength);
            Marshal.Copy(yuvData, 0, _backBuffer, (int)bufferLength);
        }

        public IntPtr GetFrame(int index)
        {
            var frameSize = FrameWidth * FrameHeight * 3 / 2;
            IntPtr frameBuffer = _backBuffer + index * frameSize;
            return frameBuffer;
        }
    }
}
