//
// H264Picture.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264Picture.h"


CH264Picture::CH264Picture()
{
    int ret = init();
}


CH264Picture::~CH264Picture()
{
    int ret = m_picture_frame.unInit();
}


int CH264Picture::init()
{
    m_current_picture_ptr = NULL;
    m_picture_previous_ref = NULL;
    m_picture_previous = NULL;
    m_picture_coded_type = H264_PICTURE_CODED_TYPE_UNKNOWN;
    m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
    
    TopFieldOrderCnt = 0;
    BottomFieldOrderCnt = 0;
    PicOrderCntMsb = 0;
    PicOrderCntLsb = 0;
    FrameNumOffset = 0;
    absFrameNum = 0;
    picOrderCntCycleCnt = 0;
    frameNumInPicOrderCntCycle = 0;
    expectedPicOrderCnt = 0;
    PicOrderCnt = 0;
    PicNum= 0;
    LongTermPicNum= 0;
    reference_marked_type = H264_PICTURE_MARKED_AS_unkown;
    m_is_decode_finished = 0;
    m_is_in_use = 0; //闲置状态

    return 0;
}


int CH264Picture::reset()
{
//    m_current_picture_ptr = NULL;
//    m_picture_previous_ref = NULL;
//    m_picture_previous = NULL;
    m_picture_coded_type = H264_PICTURE_CODED_TYPE_UNKNOWN;
    m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
    
    TopFieldOrderCnt = 0;
    BottomFieldOrderCnt = 0;
    PicOrderCntMsb = 0;
    PicOrderCntLsb = 0;
    FrameNumOffset = 0;
    absFrameNum = 0;
    picOrderCntCycleCnt = 0;
    frameNumInPicOrderCntCycle = 0;
    expectedPicOrderCnt = 0;
    PicOrderCnt = 0;
    PicNum= 0;
    LongTermPicNum= 0;
    reference_marked_type = H264_PICTURE_MARKED_AS_unkown;
    m_is_decode_finished = 0;
    m_is_in_use = 1; //正在使用状态

    return 0;
}


CH264Picture & CH264Picture::operator = (const CH264Picture &src)
{
    int ret = 0;

    ret = copyPicture(src);

    return *this;
}


int CH264Picture::copyPicture(const CH264Picture &src)
{
    int ret = 0;

    this->m_current_picture_ptr = src.m_current_picture_ptr;
    this->m_picture_previous_ref = src.m_picture_previous_ref;
    this->m_picture_previous = src.m_picture_previous;
    this->m_picture_coded_type = src.m_picture_coded_type;
    this->m_picture_coded_type_marked_as_refrence = src.m_picture_coded_type_marked_as_refrence;
    
    this->TopFieldOrderCnt = src.TopFieldOrderCnt;
    this->BottomFieldOrderCnt = src.BottomFieldOrderCnt;
    this->PicOrderCntMsb = src.PicOrderCntMsb;
    this->PicOrderCntLsb = src.PicOrderCntLsb;
    this->FrameNumOffset = src.FrameNumOffset;
    this->absFrameNum = src.absFrameNum;
    this->picOrderCntCycleCnt = src.picOrderCntCycleCnt;
    this->frameNumInPicOrderCntCycle = src.frameNumInPicOrderCntCycle;
    this->expectedPicOrderCnt = src.expectedPicOrderCnt;
    this->PicOrderCnt = src.PicOrderCnt;
    this->PicNum = src.PicNum;
    this->LongTermPicNum = src.LongTermPicNum;
    this->reference_marked_type = src.reference_marked_type;
    this->m_is_decode_finished = src.m_is_decode_finished;
    this->m_is_in_use = src.m_is_in_use;

    m_picture_frame = src.m_picture_frame;
    m_picture_top_filed = src.m_picture_top_filed;
    m_picture_bottom_filed = src.m_picture_bottom_filed;
    
    return ret;
}


int CH264Picture::decode_one_slice(CH264SliceHeader &slice_header, CBitstream &bs, CH264Picture *(&dpb)[16])
{
    int ret = 0;
    int32_t size_pdb = 16;
    int32_t i = 0;
    
    //----------------帧----------------------------------
    m_picture_coded_type = H264_PICTURE_CODED_TYPE_FRAME;
    m_picture_frame.m_picture_coded_type = H264_PICTURE_CODED_TYPE_FRAME;
    m_picture_frame.m_parent = this;

    memcpy(m_picture_frame.m_dpb, dpb, sizeof(CH264Picture *) * size_pdb);

    m_current_picture_ptr = &m_picture_frame;

    ret = m_picture_frame.init(slice_header);
    RETURN_IF_FAILED(ret != 0, ret);

    //----------------顶场-------------------------------
    m_picture_top_filed.m_picture_coded_type = H264_PICTURE_CODED_TYPE_TOP_FIELD;
    m_picture_top_filed.m_parent = this;

    ret = m_picture_top_filed.init(slice_header);
    RETURN_IF_FAILED(ret != 0, ret);

    //----------------底场-------------------------------
    m_picture_bottom_filed.m_picture_coded_type = H264_PICTURE_CODED_TYPE_BOTTOM_FIELD;
    m_picture_bottom_filed.m_parent = this;

    ret = m_picture_bottom_filed.init(slice_header);
    RETURN_IF_FAILED(ret != 0, ret);
    
    //--------------------------------------------
    if (slice_header.field_pic_flag == 0) //帧
    {
        ret = m_picture_frame.m_h264_slice_data.slice_data(bs, m_picture_frame, slice_header.slice_id);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else //if (slice_header.field_pic_flag == 1) //场
    {
        m_picture_coded_type = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
        if (slice_header.bottom_field_flag == 0) //顶场
        {
            m_current_picture_ptr = &m_picture_top_filed;

            ret = m_picture_top_filed.m_h264_slice_data.slice_data(bs, m_picture_top_filed, slice_header.slice_id);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else //if (slice_header.bottom_field_flag == 1) //底场
        {
            m_current_picture_ptr = &m_picture_bottom_filed;

            ret = m_picture_bottom_filed.m_h264_slice_data.slice_data(bs, m_picture_bottom_filed, slice_header.slice_id);
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }

    m_is_in_use = 1; //正在使用状态

    return 0;
}

