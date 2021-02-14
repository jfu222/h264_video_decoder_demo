//
// H264RefPicList.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264PictureBase.h"
#include "H264PicturesGOP.h"
#include "CommonFunction.h"


//--------------参考帧列表重排序------------------------
//8.2.1 Decoding process for picture order count (only needed to be invoked for one slice of a picture)
int CH264PictureBase::Decoding_process_for_picture_order_count()
{
    int ret = 0;

    if (m_h264_slice_header.m_sps.pic_order_cnt_type == 0)
    {
        ret = Decoding_process_for_picture_order_count_type_0(m_parent->m_picture_previous_ref);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else if (m_h264_slice_header.m_sps.pic_order_cnt_type == 1)
    {
        ret = Decoding_process_for_picture_order_count_type_1(m_parent->m_picture_previous);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    else if (m_h264_slice_header.m_sps.pic_order_cnt_type == 2)
    {
        ret = Decoding_process_for_picture_order_count_type_2(m_parent->m_picture_previous);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    int32_t PicOrderCntTemp = PicOrderCntFunc(this); //设置this->PicOrderCnt字段值

    return ret;
}


//8.2.1.1 Decoding process for picture order count type 0
//This process is invoked when pic_order_cnt_type is equal to 0.
int CH264PictureBase::Decoding_process_for_picture_order_count_type_0(const CH264PictureBase *picture_previous_ref)
{
    int32_t prevPicOrderCntMsb = 0;
    int32_t prevPicOrderCntLsb = 0;

    if (m_h264_slice_header.m_nal_unit.IdrPicFlag == 1) //IDR picture
    {
        prevPicOrderCntMsb = 0;
        prevPicOrderCntLsb = 0;
    }
    else //if (m_h264_slice_header.m_nal_unit.IdrPicFlag != 1)
    {
        RETURN_IF_FAILED(picture_previous_ref == NULL, -1);

        if (picture_previous_ref->memory_management_control_operation_5_flag == 1) //If the previous reference picture in decoding order included a memory_management_control_operation equal to 5
        {
            if (picture_previous_ref->m_picture_coded_type != H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //If the previous reference picture in decoding order is not a bottom field
            {
                prevPicOrderCntMsb = 0;
                prevPicOrderCntLsb = picture_previous_ref->TopFieldOrderCnt;
            }
            else //if (picture_previous_ref.m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //the previous reference picture in decoding order is a bottom field
            {
                prevPicOrderCntMsb = 0;
                prevPicOrderCntLsb = 0;
            }
        }
        else //the previous reference picture in decoding order did not include a memory_management_control_operation equal to 5
        {
            prevPicOrderCntMsb = picture_previous_ref->PicOrderCntMsb;
            prevPicOrderCntLsb = picture_previous_ref->m_h264_slice_header.pic_order_cnt_lsb;
        }
    }

    //--------------------------
    if ( (m_h264_slice_header.pic_order_cnt_lsb < prevPicOrderCntLsb) 
            && ((prevPicOrderCntLsb - m_h264_slice_header.pic_order_cnt_lsb) >= (m_h264_slice_header.m_sps.MaxPicOrderCntLsb / 2)))
    {
        PicOrderCntMsb = prevPicOrderCntMsb + m_h264_slice_header.m_sps.MaxPicOrderCntLsb;
    }
    else if ((m_h264_slice_header.pic_order_cnt_lsb > prevPicOrderCntLsb) 
            && ((m_h264_slice_header.pic_order_cnt_lsb - prevPicOrderCntLsb ) > (m_h264_slice_header.m_sps.MaxPicOrderCntLsb / 2)))
    {
        PicOrderCntMsb = prevPicOrderCntMsb - m_h264_slice_header.m_sps.MaxPicOrderCntLsb;
    }
    else
    {
        PicOrderCntMsb = prevPicOrderCntMsb;
    }
    
    //--------------------------
    if (m_picture_coded_type != H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //When the current picture is not a bottom field 当前图像为非底场
    {
        TopFieldOrderCnt = PicOrderCntMsb + m_h264_slice_header.pic_order_cnt_lsb;
    }
    
    if (m_picture_coded_type != H264_PICTURE_CODED_TYPE_TOP_FIELD) //When the current picture is not a top field
    {
        if (!m_h264_slice_header.field_pic_flag) //当前图像为帧
        {
            BottomFieldOrderCnt = TopFieldOrderCnt + m_h264_slice_header.delta_pic_order_cnt_bottom;
        }
        else //当前图像为底场
        {
            BottomFieldOrderCnt = PicOrderCntMsb + m_h264_slice_header.pic_order_cnt_lsb;
        }
    }

    return 0;
}


//8.2.1.2 Decoding process for picture order count type 1
//This process is invoked when pic_order_cnt_type is equal to 1.
int CH264PictureBase::Decoding_process_for_picture_order_count_type_1(const CH264PictureBase *picture_previous)
{
    RETURN_IF_FAILED(m_h264_slice_header.m_nal_unit.IdrPicFlag != 1 && picture_previous == NULL, -1);

    int ret = 0;
    int32_t prevFrameNumOffset = 0;
    
    //--------------prevFrameNumOffset----------------
    if (m_h264_slice_header.m_nal_unit.IdrPicFlag != 1) //not IDR picture
    {
        if (picture_previous->memory_management_control_operation_5_flag == 1) //If the previous picture in decoding order included a memory_management_control_operation equal to 5
        {
            prevFrameNumOffset = 0;
        }
        else
        {
            prevFrameNumOffset = picture_previous->FrameNumOffset;
        }
    }

    //--------------FrameNumOffset----------------
    if (m_h264_slice_header.m_nal_unit.IdrPicFlag == 1) //IDR图像
    {
        FrameNumOffset = 0;
    }
    else if (picture_previous->m_h264_slice_header.frame_num > m_h264_slice_header.frame_num) //前一图像的帧号比当前图像大
    {
        FrameNumOffset = prevFrameNumOffset + m_h264_slice_header.m_sps.MaxFrameNum;
    }
    else
    {
        FrameNumOffset = prevFrameNumOffset;
    }
    
    //--------------absFrameNum----------------
    if (m_h264_slice_header.m_sps.num_ref_frames_in_pic_order_cnt_cycle != 0)
    {
        absFrameNum = FrameNumOffset + m_h264_slice_header.frame_num;
    }
    else
    {
        absFrameNum = 0;
    }

    if (m_h264_slice_header.m_nal_unit.nal_ref_idc == 0 && absFrameNum > 0)
    {
        absFrameNum = absFrameNum - 1;
    }

    if (absFrameNum > 0)
    {
        picOrderCntCycleCnt = ( absFrameNum - 1 ) / m_h264_slice_header.m_sps.num_ref_frames_in_pic_order_cnt_cycle;
        frameNumInPicOrderCntCycle = ( absFrameNum - 1 ) % m_h264_slice_header.m_sps.num_ref_frames_in_pic_order_cnt_cycle;
    }
    
    //--------------expectedPicOrderCnt----------------
    if (absFrameNum > 0)
    {
        expectedPicOrderCnt = picOrderCntCycleCnt * m_h264_slice_header.m_sps.ExpectedDeltaPerPicOrderCntCycle;
        for (int i = 0; i <= frameNumInPicOrderCntCycle; i++)
        {
            expectedPicOrderCnt = expectedPicOrderCnt + m_h264_slice_header.m_sps.offset_for_ref_frame[i];
        }
    }
    else
    {
        expectedPicOrderCnt = 0;
    }

    if (m_h264_slice_header.m_nal_unit.nal_ref_idc == 0)
    {
        expectedPicOrderCnt = expectedPicOrderCnt + m_h264_slice_header.m_sps.offset_for_non_ref_pic;
    }

    //--------------TopFieldOrderCnt or BottomFieldOrderCnt----------------
    if (!m_h264_slice_header.field_pic_flag) //当前图像为帧
    {
        TopFieldOrderCnt = expectedPicOrderCnt + m_h264_slice_header.delta_pic_order_cnt[0];
        BottomFieldOrderCnt = TopFieldOrderCnt + m_h264_slice_header.m_sps.offset_for_top_to_bottom_field + m_h264_slice_header.delta_pic_order_cnt[1];
    }
    else if (!m_h264_slice_header.bottom_field_flag) //当前图像为顶场
    {
        TopFieldOrderCnt = expectedPicOrderCnt + m_h264_slice_header.delta_pic_order_cnt[0];
    }
    else //当前图像为底场
    {
        BottomFieldOrderCnt = expectedPicOrderCnt + m_h264_slice_header.m_sps.offset_for_top_to_bottom_field + m_h264_slice_header.delta_pic_order_cnt[0];
    }
    
    return 0;
}


//8.2.1.3 Decoding process for picture order count type 2
//This process is invoked when pic_order_cnt_type is equal to 2.
int CH264PictureBase::Decoding_process_for_picture_order_count_type_2(const CH264PictureBase *picture_previous)
{
    RETURN_IF_FAILED(m_h264_slice_header.m_nal_unit.IdrPicFlag != 1 && picture_previous == NULL, -1);

    int ret = 0;
    int32_t prevFrameNumOffset = 0;
    
    //--------------prevFrameNumOffset----------------
    if (m_h264_slice_header.m_nal_unit.IdrPicFlag != 1) //not IDR picture
    {
        if (picture_previous->memory_management_control_operation_5_flag == 1) //If the previous picture in decoding order included a memory_management_control_operation equal to 5
        {
            prevFrameNumOffset = 0;
        }
        else
        {
            prevFrameNumOffset = picture_previous->FrameNumOffset;
        }
    }

    //--------------FrameNumOffset----------------
    if (m_h264_slice_header.m_nal_unit.IdrPicFlag == 1)
    {
        FrameNumOffset = 0;
    }
    else if (picture_previous->m_h264_slice_header.frame_num > m_h264_slice_header.frame_num)
    {
        FrameNumOffset = prevFrameNumOffset + m_h264_slice_header.m_sps.MaxFrameNum;
    }
    else
    {
        FrameNumOffset = prevFrameNumOffset;
    }
    
    //--------------tempPicOrderCnt----------------
    int32_t tempPicOrderCnt = 0;

    if (m_h264_slice_header.m_nal_unit.IdrPicFlag == 1)
    {
        tempPicOrderCnt = 0;
    }
    else if (m_h264_slice_header.m_nal_unit.nal_ref_idc == 0)//当前图像为非参考图像
    {
        tempPicOrderCnt = 2 * (FrameNumOffset + m_h264_slice_header.frame_num) - 1;
    }
    else
    {
        tempPicOrderCnt = 2 * (FrameNumOffset + m_h264_slice_header.frame_num);
    }
    
    //--------------TopFieldOrderCnt or BottomFieldOrderCnt----------------
    if (!m_h264_slice_header.field_pic_flag) //当前图像为帧
    {
        TopFieldOrderCnt = tempPicOrderCnt;
        BottomFieldOrderCnt = tempPicOrderCnt;
    }
    else if (m_h264_slice_header.bottom_field_flag) //当前图像为底场
    {
        BottomFieldOrderCnt = tempPicOrderCnt;
    }
    else //当前图像为顶场
    {
        TopFieldOrderCnt = tempPicOrderCnt;
    }
    
    return 0;
}


//8.2.4 Decoding process for reference picture lists construction
//This process is invoked at the beginning of the decoding process for each P, SP, or B slice.
int CH264PictureBase::Decoding_process_for_reference_picture_lists_construction(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], CH264Picture *(&RefPicList1)[16])
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //Decoded reference pictures are marked as "used for short-term reference" or "used for long-term reference" 
    //as specified by the bitstream and specified in clause 8.2.5.
    //Short-term reference pictures are identified by the value of frame_num.
    //Long-term reference pictures are assigned a long-term frame index as specified by the bitstream and specified in clause 8.2.5.

    //8.2.5 Decoded reference picture marking process
//    ret = Decoded_reference_picture_marking_process(dpb);
//    RETURN_IF_FAILED(ret != 0 , ret);
    
    //8.2.4.1 Decoding process for picture numbers
    ret = Decoding_process_for_picture_numbers(dpb);
    RETURN_IF_FAILED(ret != 0 , ret);
    
    //At the beginning of the decoding process for each slice, reference picture list RefPicList0, and for B slices RefPicList1, 
    //are derived as specified by the following ordered steps:

    //1. An initial reference picture list RefPicList0 and for B slices RefPicList1 are derived as specified in clause 8.2.4.2
    //8.2.4.2 Initialisation process for reference picture lists
    ret = Initialisation_process_for_reference_picture_lists(dpb, RefPicList0, RefPicList1);
    RETURN_IF_FAILED(ret != 0 , ret);

    //2. When ref_pic_list_modification_flag_l0 is equal to 1 or, when decoding a B slice, ref_pic_list_modification_flag_l1 is 
    //equal to 1, the initial reference picture list RefPicList0 and, for B slices, RefPicList1 are modified as specified in clause 8.2.4.3
    //8.2.4.3 Modification process for reference picture lists 参考图像列表的重排序过程
    ret = Modification_process_for_reference_picture_lists(RefPicList0, RefPicList1);
    RETURN_IF_FAILED(ret != 0 , ret);

    //The number of entries in the modified reference picture list RefPicList0 is num_ref_idx_l0_active_minus1 + 1, and for B slices the 
    //number of entries in the modified reference picture list RefPicList1 is num_ref_idx_l1_active_minus1 + 1. A reference picture may 
    //appear at more than one index in the modified reference picture lists RefPicList0 or RefPicList1.
    this->m_RefPicList0Length = slice_header.num_ref_idx_l0_active_minus1 + 1;
    this->m_RefPicList1Length = slice_header.num_ref_idx_l1_active_minus1 + 1;

    return 0;
}


//8.2.4.1 Decoding process for picture numbers
int CH264PictureBase::Decoding_process_for_picture_numbers(CH264Picture *(&dpb)[16])
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;
    int32_t size_dpb = 16;
    int32_t i = 0;

    this->FrameNum = slice_header.frame_num;

    //To each short-term reference picture the variables FrameNum and FrameNumWrap are assigned as follows. First, 
    //FrameNum is set equal to the syntax element frame_num that has been decoded in the slice header(s) of the 
    //corresponding short-term reference picture. Then the variable FrameNumWrap is derived as
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
        {
            if (dpb[i]->m_picture_frame.FrameNum > slice_header.frame_num)
            {
                dpb[i]->m_picture_frame.FrameNumWrap = dpb[i]->m_picture_frame.FrameNum - dpb[i]->m_picture_frame.m_h264_slice_header.m_sps.MaxFrameNum;
            }
            else
            {
                dpb[i]->m_picture_frame.FrameNumWrap = dpb[i]->m_picture_frame.FrameNum;
            }
        }
        
        if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
        {
            if (dpb[i]->m_picture_top_filed.FrameNum > slice_header.frame_num)
            {
                dpb[i]->m_picture_top_filed.FrameNumWrap = dpb[i]->m_picture_top_filed.FrameNum - dpb[i]->m_picture_top_filed.m_h264_slice_header.m_sps.MaxFrameNum;
            }
            else
            {
                dpb[i]->m_picture_top_filed.FrameNumWrap = dpb[i]->m_picture_top_filed.FrameNum;
            }
        }
        
        if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
        {
            if (dpb[i]->m_picture_bottom_filed.FrameNum > slice_header.frame_num)
            {
                dpb[i]->m_picture_bottom_filed.FrameNumWrap = dpb[i]->m_picture_bottom_filed.FrameNum - dpb[i]->m_picture_bottom_filed.m_h264_slice_header.m_sps.MaxFrameNum;
            }
            else
            {
                dpb[i]->m_picture_bottom_filed.FrameNumWrap = dpb[i]->m_picture_bottom_filed.FrameNum;
            }
        }
    }

    //Each long-term reference picture has an associated value of LongTermFrameIdx (that was assigned to it as specified in clause 8.2.5).
    //8.2.5 Decoded reference picture marking process
    //ret = Decoded_reference_picture_marking_process(dpb);
    //RETURN_IF_FAILED(ret != 0, ret);

    //--------------PicNum and LongTermPicNum----------------
    if (slice_header.field_pic_flag == 0) //参考帧或互补参考场对
    {
        for (i = 0; i < size_dpb; i++)
        {
            if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //For each short-term reference frame or complementary reference field pair:
            {
                dpb[i]->m_picture_frame.PicNum = dpb[i]->m_picture_frame.FrameNumWrap;
                dpb[i]->PicNum = dpb[i]->m_picture_frame.PicNum;
            }
            
            if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                && dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
               )
            {
                dpb[i]->m_picture_top_filed.PicNum = dpb[i]->m_picture_top_filed.FrameNumWrap;
                dpb[i]->m_picture_bottom_filed.PicNum = dpb[i]->m_picture_bottom_filed.FrameNumWrap;
                dpb[i]->PicNum = dpb[i]->m_picture_top_filed.PicNum;
                RETURN_IF_FAILED(dpb[i]->m_picture_top_filed.PicNum != dpb[i]->m_picture_bottom_filed.PicNum, -1);
            }
            
            if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //For each long-term reference frame or long-term complementary reference field pair
            {
                dpb[i]->m_picture_frame.LongTermPicNum = dpb[i]->m_picture_frame.LongTermFrameIdx;
                dpb[i]->LongTermPicNum = dpb[i]->m_picture_frame.LongTermPicNum;
            }
            
            if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
               && dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
               )
            {
                dpb[i]->m_picture_top_filed.LongTermPicNum = dpb[i]->m_picture_top_filed.LongTermFrameIdx;
                dpb[i]->m_picture_bottom_filed.LongTermPicNum = dpb[i]->m_picture_bottom_filed.LongTermFrameIdx;
                dpb[i]->LongTermPicNum = dpb[i]->m_picture_top_filed.LongTermPicNum;
                RETURN_IF_FAILED(dpb[i]->m_picture_top_filed.LongTermPicNum != dpb[i]->m_picture_bottom_filed.LongTermPicNum, -1);
            }
        }
    }
    else //if (slice_header.field_pic_flag == 1) //参考场
    {
        for (i = 0; i < size_dpb; i++)
        {
            if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
            {
                if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) //If the reference field has the same parity as the current field 参考场和当前场奇偶性相同
                {
                    dpb[i]->m_picture_top_filed.PicNum = 2 * dpb[i]->m_picture_top_filed.FrameNumWrap + 1;
                }
                else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //the reference field has the opposite parity of the current field
                {
                    dpb[i]->m_picture_top_filed.PicNum = 2 * dpb[i]->m_picture_top_filed.FrameNumWrap;
                }
                dpb[i]->PicNum = dpb[i]->m_picture_top_filed.PicNum;
            }

            if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
            {
                if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) //If the reference field has the same parity as the current field 参考场和当前场奇偶性相同
                {
                    dpb[i]->m_picture_bottom_filed.PicNum = 2 * dpb[i]->m_picture_bottom_filed.FrameNumWrap + 1;
                }
                else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //the reference field has the opposite parity of the current field
                {
                    dpb[i]->m_picture_bottom_filed.PicNum = 2 * dpb[i]->m_picture_bottom_filed.FrameNumWrap;
                }
                dpb[i]->PicNum = dpb[i]->m_picture_bottom_filed.PicNum;
            }
            
            if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference)
            {
                if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) //If the reference field has the same parity as the current field 参考场和当前场奇偶性相同
                {
                    dpb[i]->m_picture_top_filed.LongTermPicNum = 2 * dpb[i]->m_picture_top_filed.LongTermFrameIdx + 1;
                }
                else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //the reference field has the opposite parity of the current field
                {
                    dpb[i]->m_picture_top_filed.LongTermPicNum = 2 * dpb[i]->m_picture_top_filed.LongTermFrameIdx;
                }
                dpb[i]->LongTermPicNum = dpb[i]->m_picture_top_filed.LongTermPicNum;
            }
            
            if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference)
            {
                if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) //If the reference field has the same parity as the current field 参考场和当前场奇偶性相同
                {
                    dpb[i]->m_picture_bottom_filed.LongTermPicNum = 2 * dpb[i]->m_picture_bottom_filed.LongTermFrameIdx + 1;
                }
                else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //the reference field has the opposite parity of the current field
                {
                    dpb[i]->m_picture_bottom_filed.LongTermPicNum = 2 * dpb[i]->m_picture_bottom_filed.LongTermFrameIdx;
                }
                dpb[i]->LongTermPicNum = dpb[i]->m_picture_bottom_filed.LongTermPicNum;
            }
        }
    }
    
    return 0;
}


//8.2.4.2 Initialisation process for reference picture lists
//This initialisation process is invoked when decoding a P, SP, or B slice header.
int CH264PictureBase::Initialisation_process_for_reference_picture_lists(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], CH264Picture *(&RefPicList1)[16])
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    int32_t i = 0;
    
    //RefPicList0 and RefPicList1 have initial entries as specified in clauses 8.2.4.2.1 through 8.2.4.2.5
    if (slice_header.slice_type % 5 == H264_SLIECE_TYPE_P || slice_header.slice_type % 5 == H264_SLIECE_TYPE_SP)
    {
        if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
        {
            ret = Initialisation_process_for_the_reference_picture_list_for_P_and_SP_slices_in_frames(dpb, RefPicList0, m_RefPicList0Length);
            RETURN_IF_FAILED(ret != 0 , ret);
        }
        else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD || m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
        {
            ret = Initialisation_process_for_the_reference_picture_list_for_P_and_SP_slices_in_fields(dpb, RefPicList0, m_RefPicList0Length);
            RETURN_IF_FAILED(ret != 0 , ret);
        }
    }
    else if (slice_header.slice_type % 5 == H264_SLIECE_TYPE_B)
    {
        if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
        {
            ret = Initialisation_process_for_reference_picture_lists_for_B_slices_in_frames(dpb, RefPicList0, RefPicList1, m_RefPicList0Length, m_RefPicList1Length);
            RETURN_IF_FAILED(ret != 0 , ret);
        }
        else if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD || m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
        {
            ret = Initialisation_process_for_reference_picture_lists_for_B_slices_in_fields(dpb, RefPicList0, RefPicList1, m_RefPicList0Length, m_RefPicList1Length);
            RETURN_IF_FAILED(ret != 0 , ret);
        }
    }

    //When the number of entries in the initial RefPicList0 or RefPicList1 produced as specified in clauses 8.2.4.2.1 through 8.2.4.2.5 
    //is greater than num_ref_idx_l0_active_minus1 + 1 or num_ref_idx_l1_active_minus1 + 1, respectively, the extra entries past position 
    //num_ref_idx_l0_active_minus1 or num_ref_idx_l1_active_minus1 are discarded from the initial reference picture list.
    if (m_RefPicList0Length > slice_header.num_ref_idx_l0_active_minus1 + 1)
    {
        for (i = slice_header.num_ref_idx_l0_active_minus1 + 1; i < m_RefPicList0Length; i++)
        {
            RefPicList0[i] = NULL;
        }
        m_RefPicList0Length = slice_header.num_ref_idx_l0_active_minus1 + 1;
    }

    if (m_RefPicList1Length > slice_header.num_ref_idx_l1_active_minus1 + 1)
    {
        for (i = slice_header.num_ref_idx_l1_active_minus1 + 1; i < m_RefPicList1Length; i++)
        {
            RefPicList1[i] = NULL;
        }
        m_RefPicList1Length = slice_header.num_ref_idx_l1_active_minus1 + 1;
    }

    //When the number of entries in the initial RefPicList0 or RefPicList1 produced as specified in clauses 8.2.4.2.1 through 8.2.4.2.5 
    //is less than num_ref_idx_l0_active_minus1 + 1 or num_ref_idx_l1_active_minus1 + 1, respectively, the remaining entries in the initial 
    //reference picture list are set equal to "no reference picture".
    if (m_RefPicList0Length < slice_header.num_ref_idx_l0_active_minus1 + 1)
    {
        for (i = m_RefPicList0Length; i < slice_header.num_ref_idx_l0_active_minus1 + 1; i++)
        {
            RefPicList0[i] = NULL; //FIXME: set equal to "no reference picture".
        }
    }
    
    if (m_RefPicList1Length < slice_header.num_ref_idx_l1_active_minus1 + 1)
    {
        for (i = m_RefPicList1Length; i < slice_header.num_ref_idx_l1_active_minus1 + 1; i++)
        {
            RefPicList1[i] = NULL; //FIXME: set equal to "no reference picture".
        }
    }

    return 0;
}


//8.2.4.2.1 Initialisation process for the reference picture list for P and SP slices in frames
//This initialisation process is invoked when decoding a P or SP slice in a coded frame.
int CH264PictureBase::Initialisation_process_for_the_reference_picture_list_for_P_and_SP_slices_in_frames(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], int32_t &RefPicList0Length)
{
    int ret = 0;
    int32_t size_dpb = 16;
    int32_t i = 0;
    int32_t j = 0;
    int32_t short_refs_count = 0;
    int32_t long_refs_count = 0;
    int32_t not_refs_count = 0;

    int32_t indexTemp_short[16] = {0};
    int32_t indexTemp_long[16] = {0};
    int32_t indexTemp3[16] = {0};

    CH264SliceHeader & slice_header = m_h264_slice_header;

    //1. 先把所有的短期参考帧放到数组的前半部分，把所有的长期参考帧放到数组的后半部分
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //reference frame or complementary reference field pair
        {
            indexTemp_short[short_refs_count] = i;
            short_refs_count++;
        }
        else if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //reference frame or complementary reference field pair
        {
            indexTemp_long[long_refs_count] = i;
            long_refs_count++;
        }
        else
        {
            indexTemp3[not_refs_count] = i;
            not_refs_count++;
        }
    }
    
    //there shall be at least one reference frame or complementary reference field pair that is currently marked as "used for reference"
    RETURN_IF_FAILED(short_refs_count + long_refs_count == 0, -1);

    //2. 按帧号降序排列短期参考帧(冒泡排序)
    for (i = 0; i < short_refs_count - 1; i++)
    {
        for (j = 0; j < short_refs_count - i - 1; j++)
        {
            if (dpb[indexTemp_short[j]]->m_picture_frame.PicNum < dpb[indexTemp_short[j + 1]]->m_picture_frame.PicNum) //降序排列
            {
                int32_t temp = indexTemp_short[j];
                indexTemp_short[j] = indexTemp_short[j + 1];
                indexTemp_short[j + 1] = temp;
            }
        }
    }
    
    //3. 按帧号升序排列长期参考帧(冒泡排序)
    for (i = 0; i < long_refs_count - 1; i++)
    {
        for (j = 0; j < long_refs_count - i - 1; j++)
        {
            if (dpb[indexTemp_long[j]]->m_picture_frame.LongTermPicNum > dpb[indexTemp_long[j + 1]]->m_picture_frame.LongTermPicNum) //升序排列
            {
                int32_t temp = indexTemp_long[j];
                indexTemp_long[j] = indexTemp_long[j + 1];
                indexTemp_long[j + 1] = temp;
            }
        }
    }
    
    //4. 生成排序后的参考序列
    j = 0;

    for (i = 0; i < short_refs_count; i++)
    {
        RefPicList0[j++] = dpb[ indexTemp_short[i] ];
    }
    
    for (i = 0; i < long_refs_count; i++)
    {
        RefPicList0[j++] = dpb[ indexTemp_long[i] ];
    }

    RefPicList0Length = j;

    //------------------------------------
    if (slice_header.MbaffFrameFlag)
    {
        for (i = 0; i < RefPicList0Length; ++i)
        {
            CH264Picture *& ref_list_frame = RefPicList0[i];
            CH264Picture *& ref_list_top_filed = RefPicList0[16 + 2 * i];
            CH264Picture *& ref_list_bottom_filed = RefPicList0[16 + 2 * i + 1];

            ref_list_top_filed = ref_list_frame;
            ref_list_bottom_filed = ref_list_frame;
        }
    }

    return 0;
}


//8.2.4.2.2 Initialisation process for the reference picture list for P and SP slices in fields
//This initialisation process is invoked when decoding a P or SP slice in a coded field.
int CH264PictureBase::Initialisation_process_for_the_reference_picture_list_for_P_and_SP_slices_in_fields(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], int32_t &RefPicList0Length)
{
    int ret = 0;
    int32_t size_dpb = 16;
    int32_t i = 0;
    int32_t j = 0;
    int32_t short_refs_count = 0;
    int32_t long_refs_count = 0;
    int32_t not_refs_count = 0;
    int32_t indexTemp_short[16] = {0};
    int32_t indexTemp_long[16] = {0};

    CH264Picture * refFrameList0ShortTerm[16] = {NULL};
    CH264Picture * refFrameList0LongTerm[16] = {NULL};

    CH264Picture * curPic = m_parent;

    //1. 先把所有的短期参考场放到数组的前半部分，把所有的长期参考场放到数组的后半部分
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
            || dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
           )
        {
            refFrameList0ShortTerm[short_refs_count] = dpb[i];
            short_refs_count++;
        }
        else if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                || dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
            )
        {
            refFrameList0LongTerm[long_refs_count] = dpb[i];
            long_refs_count++;
        }
        else
        {
            not_refs_count++;
        }
    }
    
    if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD) //When the current field is the second field (in decoding order) of a complementary reference field pair
    {
        if (curPic->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //and the first field is marked as "used for short-term reference"
        {
            refFrameList0ShortTerm[short_refs_count] = curPic; //the first field is included in the list of short-term reference frames refFrameList0ShortTerm
            short_refs_count++;
        }
        else if (curPic->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //and the first field is marked as "used for long-term reference
        {
            refFrameList0LongTerm[long_refs_count] = curPic; //the first field is included in the list of long-term reference frames refFrameList0LongTerm
            long_refs_count++;
        }
    }

    //there shall be at least one reference field (which can be a field of a reference frame) that is currently marked as "used for reference"
    RETURN_IF_FAILED(short_refs_count == 0 && long_refs_count == 0, -1);

    //2. 按帧号降序排列短期参考场(冒泡排序)
    for (i = 0; i < short_refs_count - 1; i++)
    {
        for (j = 0; j < short_refs_count - i - 1; j++)
        {
            if (refFrameList0ShortTerm[j]->m_picture_top_filed.FrameNumWrap < refFrameList0ShortTerm[j + 1]->m_picture_top_filed.FrameNumWrap
                || refFrameList0ShortTerm[j]->m_picture_bottom_filed.FrameNumWrap < refFrameList0ShortTerm[j + 1]->m_picture_bottom_filed.FrameNumWrap
                )//降序排列
            {
                CH264Picture * temp = refFrameList0ShortTerm[j];
                refFrameList0ShortTerm[j] = refFrameList0ShortTerm[j + 1];
                refFrameList0ShortTerm[j + 1] = temp;
            }
        }
    }
    
    //3. 按帧号升序排列长期参考场(冒泡排序)
    for (i = 0; i < long_refs_count - 1; i++)
    {
        for (j = 0; j < long_refs_count - i - 1; j++)
        {
            if (refFrameList0LongTerm[j]->m_picture_top_filed.LongTermFrameIdx > refFrameList0LongTerm[j + 1]->m_picture_top_filed.LongTermFrameIdx
                || refFrameList0LongTerm[j]->m_picture_top_filed.LongTermFrameIdx > refFrameList0LongTerm[j + 1]->m_picture_top_filed.LongTermFrameIdx
                ) //升序排列
            {
                CH264Picture * temp = refFrameList0LongTerm[j];
                refFrameList0LongTerm[j] = refFrameList0LongTerm[j + 1];
                refFrameList0LongTerm[j + 1] = temp;
            }
        }
    }

    //-------------------------
    //8.2.4.2.5 Initialisation process for reference picture lists in fields
    ret = Initialisation_process_for_reference_picture_lists_in_fields(refFrameList0ShortTerm, refFrameList0LongTerm, RefPicList0, RefPicList0Length, 0);
    RETURN_IF_FAILED(ret != 0, -1);
    
    return 0;
}


//8.2.4.2.3 Initialisation process for reference picture lists for B slices in frames
//This initialisation process is invoked when decoding a B slice in a coded frame.
int CH264PictureBase::Initialisation_process_for_reference_picture_lists_for_B_slices_in_frames(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], CH264Picture *(&RefPicList1)[16],
        int32_t &RefPicList0Length, int32_t &RefPicList1Length)
{
    int ret = 0;
    int32_t size_dpb = 16;
    int32_t i = 0;
    int32_t j = 0;
    int32_t short_refs_count_left = 0;
    int32_t short_refs_count_right = 0;
    int32_t long_refs_count = 0;
    int32_t not_refs_count = 0;

    int32_t indexTemp_short_left[16] = {0};
    int32_t indexTemp_short_right[16] = {0};
    int32_t indexTemp_long[16] = {0};
    int32_t indexTemp3[16] = {0};
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    //For B slices, the order of short-term reference entries in the reference picture lists RefPicList0 and RefPicList1 
    //depends on output order, as given by PicOrderCnt( ).

    //FIXME: When pic_order_cnt_type is equal to 0, reference pictures that are marked as "non-existing" as specified 
    //       in clause 8.2.5.2 are not included in either RefPicList0 or RefPicList1.

    //-------------RefPicList0----------------
    //1. 先把所有的短期参考帧放到数组的前半部分，把所有的长期参考帧放到数组的后半部分
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //reference frame or complementary reference field pair
        {
            if (dpb[i]->m_picture_frame.PicOrderCnt < PicOrderCnt)
            {
                indexTemp_short_left[short_refs_count_left] = i;
                short_refs_count_left++;
            }
            else //if (RefPicList0[i].m_picture_frame.PicOrderCnt >= PicOrderCnt)
            {
                indexTemp_short_right[short_refs_count_right] = i;
                short_refs_count_right++;
            }
        }
        else if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //reference frame or complementary reference field pair
        {
            indexTemp_long[long_refs_count] = i;
            long_refs_count++;
        }
        else
        {
            indexTemp3[not_refs_count] = i;
            not_refs_count++;
        }
    }
    
    //there shall be at least one reference frame or complementary reference field pair that is currently marked as "used for reference"
    RETURN_IF_FAILED(short_refs_count_left + short_refs_count_right + long_refs_count == 0, -1);

    //2. 按帧号(PicOrderCnt)按顺序排列短期参考帧(冒泡排序)
    for (i = 0; i < short_refs_count_left - 1; i++)
    {
        for (j = 0; j < short_refs_count_left - i - 1; j++)
        {
            if (dpb[indexTemp_short_left[j]]->m_picture_frame.PicOrderCnt < dpb[indexTemp_short_left[j + 1]]->m_picture_frame.PicOrderCnt) //降序排列
            {
                int32_t temp = indexTemp_short_left[j];
                indexTemp_short_left[j] = indexTemp_short_left[j + 1];
                indexTemp_short_left[j + 1] = temp;
            }
        }
    }
    
    for (i = 0; i < short_refs_count_right - 1; i++)
    {
        for (j = 0; j < short_refs_count_right - i - 1; j++)
        {
            if (dpb[indexTemp_short_right[j]]->m_picture_frame.PicOrderCnt > dpb[indexTemp_short_right[j + 1]]->m_picture_frame.PicOrderCnt) //升序排列
            {
                int32_t temp = indexTemp_short_right[j]; //交换元素值
                indexTemp_short_right[j] = indexTemp_short_right[j + 1];
                indexTemp_short_right[j + 1] = temp;
            }
        }
    }

    //3. 按帧号(LongTermPicNum)升序排列长期参考帧(冒泡排序)
    for (i = 0; i < long_refs_count - 1; i++)
    {
        for (j = 0; j < long_refs_count - i - 1; j++)
        {
            if (dpb[indexTemp_long[j]]->m_picture_frame.LongTermPicNum > dpb[indexTemp_long[j + 1]]->m_picture_frame.LongTermPicNum) //升序排列
            {
                int32_t temp = indexTemp_long[j]; //交换元素值
                indexTemp_long[j] = indexTemp_long[j + 1];
                indexTemp_long[j + 1] = temp;
            }
        }
    }
    
    //4. 生成排序后的参考序列
    j = 0;

    for (i = 0; i < short_refs_count_left; i++)
    {
        RefPicList0[j++] = dpb[ indexTemp_short_left[i] ];
    }

    for (i = 0; i < short_refs_count_right; i++)
    {
        RefPicList0[j++] = dpb[ indexTemp_short_right[i] ];
    }

    for (i = 0; i < long_refs_count; i++)
    {
        RefPicList0[j++] = dpb[ indexTemp_long[i] ];
    }

    RefPicList0Length = j;

    //-------------RefPicList1----------------
    j = 0;
    
    short_refs_count_left = 0;
    short_refs_count_right = 0;
    long_refs_count = 0;
    not_refs_count = 0;

    memset(indexTemp_short_left, 0, sizeof(int32_t) * 16);
    memset(indexTemp_short_right, 0, sizeof(int32_t) * 16);
    memset(indexTemp_long, 0, sizeof(int32_t) * 16);
    memset(indexTemp3, 0, sizeof(int32_t) * 16);
    
    //1. 先把所有的短期参考帧放到数组的前半部分，把所有的长期参考帧放到数组的后半部分
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //reference frame or complementary reference field pair
        {
            if (dpb[i]->m_picture_frame.PicOrderCnt > PicOrderCnt)
            {
                indexTemp_short_left[short_refs_count_left] = i;
                short_refs_count_left++;
            }
            else //if (RefPicList0[i].m_picture_frame.PicOrderCnt <= PicOrderCnt)
            {
                indexTemp_short_right[short_refs_count_right] = i;
                short_refs_count_right++;
            }
        }
        else if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //reference frame or complementary reference field pair
        {
            indexTemp_long[long_refs_count] = i;
            long_refs_count++;
        }
        else
        {
            indexTemp3[not_refs_count] = i;
            not_refs_count++;
        }
    }
    
    //there shall be at least one reference frame or complementary reference field pair that is currently marked as "used for reference"
    RETURN_IF_FAILED(short_refs_count_left + short_refs_count_right + long_refs_count == 0, -1);

    //2. 按帧号(PicOrderCnt)按顺序排列短期参考帧(冒泡排序)
    for (i = 0; i < short_refs_count_left - 1; i++)
    {
        for (j = 0; j < short_refs_count_left - i - 1; j++)
        {
            if (dpb[indexTemp_short_left[j]]->m_picture_frame.PicOrderCnt > dpb[indexTemp_short_left[j + 1]]->m_picture_frame.PicOrderCnt) //升序排列
            {
                int32_t temp = indexTemp_short_left[j]; //交换元素值
                indexTemp_short_left[j] = indexTemp_short_left[j + 1];
                indexTemp_short_left[j + 1] = temp;
            }
        }
    }
    
    for (i = 0; i < short_refs_count_right - 1; i++)
    {
        for (j = 0; j < short_refs_count_right - i - 1; j++)
        {
            if (dpb[indexTemp_short_right[j]]->m_picture_frame.PicOrderCnt < dpb[indexTemp_short_right[j + 1]]->m_picture_frame.PicOrderCnt) //降序排列
            {
                int32_t temp = indexTemp_short_right[j]; //交换元素值
                indexTemp_short_right[j] = indexTemp_short_right[j + 1];
                indexTemp_short_right[j + 1] = temp;
            }
        }
    }

    //3. 按帧号(LongTermPicNum)升序排列长期参考帧(冒泡排序)
    for (i = 0; i < long_refs_count - 1; i++)
    {
        for (j = 0; j < long_refs_count - i - 1; j++)
        {
            if (dpb[indexTemp_long[j]]->m_picture_frame.LongTermPicNum > dpb[indexTemp_long[j + 1]]->m_picture_frame.LongTermPicNum) //升序排列
            {
                int32_t temp = indexTemp_long[j];
                indexTemp_long[j] = indexTemp_long[j + 1];
                indexTemp_long[j + 1] = temp;
            }
        }
    }
    
    //4. 生成排序后的参考序列
    j = 0;

    for (i = 0; i < short_refs_count_left; i++)
    {
        RefPicList1[j++] = dpb[ indexTemp_short_left[i] ];
    }

    for (i = 0; i < short_refs_count_right; i++)
    {
        RefPicList1[j++] = dpb[ indexTemp_short_right[i] ];
    }

    for (i = 0; i < long_refs_count; i++)
    {
        RefPicList1[j++] = dpb[ indexTemp_long[i] ];
    }

    RefPicList1Length = j;

    //--------------------------
    //When the reference picture list RefPicList1 has more than one entry and RefPicList1 is identical to 
    //the reference picture list RefPicList0, the first two entries RefPicList1[ 0 ] and RefPicList1[ 1 ] are switched
    int flag = 0;
    if (short_refs_count_left + short_refs_count_right + long_refs_count > 1)
    {
        for (i = 0; i < size_dpb; i++)
        {
            if (RefPicList1[i] != RefPicList0[i]) //FIXME: RefPicList1 is identical to the reference picture list RefPicList0
            {
                flag = 1;
                break;
            }
        }
    }

    if (flag == 0) //the first two entries RefPicList1[ 0 ] and RefPicList1[ 1 ] are switched
    {
        CH264Picture * tmp = RefPicList1[0];
        RefPicList1[0] = RefPicList1[1];
        RefPicList1[1] = tmp;
    }
    
    return 0;
}


//8.2.4.2.4 Initialisation process for reference picture lists for B slices in fields
//This initialisation process is invoked when decoding a B slice in a coded field.
int CH264PictureBase::Initialisation_process_for_reference_picture_lists_for_B_slices_in_fields(CH264Picture *(&dpb)[16], CH264Picture *(&RefPicList0)[16], CH264Picture *(&RefPicList1)[16],
        int32_t &RefPicList0Length, int32_t &RefPicList1Length)
{
    int ret = 0;
    int32_t size_dpb = 16;
    int32_t i = 0;
    int32_t j = 0;
    int32_t short_refs_count_left = 0;
    int32_t short_refs_count_right = 0;
    int32_t long_refs_count = 0;
    int32_t not_refs_count = 0;

    int32_t indexTemp_short_left[16] = {0};
    int32_t indexTemp_short_right[16] = {0};
    int32_t indexTemp_long[16] = {0};
    int32_t indexTemp3[16] = {0};

    CH264Picture * refFrameList0ShortTerm[16] = {NULL};
    CH264Picture * refFrameList1ShortTerm[16] = {NULL};
    CH264Picture * refFrameListLongTerm[16] = {NULL};

    //FIXME: When pic_order_cnt_type is equal to 0, reference pictures that are marked as "non-existing" as specified 
    //       in clause 8.2.5.2 are not included in either RefPicList0 or RefPicList1.

    //-------------RefPicList0----------------
    //1. 先把所有的短期参考帧放到数组的前半部分，把所有的长期参考帧放到数组的后半部分
    for (i = 0; i < size_dpb; i++)
    {
        if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) //reference frame or complementary reference field pair
        {
            if (dpb[i]->m_picture_frame.PicOrderCnt < PicOrderCnt)
            {
                indexTemp_short_left[short_refs_count_left] = i;
                short_refs_count_left++;
            }
            else //if (RefPicList0[i].m_picture_frame.PicOrderCnt >= PicOrderCnt)
            {
                indexTemp_short_right[short_refs_count_right] = i;
                short_refs_count_right++;
            }
        }
        else if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference) //reference frame or complementary reference field pair
        {
            indexTemp_long[long_refs_count] = i;
            long_refs_count++;
        }
        else
        {
            indexTemp3[not_refs_count] = i;
            not_refs_count++;
        }
    }
    
    //there shall be at least one reference frame or complementary reference field pair that is currently marked as "used for reference"
    RETURN_IF_FAILED(short_refs_count_left + short_refs_count_right + long_refs_count == 0, -1);

    //2. 按帧号降序排列短期参考帧(冒泡排序)
    for (i = 0; i < short_refs_count_left - 1; i++)
    {
        for (j = 0; j < short_refs_count_left - i - 1; j++)
        {
            if (dpb[indexTemp_short_left[j]]->m_picture_frame.PicOrderCnt < dpb[indexTemp_short_left[j + 1]]->m_picture_frame.PicOrderCnt) //降序排列
            {
                int32_t temp = indexTemp_short_left[j];
                indexTemp_short_left[j] = indexTemp_short_left[j + 1];
                indexTemp_short_left[j + 1] = temp;
            }
        }
    }
    
    for (i = 0; i < short_refs_count_right - 1; i++)
    {
        for (j = 0; j < short_refs_count_right - i - 1; j++)
        {
            if (dpb[indexTemp_short_right[j]]->m_picture_frame.PicOrderCnt > dpb[indexTemp_short_right[j + 1]]->m_picture_frame.PicOrderCnt) //升序排列
            {
                int32_t temp = indexTemp_short_right[j];
                indexTemp_short_right[j] = indexTemp_short_right[j + 1];
                indexTemp_short_right[j + 1] = temp;
            }
        }
    }

    //3. 按帧号升序排列长期参考帧(冒泡排序)
    for (i = 0; i < long_refs_count - 1; i++)
    {
        for (j = 0; j < long_refs_count - i - 1; j++)
        {
            if (dpb[indexTemp_long[j]]->m_picture_frame.LongTermFrameIdx > dpb[indexTemp_long[j + 1]]->m_picture_frame.LongTermFrameIdx) //升序排列
            {
                int32_t temp = indexTemp_long[j];
                indexTemp_long[j] = indexTemp_long[j + 1];
                indexTemp_long[j + 1] = temp;
            }
        }
    }
    
    //4. 生成排序后的参考序列
    j = 0;

    for (i = 0; i < short_refs_count_left; i++)
    {
        refFrameList0ShortTerm[j++] = dpb[ indexTemp_short_left[i] ];
    }

    for (i = 0; i < short_refs_count_right; i++)
    {
        refFrameList0ShortTerm[j++] = dpb[ indexTemp_short_right[i] ];
    }

    for (i = 0; i < long_refs_count; i++)
    {
        refFrameListLongTerm[j++] = dpb[ indexTemp_long[i] ];
    }
    
    //-------------RefPicList1----------------
    j = 0;
    
    for (i = 0; i < short_refs_count_right; i++)
    {
        refFrameList1ShortTerm[j++] = dpb[ indexTemp_short_right[i] ];
    }

    for (i = 0; i < short_refs_count_left; i++)
    {
        refFrameList1ShortTerm[j++] = dpb[ indexTemp_short_left[i] ];
    }
    
    //-------------------------
    //8.2.4.2.5 Initialisation process for reference picture lists in fields
    ret = Initialisation_process_for_reference_picture_lists_in_fields(refFrameList0ShortTerm, refFrameListLongTerm, RefPicList0, RefPicList0Length, 0);
    RETURN_IF_FAILED(ret != 0, -1);
    
    //8.2.4.2.5 Initialisation process for reference picture lists in fields
    ret = Initialisation_process_for_reference_picture_lists_in_fields(refFrameList1ShortTerm, refFrameListLongTerm, RefPicList1, RefPicList0Length, 1);
    RETURN_IF_FAILED(ret != 0, -1);
    
    //--------------------------
    //When the reference picture list RefPicList1 has more than one entry and RefPicList1 is identical to 
    //the reference picture list RefPicList0, the first two entries RefPicList1[ 0 ] and RefPicList1[ 1 ] are switched
    int flag = 0;
    if (short_refs_count_left + short_refs_count_right + long_refs_count > 1)
    {
        for (i = 0; i < size_dpb; i++)
        {
            if (RefPicList1[i] != RefPicList0[i]) //FIXME: RefPicList1 is identical to the reference picture list RefPicList0
            {
                flag = 1;
                break;
            }
        }
    }

    if (flag == 0) //the first two entries RefPicList1[ 0 ] and RefPicList1[ 1 ] are switched
    {
        CH264Picture * tmp = RefPicList1[0];
        RefPicList1[0] = RefPicList1[1];
        RefPicList1[1] = tmp;
    }

    return 0;
}


//8.2.4.2.5 Initialisation process for reference picture lists in fields
int CH264PictureBase::Initialisation_process_for_reference_picture_lists_in_fields(CH264Picture *(&refFrameListXShortTerm)[16], CH264Picture *(&refFrameListXLongTerm)[16], 
        CH264Picture *(&RefPicListX)[16], int32_t &RefPicListXLength, int32_t listX)
{
    int ret = 0;
    int32_t size = 16;
    int32_t i = 0;
    int32_t j = 0;
    int32_t index = 0;
    
    H264_PICTURE_CODED_TYPE coded_type = m_picture_coded_type;

    //-----------------------------
    for (i = 0; i < size; i++)
    {
        if (refFrameListXShortTerm[i]->m_picture_top_filed.m_picture_coded_type == coded_type) //starting with a field that has the same parity as the current field (when present).
        {
            RefPicListX[index] = refFrameListXShortTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = coded_type;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
            coded_type = (coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) ? H264_PICTURE_CODED_TYPE_BOTTOM_FIELD : H264_PICTURE_CODED_TYPE_TOP_FIELD;
            index++;
            j = i;
        }

        if (refFrameListXShortTerm[i]->m_picture_bottom_filed.m_picture_coded_type == coded_type) //starting with a field that has the same parity as the current field (when present).
        {
            RefPicListX[index] = refFrameListXShortTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = coded_type;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
            coded_type = (coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) ? H264_PICTURE_CODED_TYPE_BOTTOM_FIELD : H264_PICTURE_CODED_TYPE_TOP_FIELD;
            index++;
            j = i;
        }
    }

    //FIXME:
    //When there are no more short-term reference fields of the alternate parity in the ordered list of frames refFrameListXShortTerm, the next not yet indexed fields of 
    //the available parity are inserted into RefPicListX in the order in which they occur in the ordered list of frames refFrameListXShortTerm.
    for (i = j + 1; i < size; i++)
    {
        if (refFrameListXShortTerm[i]->m_picture_top_filed.m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD)
        {
            RefPicListX[index] = refFrameListXShortTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_TOP_FIELD;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
            index++;
        }

        if (refFrameListXShortTerm[i]->m_picture_bottom_filed.m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
        {
            RefPicListX[index] = refFrameListXShortTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_BOTTOM_FIELD;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
            index++;
        }
    }

    //-----------------------------
    coded_type = m_picture_coded_type;
    j = 0;

    for (i = 0; i < size; i++)
    {
        if (refFrameListXLongTerm[i]->m_picture_top_filed.m_picture_coded_type == coded_type) //starting with a field that has the same parity as the current field (when present).
        {
            RefPicListX[index] = refFrameListXLongTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = coded_type;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            coded_type = (coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) ? H264_PICTURE_CODED_TYPE_BOTTOM_FIELD : H264_PICTURE_CODED_TYPE_TOP_FIELD;
            index++;
            j = i;
        }
        
        if (refFrameListXLongTerm[i]->m_picture_bottom_filed.m_picture_coded_type == coded_type) //starting with a field that has the same parity as the current field (when present).
        {
            RefPicListX[index] = refFrameListXLongTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = coded_type;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            coded_type = (coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD) ? H264_PICTURE_CODED_TYPE_BOTTOM_FIELD : H264_PICTURE_CODED_TYPE_TOP_FIELD;
            index++;
            j = i;
        }
    }

    //FIXME:
    //When there are no more long-term reference fields of the alternate parity in the ordered list of frames refFrameListLongTerm, the next not yet indexed fields of 
    //the available parity are inserted into RefPicListX in the order in which they occur in the ordered list of frames refFrameListLongTerm.
    for (i = j + 1; i < size; i++)
    {
        if (refFrameListXLongTerm[i]->m_picture_top_filed.m_picture_coded_type == H264_PICTURE_CODED_TYPE_TOP_FIELD)
        {
            RefPicListX[index] = refFrameListXLongTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_TOP_FIELD;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            index++;
        }

        if (refFrameListXLongTerm[i]->m_picture_bottom_filed.m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD)
        {
            RefPicListX[index] = refFrameListXLongTerm[i];
            RefPicListX[index]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_BOTTOM_FIELD;
            RefPicListX[index]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            index++;
        }
    }

    RefPicListXLength = index;

    return 0;
}


//8.2.4.3 Modification process for reference picture lists
//参考图像列表的重排序过程
int CH264PictureBase::Modification_process_for_reference_picture_lists(CH264Picture *(&RefPicList0)[16], CH264Picture *(&RefPicList1)[16])
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (slice_header.ref_pic_list_modification_flag_l0 == 1)
    {
        slice_header.refIdxL0 = 0;
        
        for (int i = 0; i < slice_header.ref_pic_list_modification_count_l0; i++)
        {
            if (slice_header.modification_of_pic_nums_idc[0][i] == 0 || slice_header.modification_of_pic_nums_idc[0][i] == 1)
            {
                ret = Modification_process_of_reference_picture_lists_for_short_term_reference_pictures(slice_header.refIdxL0, slice_header.picNumL0Pred, slice_header.modification_of_pic_nums_idc[0][i], 
                        slice_header.abs_diff_pic_num_minus1[0][i], slice_header.num_ref_idx_l0_active_minus1, RefPicList0);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else if (slice_header.modification_of_pic_nums_idc[0][i] == 2)
            {
                ret = Modification_process_of_reference_picture_lists_for_long_term_reference_pictures(slice_header.refIdxL0, slice_header.picNumL0Pred, 
                        slice_header.num_ref_idx_l0_active_minus1, slice_header.long_term_pic_num[0][i], RefPicList0);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.modification_of_pic_nums_idc[0][i] == 3)
            {
                break; //the modification process for reference picture list RefPicList0 is finished.
            }
        }
    }
    
    if (slice_header.slice_type == H264_SLIECE_TYPE_B && slice_header.ref_pic_list_modification_flag_l1 == 1)
    {
        slice_header.refIdxL1 = 0;
        
        for (int i = 0; i < slice_header.ref_pic_list_modification_count_l1; i++)
        {
            if (slice_header.modification_of_pic_nums_idc[1][i] == 0 || slice_header.modification_of_pic_nums_idc[1][i] == 1)
            {
                ret = Modification_process_of_reference_picture_lists_for_short_term_reference_pictures(slice_header.refIdxL1, slice_header.picNumL1Pred, slice_header.modification_of_pic_nums_idc[1][i], 
                        slice_header.abs_diff_pic_num_minus1[1][i], slice_header.num_ref_idx_l1_active_minus1, RefPicList1);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else if (slice_header.modification_of_pic_nums_idc[1][i] == 2)
            {
                ret = Modification_process_of_reference_picture_lists_for_long_term_reference_pictures(slice_header.refIdxL1, slice_header.picNumL1Pred, 
                        slice_header.num_ref_idx_l1_active_minus1, slice_header.long_term_pic_num[1][i], RefPicList1);
                RETURN_IF_FAILED(ret != 0, ret);
            }
            else //if (slice_header.modification_of_pic_nums_idc[1][i] == 3)
            {
                break; //he modification process for reference picture list RefPicList1 is finished.
            }
        }
    }

    return 0;
}


//8.2.4.3.1 Modification process of reference picture lists for short-term reference pictures
int CH264PictureBase::Modification_process_of_reference_picture_lists_for_short_term_reference_pictures(int32_t &refIdxLX, int32_t &picNumLXPred, int32_t modification_of_pic_nums_idc, 
        int32_t abs_diff_pic_num_minus1, int32_t num_ref_idx_lX_active_minus1, CH264Picture *(&RefPicListX)[16])
{
    int32_t picNumLXNoWrap = 0;
    int32_t picNumLX = 0;
    int32_t cIdx = 0;
    int32_t nIdx = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (modification_of_pic_nums_idc == 0)
    {
        if ( picNumLXPred - ( abs_diff_pic_num_minus1 + 1 ) < 0 )
        {
            picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1 + 1 ) + slice_header.MaxPicNum;
        }
        else
        {
            picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1 + 1 );
        }
    }else //if (modification_of_pic_nums_idc == 1)
    {
        if ( picNumLXPred + ( abs_diff_pic_num_minus1 + 1 ) >= slice_header.MaxPicNum )
        {
            picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1 + 1 ) - slice_header.MaxPicNum;
        }
        else
        {
            picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1 + 1 );
        }
    }

    picNumLXPred = picNumLXNoWrap;

    //--------------------
    if ( picNumLXNoWrap > (int32_t)slice_header.CurrPicNum )
    {
        picNumLX = picNumLXNoWrap - slice_header.MaxPicNum;
    }
    else
    {
        picNumLX = picNumLXNoWrap;
    }

    //--------------------
    for (cIdx = num_ref_idx_lX_active_minus1 + 1; cIdx > refIdxLX; cIdx--)
    {
        RefPicListX[cIdx] = RefPicListX[cIdx - 1];
    }
    
    //--------------------
    for (cIdx = 0; cIdx < num_ref_idx_lX_active_minus1 + 1; cIdx++)
    {
        if (RefPicListX[cIdx]->PicNum == picNumLX
           && RefPicListX[ cIdx ]->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
          )
        {
            break;
        }
    }

    RefPicListX[ refIdxLX++ ] = RefPicListX[cIdx]; //short-term reference picture with PicNum equal to picNumLX
    nIdx = refIdxLX;
    
    for (cIdx = refIdxLX; cIdx <= num_ref_idx_lX_active_minus1 + 1; cIdx++)
    {
        if (RefPicListX[ cIdx ] != NULL)
        {
            int32_t PicNumF = (RefPicListX[ cIdx ]->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference) 
                              ? RefPicListX[ cIdx ]->PicNum : slice_header.MaxPicNum;
            if ( PicNumF != picNumLX ) //if ( PicNumF( RefPicListX[ cIdx ] ) != picNumLX )
            {
                RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
            }
        }
    }

    //Within this pseudo-code procedure, the length of the list RefPicListX is temporarily made one element longer than the length needed for the final list. 
    //After the execution of this procedure, only elements 0 through num_ref_idx_lX_active_minus1 of the list need to be retained.
    RefPicListX[ num_ref_idx_lX_active_minus1 + 1 ] = NULL; //列表最后一个元素是多余的

    return 0;
}


//8.2.4.3.2 Modification process of reference picture lists for long-term reference pictures
int CH264PictureBase::Modification_process_of_reference_picture_lists_for_long_term_reference_pictures(int32_t &refIdxLX, int32_t picNumLXPred, 
        int32_t num_ref_idx_lX_active_minus1, int32_t long_term_pic_num, CH264Picture *(&RefPicListX)[16])
{
    int32_t cIdx = 0;
    int32_t nIdx = 0;
    
    CH264SliceHeader & slice_header = m_h264_slice_header;

    for (cIdx = num_ref_idx_lX_active_minus1 + 1; cIdx > refIdxLX; cIdx--)
    {
        RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];
    }
    
    //--------------------
    for (cIdx = 0; cIdx < num_ref_idx_lX_active_minus1 + 1; cIdx++)
    {
        if (RefPicListX[cIdx]->LongTermPicNum == long_term_pic_num)
        {
            break;
        }
    }

    RefPicListX[ refIdxLX++ ] = RefPicListX[cIdx]; //long-term reference picture with LongTermPicNum equal to long_term_pic_num
    nIdx = refIdxLX;

    for (cIdx = refIdxLX; cIdx <= num_ref_idx_lX_active_minus1 + 1; cIdx++)
    {
        int32_t LongTermPicNumF = (RefPicListX[ cIdx ]->reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference)
                                  ? RefPicListX[ cIdx ]->LongTermPicNum : (2 * (MaxLongTermFrameIdx + 1));
        if ( LongTermPicNumF != long_term_pic_num )
        {
            RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
        }
    }

    return 0;
}


//8.2.5 Decoded reference picture marking process
//This process is invoked for decoded pictures when nal_ref_idc is not equal to 0.
int CH264PictureBase::Decoded_reference_picture_marking_process(CH264Picture *(&dpb)[16])
{
    int ret = 0;
    ret = Sequence_of_operations_for_decoded_reference_picture_marking_process(dpb);
    return ret;
}


//8.2.5.1 Sequence of operations for decoded reference picture marking process
int CH264PictureBase::Sequence_of_operations_for_decoded_reference_picture_marking_process(CH264Picture *(&dpb)[16])
{
    int ret = 0;
    CH264SliceHeader & slice_header = m_h264_slice_header;
    int32_t size_dpb = 16;

    //1. All slices of the current picture are decoded.
//    RETURN_IF_FAILED(m_is_decode_finished == 0, -1);

    //2. Depending on whether the current picture is an IDR picture, the following applies:
    if (slice_header.m_nal_unit.IdrPicFlag == 1) //IDR picture
    {
        //All reference pictures are marked as "unused for reference"
        for (int i = 0; i < size_dpb; i++)
        {
            dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
            dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
            if (dpb[i] != this->m_parent)
            {
                dpb[i]->m_picture_coded_type = H264_PICTURE_CODED_TYPE_UNKNOWN; //FIXME: 需要先将之前解码的所有帧输出去
            }
        }

        if (slice_header.long_term_reference_flag == 0) //标记自己为哪一种参考帧
        {
            reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
            MaxLongTermFrameIdx = NA; //"no long-term frame indices".
            m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;

            if (m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
            {
                m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
            }
            else
            {
                m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
            }
        }
        else //if (slice_header.long_term_reference_flag == 1)
        {
            reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            LongTermFrameIdx = 0;
            MaxLongTermFrameIdx = 0;
            m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

            if (m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
            {
                m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
            }
            else
            {
                m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
            }
        }
    }
    else //the current picture is not an IDR picture
    {
        if (slice_header.adaptive_ref_pic_marking_mode_flag == 0)
        {
            //8.2.5.3 Sliding window decoded reference picture marking process 滑动窗口解码参考图像的标识过程
            ret = Sliding_window_decoded_reference_picture_marking_process(dpb);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        else //if (slice_header.adaptive_ref_pic_marking_mode_flag == 1)
        {
            //8.2.5.4 Adaptive memory control decoded reference picture marking process
            ret = Adaptive_memory_control_decoded_reference_picture_marking_process(dpb);
            RETURN_IF_FAILED(ret != 0, ret);
        }
    }

    //3. When the current picture is not an IDR picture and it was not marked as "used for long-term reference" by 
    //memory_management_control_operation equal to 6, it is marked as "used for short-term reference".
    if (slice_header.m_nal_unit.IdrPicFlag != 1 //the current picture is not an IDR picture
        && memory_management_control_operation_6_flag != 6
        )
    {
        reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
        MaxLongTermFrameIdx = NA; //"no long-term frame indices".
        m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;

        if (m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
        {
            m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
        }
        else
        {
            m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
        }
    }

    return 0;
}


//8.2.5.2 Decoding process for gaps in frame_num
//This process is invoked when frame_num is not equal to PrevRefFrameNum and is not equal to ( PrevRefFrameNum + 1 ) % MaxFrameNum.
//NOTE 1 – Although this process is specified as a subclause within clause 8.2.5 (which defines a process that is invoked only when 
//         nal_ref_idc is not equal to 0), this process may also be invoked when nal_ref_idc is equal to 0 (as specified in clause 8). 
//         The reasons for the location of this clause within the structure of this Recommendation | International Standard are historical.
//NOTE 2 – This process can only be invoked for a conforming bitstream when gaps_in_frame_num_value_allowed_flag is equal to 1. 
//         When gaps_in_frame_num_value_allowed_flag is equal to 0 and frame_num is not equal to PrevRefFrameNum and is not equal to 
//         ( PrevRefFrameNum + 1 ) % MaxFrameNum, the decoding process should infer an unintentional loss of pictures.
int CH264PictureBase::Decoding_process_for_gaps_in_frame_num()
{
    
    return 0;
}


//8.2.5.3 Sliding window decoded reference picture marking process 滑动窗口解码参考图像的标识过程
//This process is invoked when adaptive_ref_pic_marking_mode_flag is equal to 0.
int CH264PictureBase::Sliding_window_decoded_reference_picture_marking_process(CH264Picture *(&dpb)[16])
{
    CH264SliceHeader & slice_header = m_h264_slice_header;

    if (m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD //If the current picture is a coded field that is the second field in decoding order of a complementary reference field pair
       && (m_parent->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
      ) //the first field has been marked as "used for short-term reference"
    {
        //the current picture and the complementary reference field pair are also marked as "used for short-term reference".
        reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
        m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_short_term_reference;
        m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
    }
    else
    {
        int32_t size_dpb = 16;
        int32_t numShortTerm = 0;
        int32_t numLongTerm = 0;

        for (int i = 0; i < size_dpb; i++)
        {
            if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
               || dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
               || dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
              )
            {
                numShortTerm++;
            }

            if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
               || dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
               || dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
              )
            {
                numLongTerm++;
            }
        }

        if (numShortTerm + numLongTerm == MAX(slice_header.m_sps.max_num_ref_frames, 1) && numShortTerm > 0)
        {
            CH264PictureBase * refPic = NULL;
            int32_t FrameNumWrap_smallest_index= -1;

            //When numShortTerm + numLongTerm is equal to Max( max_num_ref_frames, 1 ), the condition that numShortTerm 
            //is greater than 0 shall be fulfilled, and the short-term reference frame, complementary reference field 
            //pair or non-paired reference field that has the smallest value of FrameNumWrap is marked as "unused for reference". 
            //When it is a frame or a complementary field pair, both of its fields are also marked as "unused for reference".
            for (int i = 0; i < size_dpb; i++)
            {
                if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                   || dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                   || dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                  )
                {
                    if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
                    {
                        if (FrameNumWrap_smallest_index == -1)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_frame;
                        }

                        if (dpb[i]->m_picture_frame.FrameNumWrap < dpb[FrameNumWrap_smallest_index]->m_picture_frame.FrameNumWrap)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_frame;
                        }
                    }

                    if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                       || dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                      )
                    {
                        if (FrameNumWrap_smallest_index == -1)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_top_filed;
                        }

                        if (dpb[i]->m_picture_top_filed.FrameNumWrap < dpb[FrameNumWrap_smallest_index]->m_picture_top_filed.FrameNumWrap)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_top_filed;
                        }
                    }

                    if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference)
                    {
                        if (FrameNumWrap_smallest_index == -1)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_bottom_filed;
                        }

                        if (dpb[i]->m_picture_bottom_filed.FrameNumWrap < dpb[FrameNumWrap_smallest_index]->m_picture_bottom_filed.FrameNumWrap)
                        {
                            FrameNumWrap_smallest_index = i;
                            refPic = &dpb[i]->m_picture_bottom_filed;
                        }
                    }
                }
            }

            if (FrameNumWrap_smallest_index >= 0 && refPic != NULL)
            {
                refPic->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;

                if (refPic->m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
                {
                    refPic->m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    refPic->m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                }
                else if (refPic->m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                {
                    refPic->m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    refPic->m_parent->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                    refPic->m_parent->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    refPic->m_parent->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }
            }
        }
    }

    return 0;
}


//8.2.5.4 Adaptive memory control decoded reference picture marking process
//This process is invoked when adaptive_ref_pic_marking_mode_flag is equal to 1.
int CH264PictureBase::Adaptive_memory_control_decoded_reference_picture_marking_process(CH264Picture *(&dpb)[16])
{
    CH264SliceHeader & slice_header = m_h264_slice_header;
    
    int32_t size_dpb = 16;
    int32_t i = 0;
    int32_t j = 0;

    for (j = 0; j < slice_header.dec_ref_pic_marking_count; j++)
    {
        //The memory_management_control_operation command with value of 0 specifies the end of memory_management_control_operation commands.
        if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 0)
        {
            break;
        }

        //8.2.5.4.1 Marking process of a short-term reference picture as "unused for reference"
        //将短期图像标记为“不用于参考”
        if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 1)
        {
            int32_t picNumX = slice_header.CurrPicNum - (slice_header.m_dec_ref_pic_marking[j].difference_of_pic_nums_minus1 + 1);
            if (slice_header.field_pic_flag == 0)
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if ((dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                           && dpb[i]->m_picture_frame.PicNum == picNumX)
                       || (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                             && dpb[i]->m_picture_top_filed.PicNum == picNumX)
                       || (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                             && dpb[i]->m_picture_bottom_filed.PicNum == picNumX)
                        )
                    {
                        dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
            }
            else //if (field_pic_flag == 1)
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                          && dpb[i]->m_picture_top_filed.PicNum == picNumX)
                    {
                        dpb[i]->reference_marked_type = dpb[i]->m_picture_bottom_filed.reference_marked_type;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_bottom_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                    else if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                              && dpb[i]->m_picture_bottom_filed.PicNum == picNumX)
                    {
                        dpb[i]->reference_marked_type = dpb[i]->m_picture_top_filed.reference_marked_type;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_top_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
            }
        }

        //8.2.5.4.2 Marking process of a long-term reference picture as "unused for reference"
        //将长期图像标记为“不用于参考”
        else if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 2)
        {
            if (slice_header.field_pic_flag == 0)
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                          && dpb[i]->m_picture_frame.LongTermPicNum == slice_header.m_dec_ref_pic_marking[j].long_term_pic_num_2)
                    {
                        dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
            }
            else //if (field_pic_flag == 1) //FIXME: but the marking of the other field in the same reference frame or complementary reference field pair is not changed.
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                          && dpb[i]->m_picture_top_filed.LongTermPicNum == slice_header.m_dec_ref_pic_marking[j].long_term_pic_num_2)
                    {
                        dpb[i]->reference_marked_type = dpb[i]->m_picture_bottom_filed.reference_marked_type;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_bottom_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                    else if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                              && dpb[i]->m_picture_bottom_filed.LongTermPicNum == slice_header.m_dec_ref_pic_marking[j].long_term_pic_num_2)
                    {
                        dpb[i]->reference_marked_type = dpb[i]->m_picture_top_filed.reference_marked_type;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_top_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
            }
        }
        
        //8.2.5.4.3 Assignment process of a LongTermFrameIdx to a short-term reference picture
        //分配LongTermFrameIdx给一个短期参考图像
        else if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 3)
        {
            int32_t picNumX = slice_header.CurrPicNum - (slice_header.m_dec_ref_pic_marking[j].difference_of_pic_nums_minus1 + 1);

            //When LongTermFrameIdx equal to long_term_frame_idx is already assigned to a long-term reference frame or a long-term 
            //complementary reference field pair, that frame or complementary field pair and both of its fields are marked as "unused for reference".
            for (i = 0; i < size_dpb; i++)
            {
                if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                      && dpb[i]->m_picture_frame.LongTermFrameIdx == slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx)
                {
                    dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                    dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }
            }

            //When LongTermFrameIdx is already assigned to a reference field, and that reference field is not part of a complementary field pair that 
            //includes the picture specified by picNumX, that field is marked as "unused for reference".
            int32_t picNumXIndex = -1;
            for (i = 0; i < size_dpb; i++)
            {
                if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                       && dpb[i]->m_picture_top_filed.PicNum == picNumX)
                {
                    picNumXIndex = i;
                    break;
                }
                else if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                            && dpb[i]->m_picture_bottom_filed.PicNum == picNumX)
                {
                    picNumXIndex = i;
                    break;
                }
            }

            for (i = 0; i < size_dpb; i++)
            {
                if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                       && dpb[i]->m_picture_top_filed.LongTermFrameIdx == slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx)
                {
                    if (i != picNumXIndex)
                    {
                        dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_bottom_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = dpb[i]->m_picture_bottom_filed.reference_marked_type;
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
                else if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                            && dpb[i]->m_picture_bottom_filed.LongTermFrameIdx == slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx)
                {
                    if (i != picNumXIndex)
                    {
                        dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_top_filed.m_picture_coded_type;
                        dpb[i]->m_picture_frame.reference_marked_type = dpb[i]->m_picture_top_filed.reference_marked_type;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }
            }

            //-----------------------------------------
            if (slice_header.field_pic_flag == 0)
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if (dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                          && dpb[i]->m_picture_frame.PicNum == picNumX)
                    {
                        dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                        dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
                        dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

                        if (dpb[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                        {
                            dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                            dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                        }

                        LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                    }
                }
            }
            else //if (field_pic_flag == 1) //FIXME: What meaning 'When the field is part of a reference frame or a complementary reference field pair'
            {
                for (i = 0; i < size_dpb; i++)
                {
                    if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                          && dpb[i]->m_picture_top_filed.PicNum == picNumX)
                    {
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

                        if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference)
                        {
                            dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

                            if (dpb[i]->m_picture_frame.m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
                            {
                                dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
                            }
                            else if (dpb[i]->m_picture_frame.m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                            {
                                dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
                            }

                            dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                            dpb[i]->m_picture_bottom_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                        }

                        dpb[i]->m_picture_top_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                    }
                    else if (dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_short_term_reference
                              && dpb[i]->m_picture_bottom_filed.PicNum == picNumX)
                    {
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

                        if (dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference)
                        {
                            dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;

                            if (dpb[i]->m_picture_frame.m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME)
                            {
                                dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_FRAME;
                            }
                            else if (dpb[i]->m_picture_frame.m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                            {
                                dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR;
                            }

                            dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                            dpb[i]->m_picture_top_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                        }

                        dpb[i]->m_picture_bottom_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                    }
                }
            }
        }

        //8.2.5.4.4 Decoding process for MaxLongTermFrameIdx
        //基于MaxLongTermFrameIdx的标记过程
        else if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 4)
        {
            //All pictures for which LongTermFrameIdx is greater than max_long_term_frame_idx_plus1 − 1 
            //and that are marked as "used for long-term reference" are marked as "unused for reference".
            for (i = 0; i < size_dpb; i++)
            {
                if (dpb[i]->m_picture_frame.LongTermFrameIdx > (int)slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 - 1
                    && dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                    )
                {
                    dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }

                if (dpb[i]->m_picture_top_filed.LongTermFrameIdx > (int)slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 - 1
                    && dpb[i]->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                    )
                {
                    dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }

                if (dpb[i]->m_picture_bottom_filed.LongTermFrameIdx > (int)slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 - 1
                    && dpb[i]->m_picture_bottom_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                    )
                {
                    dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }

                if (dpb[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_FRAME
                    || dpb[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
                    )
                {
                    dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                    dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }
            }

            if (slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 == 0)
            {
                MaxLongTermFrameIdx = -1; //"no long-term frame indices"
            }
            else //if (max_long_term_frame_idx_plus1 > 0)
            {
                MaxLongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 - 1;
            }
        }
        
        //8.2.5.4.5 Marking process of all reference pictures as "unused for reference" and setting MaxLongTermFrameIdx to "no long-term frame indices"
        //所有参考图像标记为“不用于参考”
        else if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 5)
        {
            for (i = 0; i < size_dpb; i++)
            {
                dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;
                dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
            }

            MaxLongTermFrameIdx = NA;
            memory_management_control_operation_5_flag = 1;
        }
        
        //8.2.5.4.6 Process for assigning a long-term frame index to the current picture
        //分配一个长期帧索引给当前图像
        else if (slice_header.m_dec_ref_pic_marking[j].memory_management_control_operation == 6)
        {
            int32_t top_field_index = -1;

            for (i = 0; i < size_dpb; i++)
            {
                //When a variable LongTermFrameIdx equal to long_term_frame_idx is already assigned to a long-term reference frame or a long-term 
                //complementary reference field pair, that frame or complementary field pair and both of its fields are marked as "unused for reference".
                if (dpb[i]->m_picture_frame.LongTermFrameIdx == (int)slice_header.m_dec_ref_pic_marking[j].max_long_term_frame_idx_plus1 - 1
                    && dpb[i]->m_picture_frame.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                    )
                {
                    dpb[i]->m_picture_frame.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    dpb[i]->reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    dpb[i]->m_picture_coded_type_marked_as_refrence = H264_PICTURE_CODED_TYPE_UNKNOWN;

                    if (dpb[i]->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                    {
                        dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                        dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                    }
                }

                //When LongTermFrameIdx is already assigned to a reference field, and that reference field is not part of a complementary field pair 
                //that includes the current picture, that field is marked as "unused for reference".
                if (dpb[i]->m_picture_top_filed.LongTermFrameIdx == LongTermFrameIdx 
                    && m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
                    && (dpb[i] != m_parent))
                {
                    dpb[i]->reference_marked_type = dpb[i]->m_picture_bottom_filed.reference_marked_type;
                    dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_bottom_filed.m_picture_coded_type;
                    dpb[i]->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }
                else if (dpb[i]->m_picture_bottom_filed.LongTermFrameIdx == LongTermFrameIdx 
                    && m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR
                    && (dpb[i] != m_parent))
                {
                    dpb[i]->reference_marked_type = dpb[i]->m_picture_top_filed.reference_marked_type;
                    dpb[i]->m_picture_coded_type_marked_as_refrence = dpb[i]->m_picture_top_filed.m_picture_coded_type;
                    dpb[i]->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_unused_for_reference;
                }
            }
            
            reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
            LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
            memory_management_control_operation_6_flag = 6;

            //When field_pic_flag is equal to 0, both its fields are also marked as "used for long-term reference" 
            //and assigned LongTermFrameIdx equal to long_term_frame_idx.
            if (slice_header.field_pic_flag == 0)
            {
                //FIXME: what meaning 'both its fields'?
                if (m_parent->m_picture_coded_type == H264_PICTURE_CODED_TYPE_COMPLEMENTARY_FIELD_PAIR)
                {
                    m_parent->m_picture_top_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                    m_parent->m_picture_top_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                    m_parent->m_picture_bottom_filed.reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                    m_parent->m_picture_bottom_filed.LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
                }
            }
            
            //When field_pic_flag is equal to 1 and the current picture is the second field (in decoding order) of a complementary reference field pair, 
            //and the first field of the complementary reference field pair is also currently marked as "used for long-term reference", the complementary 
            //reference field pair is also marked as "used for long-term reference" and assigned LongTermFrameIdx equal to long_term_frame_idx.
            if (slice_header.field_pic_flag == 1 
                && m_picture_coded_type == H264_PICTURE_CODED_TYPE_BOTTOM_FIELD 
                && m_parent->m_picture_top_filed.reference_marked_type == H264_PICTURE_MARKED_AS_used_for_long_term_reference
                )
            {
                m_parent->reference_marked_type = H264_PICTURE_MARKED_AS_used_for_long_term_reference;
                LongTermFrameIdx = slice_header.m_dec_ref_pic_marking[j].long_term_frame_idx;
            }

            //TODO:
            //After marking the current decoded reference picture, the total number of frames with at least one field marked as "used for reference", 
            //plus the number of complementary field pairs with at least one field marked as "used for reference", plus the number of non-paired fields 
            //marked as "used for reference" shall not be greater than Max( max_num_ref_frames, 1 ).
        }
    }

    return 0;
}

