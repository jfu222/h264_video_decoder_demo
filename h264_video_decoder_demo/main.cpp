//
// main.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264VideoDecoder.h"
#include "version.h"


//H264解码器，解码完一帧后的回调函数
int my_output_frame_callback(CH264Picture *outPicture, void *userData, int errorCode)
{
    int ret = 0;

    if (outPicture)
    {
        static int s_PicNumCnt = 0;

        char * outDir = (char *)userData;

        char filename[600] = {0};
        sprintf(filename, "%s/out_%dx%d.%d.bmp", outDir, outPicture->m_picture_frame.PicWidthInSamplesL, 
            outPicture->m_picture_frame.PicHeightInSamplesL, s_PicNumCnt); //不要用outPicture->m_picture_frame.m_PicNumCnt的值，因为对于含有B帧的视频来说，此值是解码顺序，不是帧显示顺序
        
        printf("my_output_frame_callback(): m_PicNumCnt=%d(%s); PicOrderCnt=%d; filename=%s;\n", outPicture->m_picture_frame.m_PicNumCnt, 
            H264_SLIECE_TYPE_TO_STR(outPicture->m_picture_frame.m_h264_slice_header.slice_type), outPicture->m_picture_frame.PicOrderCnt, filename);

        ret = outPicture->m_picture_frame.saveToBmpFile(filename);
        if (ret != 0)
        {
            printf("outPicture->m_picture_frame.saveToBmpFile() failed! %s\n", filename);
            return -1;
        }

        s_PicNumCnt++;

        //if (s_PicNumCnt >= 200) //可以提前结束解码
        //{
            //return -1;
        //}
    }
    else //if (outPicture == NULL) //表示解码结束了
    {
        return -1;
    }

    return 0;
}


//--------------------------------------------
int printHelp(int argc, char *argv[])
{
    printf("h264_video_decoder_demo - version: %s | build date: %s\n\n", VERSION_STR3(VERSION_STR), __DATE__);

    printf("Usage:\n");
    printf("  %s <in|h264_file_path> <outDir|out_bmp_dir>\n", argv[0]);
    printf("For Example:\n");
    printf("  %s ../data/HeavyHand_1080p.B_frames_cabac_tff.h264 ../data/out_bmp_dir\n", argv[0]);
    return 0;
}


int main(int argc, char *argv[])
{
    int ret = 0;
    
    if(argc != 3)
    {
        printHelp(argc, argv);
        return -1;
    }

    char * url = argv[1]; //../data/HeavyHand_1080p.B_frames_cabac_tff.h264
    char * outDir = argv[2]; //./data/out_bmp_dir

    //------------------------
    CH264VideoDecoder vd;
    void *userData = outDir;

    ret= vd.set_output_frame_callback_functuin(my_output_frame_callback, userData); //设置回调函数

    ret = vd.open(url);
    printf("vd.open(): url=%s; ret=%d;\n", url, ret);

    printf("All thing is Over!\n");

    return 0;
}

