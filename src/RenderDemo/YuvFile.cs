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
            int frameCount = 30;
            int frameSize = frameWidth * frameHeight * 3 / 2;
            long bufferLength = frameCount * frameSize;
            byte[] yuvData = new byte[bufferLength];
            Random random = new Random();
            random.NextBytes(yuvData);
            return new YuvFile(yuvData, frameWidth, frameHeight);
        }

        public static YuvFile Load(string yuvPath, int frameWidth, int frameHeight)
        {
            if (File.Exists(yuvPath))
            {
                try
                {
                    using var fileStream = new FileStream(yuvPath, FileMode.Open);
                    using var binaryReader = new BinaryReader(fileStream);
                    return new YuvFile(binaryReader.ReadBytes((int)fileStream.Length), frameWidth, frameHeight);
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

        private YuvFile(byte[] yuvData, int frameWidth, int frameHeight)
        {
            FrameWidth = frameWidth;
            FrameHeight = frameHeight;
            int frameSize = FrameWidth * FrameHeight * 3 / 2;
            _backBuffer = Marshal.AllocCoTaskMem(yuvData.Length);
            FrameCount = yuvData.Length / frameSize;
            Marshal.Copy(yuvData, 0, _backBuffer, yuvData.Length);
        }

        public IntPtr GetFrame(int index)
        {
            var frameSize = FrameWidth * FrameHeight * 3 / 2;
            IntPtr frameBuffer = _backBuffer + index * frameSize;
            return frameBuffer;
        }
    }
}
