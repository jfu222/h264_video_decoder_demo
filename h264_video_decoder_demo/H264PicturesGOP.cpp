//
// H264PicturesGOP.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264PicturesGOP.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"
#include <algorithm>


CH264PicturesGOP::CH264PicturesGOP()
{
    int ret = init();
}


CH264PicturesGOP::~CH264PicturesGOP()
{
    int ret = unInit();
}


int CH264PicturesGOP::init()
{
    int ret = 0;

    m_gop_size = 0;
    m_dpb_for_output_length = 0;
    max_num_reorder_frames = 0;
    
    int32_t size_dpb = H264_MAX_DECODED_PICTURE_BUFFER_COUNT;

    for (int i = 0; i < size_dpb; i++)
    {
        m_dpb_for_output[i] = NULL;

        m_DecodedPictureBuffer[i] = new CH264Picture;
        if (m_DecodedPictureBuffer[i] == NULL)
        {
            LOG_ERROR("new CH264Picture failed! m_DecodedPictureBuffer[%d] == NULL\n", i);
            return -1;
        }
    }

    return 0;
}


int CH264PicturesGOP::unInit()
{
    int ret = 0;
    int32_t size_dpb = H264_MAX_DECODED_PICTURE_BUFFER_COUNT;
    
    m_dpb_for_output_length = 0;
    max_num_reorder_frames = 0;

    for (int i = 0; i < size_dpb; i++)
    {
        SAFE_DELETE(m_DecodedPictureBuffer[i]);
    }

    return 0;
}


int CH264PicturesGOP::getOneEmptyPicture(CH264Picture *&pic)
{
    int ret = 0;
    int32_t size_dpb = H264_MAX_DECODED_PICTURE_BUFFER_COUNT;

    for (int i = 0; i < size_dpb; i++)
    {
        if (m_DecodedPictureBuffer[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_UNKNOWN) //重复利用被释放了的参考帧
        {
            pic = m_DecodedPictureBuffer[i];
            RETURN_IF_FAILED(pic == NULL, -1);
            return 0;
        }
    }

    return -1;
}


int CH264PicturesGOP::getOneOutPicture(CH264Picture *newDecodedPic, CH264Picture *&outPic)
{
    int32_t i = 0;
    int32_t index = -1;

    //----------处理P帧情况---------------
    if (max_num_reorder_frames == 0)
    {
        outPic = newDecodedPic; //直接返回，一般为IP编码，即码流中没有B帧的情况
        return 0;
    }
    
    //----------处理B帧情况---------------
    //先找出已经解码完毕，并且还保存在DPB中的POC最小的那一帧
    for (i = 0; i < m_dpb_for_output_length; ++i)
    {
        RETURN_IF_FAILED(m_dpb_for_output[i] == NULL, -1);

        if (i == 0 || m_dpb_for_output[i]->m_picture_frame.PicOrderCnt < m_dpb_for_output[index]->m_picture_frame.PicOrderCnt)
        {
            index = i;
        }
    }

    if (newDecodedPic == NULL) //一般表示解码结束时的flush操作
    {
        if (m_dpb_for_output_length > 0) //说明m_dpb_index_for_output[]数组还残留有未输出的帧
        {
            if (index >= 0)
            {
                outPic = m_dpb_for_output[index]; //输出DPB中的POC最小的那一帧
            }
            else //if (index < 0) //说明缓存中没有帧了
            {
                outPic = NULL;
                m_dpb_for_output_length = 0;
                return 0;
            }

            m_dpb_for_output_length--;

            for (i = index; i < m_dpb_for_output_length; ++i)
            {
                m_dpb_for_output[i] = m_dpb_for_output[i + 1]; //数组元素整体向前移动一个单元
            }
        }
        else
        {
            outPic = NULL; //说明缓存中彻底没有帧了
        }
    }
    else //if (newDecodedPic != NULL) //一般表示解码过程中的push_a_decoded_frame_and_get_a_display_frame操作
    {
        if (m_dpb_for_output_length < max_num_reorder_frames) //说明m_dpb_index_for_output[]数组还没塞满，只push，不get
        {
            m_dpb_for_output[m_dpb_for_output_length] = newDecodedPic;
            m_dpb_for_output_length++;
            outPic = NULL; //目前还没有可输出的帧
        }
        else //if (m_dpb_for_output_length >= max_num_reorder_frames) //说明m_dpb_index_for_output[]数组已满，先get，再push
        {
            RETURN_IF_FAILED(index < 0, -1);

            if (newDecodedPic->m_picture_frame.PicOrderCnt < m_dpb_for_output[index]->m_picture_frame.PicOrderCnt)
            {
                outPic = newDecodedPic; //说明当前新输入的已解码帧的POC是最小的，一般为B帧
            }
            else
            {
                outPic = m_dpb_for_output[index]; //输出DPB中的POC最小的那一帧
                m_dpb_for_output[index] = newDecodedPic; //相当于push a frame
            }
        }
    }

    return 0;
}

