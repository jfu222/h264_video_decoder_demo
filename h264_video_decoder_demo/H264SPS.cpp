//
// H264SPS.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SPS.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"


//Table A-1 – Level limits
//level_idc is equal to a value of ten times the level number
//[][0] - Level number
//[][1] - Max decoded picture buffer size MaxDpbMbs (MBs)
int32_t LevelNumber_MaxDpbMbs[19][2] =
{
    { 10, 396    },
    { 11, 900    },
    { 12, 2376   },
    { 13, 2376   },
    { 20, 2376   },
    { 21, 4752   },
    { 22, 8100   },
    { 30, 8100   },
    { 31, 18000  },
    { 32, 20480  },
    { 40, 32768  },
    { 41, 32768  },
    { 42, 34816  },
    { 50, 110400 },
    { 51, 184320 },
    { 52, 184320 },
    { 60, 696320 },
    { 61, 696320 },
    { 62, 696320 },
};


//Table 6-1 – SubWidthC, and SubHeightC values derived from chroma_format_idc and separate_colour_plane_flag
/*
struct CHROMA_FORMAT_IDC_T
{
    int32_t     chroma_format_idc;
    int32_t     separate_colour_plane_flag;
    int32_t     Chroma_Format;
    int32_t     SubWidthC;
    int32_t     SubHeightC;
};*/

CHROMA_FORMAT_IDC_T g_chroma_format_idcs[5] = 
{
    {0, 0, MONOCHROME,            NA, NA},
    {1, 0, CHROMA_FORMAT_IDC_420, 2, 2},
    {2, 0, CHROMA_FORMAT_IDC_422, 2, 1},
    {3, 0, CHROMA_FORMAT_IDC_444, 1, 1},
    {3, 1, CHROMA_FORMAT_IDC_444, NA, NA},
};


CH264SPS::CH264SPS()
{
    profile_idc = 0;
    constraint_set0_flag = 0;
    constraint_set1_flag = 0;
    constraint_set2_flag = 0;
    constraint_set3_flag = 0;
    constraint_set4_flag = 0;
    constraint_set5_flag = 0;
    reserved_zero_2bits = 0;
    level_idc = 0;
    seq_parameter_set_id = 0;
    chroma_format_idc = 0;
    separate_colour_plane_flag = 0;
    bit_depth_luma_minus8 = 0;
    bit_depth_chroma_minus8 = 0;
    qpprime_y_zero_transform_bypass_flag = 0;
    seq_scaling_matrix_present_flag = 0;
    memset(seq_scaling_list_present_flag, 0, sizeof(int32_t) * 12);
    log2_max_frame_num_minus4 = 0;
    pic_order_cnt_type = 0;
    log2_max_pic_order_cnt_lsb_minus4 = 0;
    delta_pic_order_always_zero_flag = 0;
    offset_for_non_ref_pic = 0;
    offset_for_top_to_bottom_field = 0;
    num_ref_frames_in_pic_order_cnt_cycle = 0;
    memset(offset_for_ref_frame, 0, sizeof(int32_t) * H264_MAX_OFFSET_REF_FRAME_COUNT);
    max_num_ref_frames = 0;
    gaps_in_frame_num_value_allowed_flag = 0;
    pic_width_in_mbs_minus1 = 0;
    pic_height_in_map_units_minus1 = 0;
    frame_mbs_only_flag = 0;
    mb_adaptive_frame_field_flag = 0;
    direct_8x8_inference_flag = 0;
    frame_cropping_flag = 0;
    frame_crop_left_offset = 0;
    frame_crop_right_offset = 0;
    frame_crop_top_offset = 0;
    frame_crop_bottom_offset = 0;
    vui_parameters_present_flag = 0;
    
    memset(ScalingList4x4, 0, sizeof(int32_t) * 6 * 16);
    memset(ScalingList8x8, 0, sizeof(int32_t) * 6 * 64);
    PicWidthInMbs = 0;
    PicHeightInMapUnits = 0;
    PicSizeInMapUnits = 0;
    ChromaArrayType = 0;

    BitDepthY = 0;
    QpBdOffsetY = 0;
    BitDepthC = 0;
    QpBdOffsetC = 0;

    SubWidthC = 0;
    SubHeightC = 0;
    MbWidthC = 0;
    MbHeightC = 0;
    Chroma_Format = 0;
    RawMbBits = 0;
    MaxFrameNum = 0;
    ExpectedDeltaPerPicOrderCntCycle = 0;
    fps = 0.0;
}


CH264SPS::~CH264SPS()
{

}


int CH264SPS::printInfo()
{
    printf("---------SPS info------------\n");
    printf("profile_idc=%d;\n", profile_idc);
    printf("constraint_set0_flag=%d;\n", constraint_set0_flag);
    printf("constraint_set1_flag=%d;\n", constraint_set1_flag);
    printf("constraint_set2_flag=%d;\n", constraint_set2_flag);
    printf("constraint_set3_flag=%d;\n", constraint_set3_flag);
    printf("constraint_set4_flag=%d;\n", constraint_set4_flag);
    printf("constraint_set5_flag=%d;\n", constraint_set5_flag);
    printf("reserved_zero_2bits=%d;\n", reserved_zero_2bits);
    printf("level_idc=%d;\n", level_idc);
    printf("seq_parameter_set_id=%d;\n", seq_parameter_set_id);
    printf("chroma_format_idc=%d;\n", chroma_format_idc);
    printf("separate_colour_plane_flag=%d;\n", separate_colour_plane_flag);
    printf("bit_depth_luma_minus8=%d;\n", bit_depth_luma_minus8);
    printf("bit_depth_chroma_minus8=%d;\n", bit_depth_chroma_minus8);
    printf("qpprime_y_zero_transform_bypass_flag=%d;\n", qpprime_y_zero_transform_bypass_flag);
    printf("seq_scaling_matrix_present_flag=%d;\n", seq_scaling_matrix_present_flag);
    
    printf("seq_scaling_list_present_flag[0..11]: ");
    for (int i = 0; i< 12; ++i)
    {
        printf("%d ", seq_scaling_list_present_flag[i]);
    }
    printf("\n");

    printf("log2_max_frame_num_minus4=%d;\n", log2_max_frame_num_minus4);
    printf("pic_order_cnt_type=%d;\n", pic_order_cnt_type);
    printf("log2_max_pic_order_cnt_lsb_minus4=%d;\n", log2_max_pic_order_cnt_lsb_minus4);
    printf("delta_pic_order_always_zero_flag=%d;\n", delta_pic_order_always_zero_flag);
    printf("offset_for_non_ref_pic=%d;\n", offset_for_non_ref_pic);
    printf("offset_for_top_to_bottom_field=%d;\n", offset_for_top_to_bottom_field);
    printf("num_ref_frames_in_pic_order_cnt_cycle=%d;\n", num_ref_frames_in_pic_order_cnt_cycle);

    printf("offset_for_ref_frame[0..255]: ");
    for (int i = 0; i< H264_MAX_OFFSET_REF_FRAME_COUNT; ++i)
    {
        printf("%d ", offset_for_ref_frame[i]);
    }
    printf("\n");

    printf("max_num_ref_frames=%d;\n", max_num_ref_frames);
    printf("gaps_in_frame_num_value_allowed_flag=%d;\n", gaps_in_frame_num_value_allowed_flag);
    printf("pic_width_in_mbs_minus1=%d;\n", pic_width_in_mbs_minus1);
    printf("pic_height_in_map_units_minus1=%d;\n", pic_height_in_map_units_minus1);
    printf("frame_mbs_only_flag=%d;\n", frame_mbs_only_flag);
    printf("mb_adaptive_frame_field_flag=%d;\n", mb_adaptive_frame_field_flag);
    printf("direct_8x8_inference_flag=%d;\n", direct_8x8_inference_flag);
    printf("frame_cropping_flag=%d;\n", frame_cropping_flag);
    printf("frame_crop_left_offset=%d;\n", frame_crop_left_offset);
    printf("frame_crop_right_offset=%d;\n", frame_crop_right_offset);
    printf("frame_crop_top_offset=%d;\n", frame_crop_top_offset);
    printf("frame_crop_bottom_offset=%d;\n", frame_crop_bottom_offset);
    printf("vui_parameters_present_flag=%d;\n", vui_parameters_present_flag);
    
    printf("---------SPS-vui-m_vui info------------\n");
    m_vui.printInfo();
    
    printf("PicWidthInMbs=%d;\n", PicWidthInMbs);
    printf("FrameHeightInMbs=%d;\n", FrameHeightInMbs);
    printf("PicHeightInMapUnits=%d;\n", PicHeightInMapUnits);
    printf("PicSizeInMapUnits=%d=PicWidthInMbs * PicHeightInMapUnits=%d * %d;\n", PicSizeInMapUnits, PicWidthInMbs, PicHeightInMapUnits);
    printf("ChromaArrayType=%d;\n", ChromaArrayType);
    
    printf("BitDepthY=%d;\n", BitDepthY);
    printf("QpBdOffsetY=%d;\n", QpBdOffsetY);
    printf("BitDepthC=%d;\n", BitDepthC);
    printf("QpBdOffsetC=%d;\n", QpBdOffsetC);
    
    printf("SubWidthC=%d;\n", SubWidthC);
    printf("SubHeightC=%d;\n", SubHeightC);
    printf("MbWidthC=%d;\n", MbWidthC);
    printf("MbHeightC=%d;\n", MbHeightC);
    printf("Chroma_Format=%d;\n", Chroma_Format);

    printf("PicWidthInSamplesL=%d;\n", PicWidthInSamplesL);
    printf("PicWidthInSamplesC=%d;\n", PicWidthInSamplesC);

    printf("MaxFrameNum=%d;\n", MaxFrameNum);
    printf("fps=%.4f;\n", fps);

    return 0;
}


int CH264SPS::seq_parameter_set_data(CBitstream &bs)
{
    int ret = 0;
    
    int32_t i = 0;
    CH264Golomb gb;

    this->profile_idc = bs.readBits(8); //profile_idc 0 u(8)
    this->constraint_set0_flag = bs.readBits(1); //constraint_set0_flag 0 u(1)
    this->constraint_set1_flag = bs.readBits(1); //constraint_set1_flag 0 u(1)
    this->constraint_set2_flag = bs.readBits(1); //constraint_set2_flag 0 u(1)
    this->constraint_set3_flag = bs.readBits(1); //constraint_set3_flag 0 u(1)
    this->constraint_set4_flag = bs.readBits(1); //constraint_set4_flag 0 u(1)
    this->constraint_set5_flag = bs.readBits(1); //constraint_set5_flag 0 u(1)
    this->reserved_zero_2bits = bs.readBits(2); //reserved_zero_2bits /* equal to 0 */ 0 u(2)
    this->level_idc = bs.readBits(8); //level_idc 0 u(8)
    this->seq_parameter_set_id = gb.get_ue_golomb(bs); //seq_parameter_set_id 0 ue(v)

    this->chroma_format_idc = 1; //When chroma_format_idc is not present, it shall be inferred to be equal to 1 (4:2:0 chroma format).
    int32_t profile_idc = this->profile_idc;

    //profile_idc == 66 //A.2.1 Baseline profile //Only I and P slice types may be present.
    //profile_idc == 77 //A.2.2 Main profile //Only I, P, and B slice types may be present.
    //profile_idc == 88 //A.2.3 Extended profile

    if (profile_idc == 100 //A.2.4 High profile //Only I, P, and B slice types may be present.
       || profile_idc == 110 //A.2.5(A.2.8) High 10 (Intra) profile //Only I, P, and B slice types may be present.
       || profile_idc == 122 //A.2.6(A.2.9) High 4:2:2 (Intra) profile //Only I, P, and B slice types may be present.
       || profile_idc == 244 //A.2.7(A.2.10) High 4:4:4 Predictive/Intra profile //Only I, P, B slice types may be present.
       || profile_idc == 44 //A.2.11 CAVLC 4:4:4 Intra profile
       || profile_idc == 83 //G.10.1.2.1 Scalable Constrained High profile (SVC) //Only I, P, EI, and EP slices shall be present.
       || profile_idc == 86 //Scalable High Intra profile (SVC)
       || profile_idc == 118 //Stereo High profile (MVC)
       || profile_idc == 128 //Multiview High profile (MVC)
       || profile_idc == 138 //Multiview Depth High profile (MVCD)
       || profile_idc == 139 //
       || profile_idc == 134 //
       || profile_idc == 135 //
      )
    {
        this->chroma_format_idc = gb.get_ue_golomb(bs); //chroma_format_idc 0 ue(v)
        this->separate_colour_plane_flag = 0; //When separate_colour_plane_flag is not present, it shall be inferred to be equal to 0.
        if (this->chroma_format_idc == 3)
        {
            this->separate_colour_plane_flag = bs.readBits(1); //separate_colour_plane_flag 0 u(1)
        }
        
        this->bit_depth_luma_minus8 = gb.get_ue_golomb(bs); //bit_depth_luma_minus8 0 ue(v)
        this->bit_depth_chroma_minus8 = gb.get_ue_golomb(bs); //bit_depth_chroma_minus8 0 ue(v)
        this->qpprime_y_zero_transform_bypass_flag = bs.readBits(1); //qpprime_y_zero_transform_bypass_flag 0 u(1)
        this->seq_scaling_matrix_present_flag = bs.readBits(1); //seq_scaling_matrix_present_flag 0 u(1)

        if (this->seq_scaling_matrix_present_flag)
        {
            for (i = 0; i < ((this->chroma_format_idc != 3) ? 8 : 12); i++)
            {
                this->seq_scaling_list_present_flag[i] = bs.readBits(1); //seq_scaling_list_present_flag[ i ] 0 u(1)
                if (this->seq_scaling_list_present_flag[i] == 1)
                {
                    if (i < 6)
                    {
                        ret = scaling_list(bs, ScalingList4x4[i], 16, UseDefaultScalingMatrix4x4Flag[i]);
                        RETURN_IF_FAILED(ret != 0, ret);
                    }else
                    {
                        ret = scaling_list(bs, ScalingList8x8[i - 6], 64, UseDefaultScalingMatrix8x8Flag[i - 6]);
                        RETURN_IF_FAILED(ret != 0, ret);
                    }
                }
            }
        }
    }

    this->log2_max_frame_num_minus4 = gb.get_ue_golomb(bs); //log2_max_frame_num_minus4 0 ue(v)
    this->pic_order_cnt_type = gb.get_ue_golomb(bs); //pic_order_cnt_type 0 ue(v)

    if (this->pic_order_cnt_type == 0)
    {
        this->log2_max_pic_order_cnt_lsb_minus4 = gb.get_ue_golomb(bs); //log2_max_pic_order_cnt_lsb_minus4 0 ue(v)
    }
    else if (this->pic_order_cnt_type == 1)
    {
        this->delta_pic_order_always_zero_flag = bs.readBits(1); //delta_pic_order_always_zero_flag 0 u(1)
        this->offset_for_non_ref_pic = gb.get_se_golomb(bs); //offset_for_non_ref_pic 0 se(v)
        this->offset_for_top_to_bottom_field = gb.get_se_golomb(bs); //offset_for_top_to_bottom_field 0 se(v)
        this->num_ref_frames_in_pic_order_cnt_cycle = gb.get_ue_golomb(bs); //num_ref_frames_in_pic_order_cnt_cycle 0 ue(v)

        for (i = 0; i < (int32_t)this->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            this->offset_for_ref_frame[i] = gb.get_se_golomb(bs); //offset_for_ref_frame[ i ] 0 se(v)
        }
    }
    
    this->max_num_ref_frames = gb.get_ue_golomb(bs); //max_num_ref_frames 0 ue(v)
    this->gaps_in_frame_num_value_allowed_flag = bs.readBits(1); //gaps_in_frame_num_value_allowed_flag 0 u(1)
    this->pic_width_in_mbs_minus1 = gb.get_ue_golomb(bs); //pic_width_in_mbs_minus1 0 ue(v)
    this->pic_height_in_map_units_minus1 = gb.get_ue_golomb(bs); //pic_height_in_map_units_minus1 0 ue(v)
    this->frame_mbs_only_flag = bs.readBits(1); //frame_mbs_only_flag 0 u(1)

    if (!this->frame_mbs_only_flag)
    {
        this->mb_adaptive_frame_field_flag = bs.readBits(1); //mb_adaptive_frame_field_flag 0 u(1)
    }

    this->direct_8x8_inference_flag = bs.readBits(1); //direct_8x8_inference_flag 0 u(1)
    this->frame_cropping_flag = bs.readBits(1); //frame_cropping_flag 0 u(1)

    if (this->frame_cropping_flag)
    {
        this->frame_crop_left_offset = gb.get_ue_golomb(bs); //frame_crop_left_offset 0 ue(v)
        this->frame_crop_right_offset = gb.get_ue_golomb(bs); //frame_crop_right_offset 0 ue(v)
        this->frame_crop_top_offset = gb.get_ue_golomb(bs); //frame_crop_top_offset 0 ue(v)
        this->frame_crop_bottom_offset = gb.get_ue_golomb(bs); //frame_crop_bottom_offset 0 ue(v)
    }

    this->vui_parameters_present_flag = bs.readBits(1); //vui_parameters_present_flag 0 u(1)

    if (this->vui_parameters_present_flag)
    {
        ret = m_vui.vui_parameters(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }
    
    //---------------------------
    fps = 25.0;

    if (this->vui_parameters_present_flag && m_vui.timing_info_present_flag)
    {
        fps = 1.0 * m_vui.time_scale / m_vui.num_units_in_tick;

        //if (m_vui.fixed_frame_rate_flag)
        {
            fps /= 2.0; //FIXME: 
        }
    }

    //---------------------------
    PicWidthInMbs = pic_width_in_mbs_minus1 + 1;
    PicHeightInMapUnits = pic_height_in_map_units_minus1 + 1;
    PicSizeInMapUnits = PicWidthInMbs * PicHeightInMapUnits;
    FrameHeightInMbs = ( 2 - frame_mbs_only_flag ) * PicHeightInMapUnits;
    
    //---------------------------
    if (m_vui.max_num_reorder_frames == -1) //When the max_num_reorder_frames syntax element is not present
    {
        //max_num_reorder_frames shall be in the range of 0 to max_dec_frame_buffering, inclusive
        //If profile_idc is equal to 44, 86, 100, 110, 122, or 244 and constraint_set3_flag is equal to 1, 
        //the value of max_num_reorder_frames shall be inferred to be equal to 0.
        if ((profile_idc == 44
             || profile_idc == 86
             || profile_idc == 100
             || profile_idc == 110
             || profile_idc == 122
             || profile_idc == 244
            )
            && constraint_set3_flag == 1
           )
        {
            m_vui.max_num_reorder_frames = 0;
        }
        else
        {
            int32_t MaxDpbFrames = 0;

            for (i = 0; i < 19; ++i)
            {
                if (level_idc == LevelNumber_MaxDpbMbs[i][0])
                {
                    MaxDpbFrames = MIN( LevelNumber_MaxDpbMbs[i][1] / ( PicWidthInMbs * FrameHeightInMbs ), 16 ); //Table A-1.
                    break;
                }
            }
            
            m_vui.max_num_reorder_frames = MaxDpbFrames; //此处的MaxDpbFrames就代表了码流中是否包含B帧，如果MaxDpbFrames>=2，则表示码流中含有B帧
        }
    }

    RETURN_IF_FAILED(m_vui.max_num_reorder_frames > 16, -1);

    //---------------------------
    if (separate_colour_plane_flag == 0)
    {
        ChromaArrayType = chroma_format_idc;
    }
    else //if (separate_colour_plane_flag == 1)
    {
        ChromaArrayType = 0;
    }

    BitDepthY = 8 + bit_depth_luma_minus8;
    QpBdOffsetY = 6 * bit_depth_luma_minus8;
    BitDepthC = 8 + bit_depth_chroma_minus8;
    QpBdOffsetC = 6 * bit_depth_chroma_minus8;

    if (chroma_format_idc == 0 || separate_colour_plane_flag == 1)
    {
        MbWidthC = 0;
        MbHeightC = 0;
    }
    else
    {
        int32_t index = chroma_format_idc;
        if (chroma_format_idc == 3 && separate_colour_plane_flag == 1)
        {
            index = 4;
        }
        Chroma_Format = g_chroma_format_idcs[index].Chroma_Format;
        SubWidthC = g_chroma_format_idcs[index].SubWidthC;
        SubHeightC = g_chroma_format_idcs[index].SubHeightC;

        MbWidthC = 16 / SubWidthC;
        MbHeightC = 16 / SubHeightC;
    }

    PicWidthInSamplesL = PicWidthInMbs * 16;
    PicWidthInSamplesC = PicWidthInMbs * MbWidthC;
    RawMbBits = 256 * BitDepthY + 2 * MbWidthC * MbHeightC * BitDepthC;

    MaxFrameNum = h264_power2(log2_max_frame_num_minus4 + 4);
    MaxPicOrderCntLsb = h264_power2(log2_max_pic_order_cnt_lsb_minus4 + 4);

    if (pic_order_cnt_type == 1)
    {
        ExpectedDeltaPerPicOrderCntCycle = 0;
        for (i = 0; i < (int32_t)num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            ExpectedDeltaPerPicOrderCntCycle += offset_for_ref_frame[i];
        }
    }

    //----------------------
    int ret2 = printInfo();

    return ret;
}


int CH264SPS::get_h264_profile(char str[100])
{
    switch (profile_idc)
    {
    case 66:
        {
            sprintf(str, "H264 Baseline (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 77:
        {
            sprintf(str, "H264 Main (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 88:
        {
            sprintf(str, "H264 Extended (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 100:
        {
            sprintf(str, "H264 High (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 110:
        {
            sprintf(str, "H264 High 10 (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 122:
        {
            sprintf(str, "H264 High 4:2:2 (Intra) (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    case 244:
        {
            sprintf(str, "H264 High 4:4:4 (Predictive/Intra) (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    default:
        {
            sprintf(str, "H264 Unkown (profile=%d, level=%.1f)", profile_idc, level_idc / 10.0);
            break;
        }
    }
    
    return 0;
}

