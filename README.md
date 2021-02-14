H264 Video Decoder

1. h264_video_decoder_demo工程是按照 T-REC-H.264-201704-S!!PDF-E.pdf 文档中的说明，使用C++实现的H264视频解码器。
   目前第一个版本v1.0.0.1可解码 IP, IBP，CAVLC，CABAC，帧场自适应， 这几种模式组合情况下的H264裸码流

2. SDH264Player工程是相应的Windows下的H264视频播放器

3. 编写此解码器的主要目的是弄清楚H264视频码流是如何解码的。因此目前并不关心解码速度。

## Visual Studio 版本要求
1. 开发时使用的是vs2013版本


## 编译说明
1. 在Windows上，若要编译成命令行二进制可执行程序，请打开h264_video_decoder_demo_vs2013.sln，编译后会生成：
   .\h264_video_decoder_demo\build\x64\Debug\h264_video_decoder_demo_vs2013.exe

2. 在Windows上，若要编译生成视频播放器，请先打开h264_video_decoder_demo_static_vs2013.sln，编译后会生成静态链接库：
   .\h264_video_decoder_demo\build_static\x64\Debug\h264_video_decoder_demo_static_vs2013.lib
   然后再打开SDH264Player.sln，编译后会生成：
   .\h264_video_decoder_demo\SDH264Player\x64\Debug\SDH264Player.exe

## 其他
- blog
