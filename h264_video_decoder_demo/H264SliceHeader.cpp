//
// H264SliceHeader.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SliceHeader.h"
#include "H264Golomb.h"
#include "CommonFunction.h"
#include "H264CommonFunc.h"


CH264SliceHeader::CH264SliceHeader()
{
    first_mb_in_slice = 0;
    slice_type = 0;
    pic_parameter_set_id = 0;
    colour_plane_id = 0;
    frame_num = 0;
    field_pic_flag = 0;
    bottom_field_flag = 0;
    idr_pic_id = 0;
    pic_order_cnt_lsb = 0;
    delta_pic_order_cnt_bottom = 0;
    delta_pic_order_cnt[0] = 0;
    delta_pic_order_cnt[1] = 0;
    redundant_pic_cnt = 0;
    direct_spatial_mv_pred_flag = 0;
    num_ref_idx_active_override_flag = 0;
    num_ref_idx_l0_active_minus1 = 0;
    num_ref_idx_l1_active_minus1 = 0;
    cabac_init_idc = 0;
    slice_qp_delta = 0;
    sp_for_switch_flag = 0;
    slice_qs_delta = 0;
    disable_deblocking_filter_idc = 0;
    slice_alpha_c0_offset_div2 = 0;
    slice_beta_offset_div2 = 0;
    slice_group_change_cycle = 0;

    ref_pic_list_modification_flag_l0 = 0;
    memset(modification_of_pic_nums_idc, 0, sizeof(int32_t) * 32 * 2);
    memset(abs_diff_pic_num_minus1, 0, sizeof(int32_t) * 32 * 2);
    memset(long_term_pic_num, 0, sizeof(int32_t) * 32 * 2);
    ref_pic_list_modification_flag_l1 = 0;
    ref_pic_list_modification_count_l0 = 0;
    ref_pic_list_modification_count_l1 = 0;
    
    luma_log2_weight_denom = 0;
    chroma_log2_weight_denom = 0;
    luma_weight_l0_flag = 0;
    memset(luma_weight_l0, 0, sizeof(int32_t) * 32);
    memset(luma_offset_l0, 0, sizeof(int32_t) * 32);
    chroma_weight_l0_flag = 0;
    memset(chroma_weight_l0, 0, sizeof(int32_t) * 32 * 2);
    memset(chroma_offset_l0, 0, sizeof(int32_t) * 32 * 2);
    luma_weight_l1_flag = 0;
    memset(luma_weight_l1, 0, sizeof(int32_t) * 32);
    memset(luma_offset_l1, 0, sizeof(int32_t) * 32);
    chroma_weight_l1_flag = 0;
    memset(chroma_weight_l1, 0, sizeof(int32_t) * 32 * 2);
    memset(chroma_offset_l1, 0, sizeof(int32_t) * 32 * 2);

    no_output_of_prior_pics_flag = 0;
    long_term_reference_flag = 0;
    adaptive_ref_pic_marking_mode_flag = 0;
    memset(m_dec_ref_pic_marking, 0, sizeof(DEC_REF_PIC_MARKING) * 32);
    dec_ref_pic_marking_count = 0;

    slice_id = 0;
    syntax_element_categories = 0;
    slice_type_fixed = 0;
    mb_cnt = 0;
    QPY_prev = 0;
    SliceQPY = 0;
    MbaffFrameFlag = 0;
    PicHeightInMbs = 0;
    PicHeightInSamplesL = 0;
    PicHeightInSamplesC = 0;
    PicSizeInMbs = 0;
    MaxPicNum = 0;
    CurrPicNum = 0;
    SliceGroupChangeRate = 0;
    MapUnitsInSliceGroup0 = 0;
    QSY = 0;
    picNumL0Pred = 0;
    picNumL1Pred = 0;
    refIdxL0 = 0;
    refIdxL1 = 0;

    mapUnitToSliceGroupMap = NULL;
    MbToSliceGroupMap = NULL;
    PrevRefFrameNum = 0;
    UnusedShortTermFrameNum = 0;
    FilterOffsetA = 0;
    FilterOffsetB = 0;
    
    memset(ScalingList4x4, 0, sizeof(int32_t) * 6 * 16);
    memset(ScalingList8x8, 0, sizeof(int32_t) * 6 * 64);
    m_picture_coded_type = H264_PICTURE_CODED_TYPE_UNKNOWN;
    m_is_malloc_mem_self = 0;
}


CH264SliceHeader::~CH264SliceHeader()
{
    if (m_is_malloc_mem_self == 1)
    {
        SAFE_FREE(this->mapUnitToSliceGroupMap);
        SAFE_FREE(this->MbToSliceGroupMap);
    }
    else
    {
        this->mapUnitToSliceGroupMap = NULL;
        this->MbToSliceGroupMap = NULL;
    }

    m_is_malloc_mem_self = 0;
}


CH264SliceHeader & CH264SliceHeader::operator = (const CH264SliceHeader &src)
{
    int ret = 0;
    bool isMallocAndCopyData = false;

    ret = copyData(src, isMallocAndCopyData); //重载的等号运算符，默认不my_malloc数据

    return *this;
}


int CH264SliceHeader::copyData(const CH264SliceHeader &src, bool isCopyMallocData)
{
    if (isCopyMallocData)
    {
        SAFE_FREE(this->mapUnitToSliceGroupMap);
        SAFE_FREE(this->MbToSliceGroupMap);

        memcpy(this, &src, sizeof(CH264SliceHeader));
        
        this->mapUnitToSliceGroupMap = (int32_t *)my_malloc(sizeof(int32_t) * src.m_sps.PicSizeInMapUnits);
        RETURN_IF_FAILED(this->mapUnitToSliceGroupMap == NULL, -1);
        memcpy(this->mapUnitToSliceGroupMap, src.mapUnitToSliceGroupMap, sizeof(int32_t) * src.m_sps.PicSizeInMapUnits);
        
        this->MbToSliceGroupMap = (int32_t *)my_malloc(sizeof(int32_t) * src.PicSizeInMbs);
        RETURN_IF_FAILED(this->MbToSliceGroupMap == NULL, -2);
        memcpy(this->MbToSliceGroupMap, src.MbToSliceGroupMap, sizeof(int32_t) * src.PicSizeInMbs);

        m_is_malloc_mem_self = 1;
    }
    else
    {
        memcpy(this, &src, sizeof(CH264SliceHeader));
        m_is_malloc_mem_self = 0;
    }

    return 0;
}


int CH264SliceHeader::printInfo()
{
    printf("---------Slice Header info------------\n");
    printf("first_mb_in_slice=%d;\n", first_mb_in_slice);
    printf("slice_type=%d;\n", slice_type);
    printf("pic_parameter_set_id=%d;\n", pic_parameter_set_id);
    printf("colour_plane_id=%d;\n", colour_plane_id);
    printf("frame_num=%d;\n", frame_num);
    printf("field_pic_flag=%d;\n", field_pic_flag);
    printf("bottom_field_flag=%d;\n", bottom_field_flag);
    printf("idr_pic_id=%d;\n", idr_pic_id);
    printf("pic_order_cnt_lsb=%d;\n", pic_order_cnt_lsb);
    printf("delta_pic_order_cnt_bottom=%d;\n", delta_pic_order_cnt_bottom);
    printf("delta_pic_order_cnt[0]=%d;\n", delta_pic_order_cnt[0]);
    printf("delta_pic_order_cnt[1]=%d;\n", delta_pic_order_cnt[1]);
    printf("redundant_pic_cnt=%d;\n", redundant_pic_cnt);
    printf("direct_spatial_mv_pred_flag=%d;\n", direct_spatial_mv_pred_flag);
    printf("num_ref_idx_active_override_flag=%d;\n", num_ref_idx_active_override_flag);
    printf("num_ref_idx_l0_active_minus1=%d;\n", num_ref_idx_l0_active_minus1);
    printf("num_ref_idx_l1_active_minus1=%d;\n", num_ref_idx_l1_active_minus1);
    printf("cabac_init_idc=%d;\n", cabac_init_idc);
    printf("slice_qp_delta=%d;\n", slice_qp_delta);
    printf("sp_for_switch_flag=%d;\n", sp_for_switch_flag);
    printf("slice_qs_delta=%d;\n", slice_qs_delta);
    printf("disable_deblocking_filter_idc=%d;\n", disable_deblocking_filter_idc);
    printf("slice_alpha_c0_offset_div2=%d;\n", slice_alpha_c0_offset_div2);
    printf("slice_beta_offset_div2=%d;\n", slice_beta_offset_div2);
    printf("slice_group_change_cycle=%d;\n", slice_group_change_cycle);
    
    printf("slice_type_fixed=%d;\n", slice_type_fixed);
    printf("mb_cnt=%d;\n", mb_cnt);
    printf("SliceQPY=%d;\n", SliceQPY);
    printf("MbaffFrameFlag=%d;\n", MbaffFrameFlag);
    printf("PicHeightInMbs=%d;\n", PicHeightInMbs);
    printf("PicHeightInSamplesL=%d;\n", PicHeightInSamplesL);
    printf("PicHeightInSamplesC=%d;\n", PicHeightInSamplesC);
    printf("PicSizeInMbs=%d;\n", PicSizeInMbs);
    printf("MaxPicNum=%d;\n", MaxPicNum);
    printf("CurrPicNum=%d;\n", CurrPicNum);
    printf("SliceGroupChangeRate=%d;\n", SliceGroupChangeRate);
    printf("MapUnitsInSliceGroup0=%d;\n", MapUnitsInSliceGroup0);

    printf("---------Slice Header ref_pic_list info------------\n");
    printf("ref_pic_list_modification_flag_l0=%d;\n", ref_pic_list_modification_flag_l0);
//    printf("modification_of_pic_nums_idc=%d;\n", modification_of_pic_nums_idc);
//    printf("abs_diff_pic_num_minus1=%d;\n", abs_diff_pic_num_minus1);
//    printf("long_term_pic_num=%d;\n", long_term_pic_num);
    printf("ref_pic_list_modification_flag_l1=%d;\n", ref_pic_list_modification_flag_l1);
    
    printf("---------Slice Header pred_weight_table info------------\n");
    printf("luma_log2_weight_denom=%d;\n", luma_log2_weight_denom);
    printf("chroma_log2_weight_denom=%d;\n", chroma_log2_weight_denom);
    printf("luma_weight_l0_flag=%d;\n", luma_weight_l0_flag);
    
    printf("luma_weight_l0[0..31]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d ", luma_weight_l0[i]);
    }
    printf("\n");
    
    printf("luma_offset_l0[0..31]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d ", luma_offset_l0[i]);
    }
    printf("\n");

    printf("chroma_weight_l0_flag=%d;\n", chroma_weight_l0_flag);

    printf("chroma_weight_l0[0..31][0..1]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d:%d ", chroma_weight_l0[i][0], chroma_weight_l0[i][1]);
    }
    printf("\n");
    
    printf("chroma_offset_l0[0..31][0..1]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d:%d ", chroma_offset_l0[i][0], chroma_offset_l0[i][1]);
    }
    printf("\n");

    printf("luma_weight_l1_flag=%d;\n", luma_weight_l1_flag);
    
    printf("luma_weight_l1[0..31]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d ", luma_weight_l1[i]);
    }
    printf("\n");
    
    printf("luma_offset_l1[0..31]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d ", luma_offset_l1[i]);
    }
    printf("\n");

    printf("chroma_weight_l1_flag=%d;\n", chroma_weight_l1_flag);
    
    printf("chroma_weight_l1[0..31][0..1]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d:%d ", chroma_weight_l1[i][0], chroma_weight_l1[i][1]);
    }
    printf("\n");
    
    printf("chroma_offset_l1[0..31][0..1]: ");
    for (int i = 0; i< 32; ++i)
    {
        printf("%d:%d ", chroma_offset_l1[i][0], chroma_offset_l1[i][1]);
    }
    printf("\n");
    
    printf("---------Slice Header dec_ref_pic_marking info------------\n");
    printf("no_output_of_prior_pics_flag=%d;\n", no_output_of_prior_pics_flag);
    printf("long_term_reference_flag=%d;\n", long_term_reference_flag);
    printf("adaptive_ref_pic_marking_mode_flag=%d;\n", adaptive_ref_pic_marking_mode_flag);

    return 0;
}


int CH264SliceHeader::slice_header(CBitstream &bs, const CH264NalUnit &nal_unit, const CH264SPS spss[32], const CH264PPS ppss[256])
{
    int ret = 0;
    CH264Golomb gb;

    first_mb_in_slice = gb.get_ue_golomb(bs); //2 ue(v)
    slice_type = gb.get_ue_golomb(bs); //2 ue(v)
    pic_parameter_set_id = gb.get_ue_golomb(bs); //2 ue(v)
    
    RETURN_IF_FAILED(slice_type > 9, -1);
    RETURN_IF_FAILED(pic_parameter_set_id >= H264_MAX_PPS_COUNT, -1);
    RETURN_IF_FAILED(m_pps.seq_parameter_set_id >= H264_MAX_SPS_COUNT, -1);
    
    //--------------------
    m_nal_unit = nal_unit;
    m_pps = ppss[pic_parameter_set_id];
    m_sps = spss[m_pps.seq_parameter_set_id];

    //--------------------
    if (slice_type > 4)
    {
        slice_type -= 5;
        slice_type_fixed = 1;
    }
    else
    {
        slice_type_fixed = 0;
    }

    if (m_sps.separate_colour_plane_flag == 1)
    {
        colour_plane_id = bs.readBits(2); //2 u(2)
    }

    frame_num = bs.readBits(m_sps.log2_max_frame_num_minus4 + 4); //2 u(v)
    m_picture_coded_type = H264_PICTURE_CODED_TYPE_FRAME;

    if (!m_sps.frame_mbs_only_flag)
    {
        field_pic_flag = bs.readBits(1); //2 u(1)
        m_picture_coded_type = H264_PICTURE_CODED_TYPE_TOP_FIELD;
        if (field_pic_flag)
        {
            bottom_field_flag = bs.readBits(1); //2 u(1)
            if (bottom_field_flag)
            {
                m_picture_coded_type = H264_PICTURE_CODED_TYPE_BOTTOM_FIELD;
            }
        }
    }

    if (m_nal_unit.IdrPicFlag)
    {
        idr_pic_id = gb.get_ue_golomb(bs); //2 ue(v)
    }

    if (m_sps.pic_order_cnt_type == 0)
    {
        int32_t v = m_sps.log2_max_pic_order_cnt_lsb_minus4 + 4;
        pic_order_cnt_lsb =bs.readBits(v); //2 u(v)
        if (m_pps.bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
        {
            delta_pic_order_cnt_bottom = gb.get_se_golomb(bs); //2 se(v)
        }
    }

    if (m_sps.pic_order_cnt_type == 1 && !m_sps.delta_pic_order_always_zero_flag)
    {
        delta_pic_order_cnt[ 0 ] = gb.get_se_golomb(bs); //2 se(v)
        if (m_pps.bottom_field_pic_order_in_frame_present_flag && !field_pic_flag)
        {
            delta_pic_order_cnt[ 1 ] = gb.get_se_golomb(bs); //2 se(v)
        }
    }

    if (m_pps.redundant_pic_cnt_present_flag)
    {
        redundant_pic_cnt = gb.get_ue_golomb(bs); //2 ue(v)
    }

    if (slice_type == H264_SLIECE_TYPE_B)
    {
        direct_spatial_mv_pred_flag = bs.readBits(1); //2 u(1)
    }

    if (slice_type == H264_SLIECE_TYPE_P || slice_type == H264_SLIECE_TYPE_SP || slice_type == H264_SLIECE_TYPE_B)
    {
        num_ref_idx_active_override_flag = bs.readBits(1); //2 u(1)
        
        num_ref_idx_l0_active_minus1 = m_pps.num_ref_idx_l0_default_active_minus1;
        num_ref_idx_l1_active_minus1 = m_pps.num_ref_idx_l1_default_active_minus1;

        if (num_ref_idx_active_override_flag)
        {
            num_ref_idx_l0_active_minus1 = gb.get_ue_golomb(bs); //2 ue(v)
            if (slice_type == H264_SLIECE_TYPE_B)
            {
                num_ref_idx_l1_active_minus1 = gb.get_ue_golomb(bs); //2 ue(v)
            }
        }

        if (num_ref_idx_l0_active_minus1 >= 32)
        {
            LOG_ERROR("num_ref_idx_l0_active_minus1=%d, which should be in [0,31].\n", num_ref_idx_l0_active_minus1);
            return -1;
        }

        if (num_ref_idx_l1_active_minus1 >= 32)
        {
            LOG_ERROR("num_ref_idx_l1_active_minus1=%d, which should be in [0,31].\n", num_ref_idx_l1_active_minus1);
            return -1;
        }
    }

    if (m_nal_unit.nal_unit_type == 20 || m_nal_unit.nal_unit_type == 21)
    {
        //ret = ref_pic_list_mvc_modification(bs); // /* specified in Annex H */ 2
        LOG_ERROR("Unsupported m_nal_unit.nal_unit_type == 20 || m_nal_unit.nal_unit_type == 21;\n");
        return -1;
    }else
    {
        ret = ref_pic_list_modification(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    if ((m_pps.weighted_pred_flag && (slice_type == H264_SLIECE_TYPE_P || slice_type == H264_SLIECE_TYPE_SP)) 
        || (m_pps.weighted_bipred_idc == 1 && slice_type == H264_SLIECE_TYPE_B)
        )
    {
        ret = pred_weight_table(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    if (m_nal_unit.nal_ref_idc != 0)
    {
        ret = dec_ref_pic_marking(bs);
        RETURN_IF_FAILED(ret != 0, ret);
    }

    if (m_pps.entropy_coding_mode_flag && slice_type != H264_SLIECE_TYPE_I && slice_type != H264_SLIECE_TYPE_SI)
    {
        cabac_init_idc = gb.get_ue_golomb(bs); //2 ue(v)
    }

    slice_qp_delta = gb.get_se_golomb(bs); //2 se(v)

    if (slice_type == H264_SLIECE_TYPE_SP || slice_type == H264_SLIECE_TYPE_SI)
    {
        if (slice_type == H264_SLIECE_TYPE_SP)
        {
            sp_for_switch_flag = bs.readBits(1); //2 u(1)
        }
        slice_qs_delta = gb.get_se_golomb(bs); //2 se(v)
    }

    if (m_pps.deblocking_filter_control_present_flag)
    {
        disable_deblocking_filter_idc = gb.get_ue_golomb(bs); //2 ue(v)

        if (disable_deblocking_filter_idc != 1)
        {
            slice_alpha_c0_offset_div2 = gb.get_se_golomb(bs); //2 se(v)
            slice_beta_offset_div2 = gb.get_se_golomb(bs); //2 se(v)
        }
    }
    
    SliceGroupChangeRate = m_pps.slice_group_change_rate_minus1 + 1;

    if (m_pps.num_slice_groups_minus1 > 0 && m_pps.slice_group_map_type >= 3 && m_pps.slice_group_map_type <= 5)
    {
        int32_t temp = m_sps.PicSizeInMapUnits / SliceGroupChangeRate + 1;
        int32_t v = h264_log2(temp); // Ceil( Log2( PicSizeInMapUnits ÷ SliceGroupChangeRate + 1 ) );
        slice_group_change_cycle = bs.readBits(v); //2 u(v)
    }
    
    //-------------------
    SliceQPY = 26 + m_pps.pic_init_qp_minus26 + slice_qp_delta;
    QPY_prev = SliceQPY;
    MbaffFrameFlag = ( m_sps.mb_adaptive_frame_field_flag && !field_pic_flag );
    PicHeightInMbs = m_sps.FrameHeightInMbs / ( 1 + field_pic_flag );
    PicHeightInSamplesL = PicHeightInMbs * 16;
    PicHeightInSamplesC = PicHeightInMbs * m_sps.MbHeightC;
    PicSizeInMbs = m_sps.PicWidthInMbs * PicHeightInMbs;
    MaxPicNum = (field_pic_flag == 0) ? m_sps.MaxFrameNum : (2 * m_sps.MaxFrameNum);
    CurrPicNum = (field_pic_flag == 0) ? frame_num : (2 * frame_num + 1);
    MapUnitsInSliceGroup0 = MIN( slice_group_change_cycle * SliceGroupChangeRate, m_sps.PicSizeInMapUnits );
    QSY = 26 + m_pps.pic_init_qs_minus26 + slice_qs_delta;

    FilterOffsetA = slice_alpha_c0_offset_div2 << 1;
    FilterOffsetB = slice_beta_offset_div2 << 1;

    if (mapUnitToSliceGroupMap == NULL)
    {
        mapUnitToSliceGroupMap = (int32_t *)my_malloc(sizeof(int32_t) * m_sps.PicSizeInMapUnits);
        RETURN_IF_FAILED(mapUnitToSliceGroupMap == NULL, -1);
        memset(mapUnitToSliceGroupMap, 0, sizeof(int32_t) * m_sps.PicSizeInMapUnits);
    }
    
    if (MbToSliceGroupMap == NULL)
    {
        MbToSliceGroupMap = (int32_t *)my_malloc(sizeof(int32_t) * PicSizeInMbs);
        RETURN_IF_FAILED(MbToSliceGroupMap == NULL, -1);
        memset(MbToSliceGroupMap, 0, sizeof(int32_t) * PicSizeInMbs);
    }
    
    ret = setMapUnitToSliceGroupMap();
    RETURN_IF_FAILED(ret != 0, ret);
    
    ret = setMbToSliceGroupMap();
    RETURN_IF_FAILED(ret != 0, ret);
    
    ret = set_scaling_lists_values(m_sps, m_pps);
    RETURN_IF_FAILED(ret != 0, ret);

    //-------------------
//    int ret2 = printInfo();

    m_is_malloc_mem_self = 1;

    return 0;
}


/*
 * Page 52/74/812
 * 7.3.3.1 Reference picture list modification syntax
 */
int CH264SliceHeader::ref_pic_list_modification(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;

    if (slice_type % 5 != 2 && slice_type % 5 != 4) //H264_SLIECE_TYPE_I = 2; H264_SLIECE_TYPE_SI = 4;
    {
        ref_pic_list_modification_flag_l0 = bs.readBits(1); //2 u(1)
        if (ref_pic_list_modification_flag_l0)
        {
            int32_t i = 0;
            do
            {
                if (i >= 32)
                {
                    LOG_ERROR("modification_of_pic_nums_idc[0][i]: i=%d, must be in [0,31]\n", i);
                    return -1;
                }

                modification_of_pic_nums_idc[0][i] = gb.get_ue_golomb(bs); //2 ue(v)
                if (modification_of_pic_nums_idc[0][i] == 0 || modification_of_pic_nums_idc[0][i] == 1)
                {
                    abs_diff_pic_num_minus1[0][i] = gb.get_ue_golomb(bs); //2 ue(v)
                }
                else if (modification_of_pic_nums_idc[0][i] == 2)
                {
                    long_term_pic_num[0][i] = gb.get_ue_golomb(bs); //2 ue(v)
                }
                i++;
            } while(modification_of_pic_nums_idc[0][i - 1] != 3);
            ref_pic_list_modification_count_l0 = i;
        }
    }

    if (slice_type % 5 == 1) //H264_SLIECE_TYPE_B = 1;
    {
        ref_pic_list_modification_flag_l1 = bs.readBits(1); //2 u(1)
        if (ref_pic_list_modification_flag_l1)
        {
            int32_t i = 0;
            do
            {
                if (i >= 32)
                {
                    LOG_ERROR("modification_of_pic_nums_idc[1][i]: i=%d, must be in [0,31]\n", i);
                    return -1;
                }

                modification_of_pic_nums_idc[1][i] = gb.get_ue_golomb(bs); //2 ue(v)
                if (modification_of_pic_nums_idc[1][i] == 0 || modification_of_pic_nums_idc[1][i] == 1)
                {
                    abs_diff_pic_num_minus1[1][i] = gb.get_ue_golomb(bs); //2 ue(v)
                } else if (modification_of_pic_nums_idc[1][i] == 2)
                {
                    long_term_pic_num[1][i] = gb.get_ue_golomb(bs); //2 ue(v)
                }
                i++;
            } while (modification_of_pic_nums_idc[1][i - 1] != 3);
            ref_pic_list_modification_count_l1 = i;
        }
    }

    return 0;
}


/*
 * Page 53/75/812
 * 7.3.3.2 Prediction weight table syntax
 */
int CH264SliceHeader::pred_weight_table(CBitstream &bs)
{
    int ret = 0;
    int32_t i = 0;
    int32_t j = 0;
    CH264Golomb gb;

    luma_log2_weight_denom = gb.get_ue_golomb(bs); //2 ue(v)
    if (m_sps.ChromaArrayType != 0)
    {
        chroma_log2_weight_denom = gb.get_ue_golomb(bs); //2 ue(v)
    }

    for (i = 0; i <= num_ref_idx_l0_active_minus1; i++)
    {
        luma_weight_l0[ i ] = 1 << luma_log2_weight_denom; //When luma_weight_l0_flag is equal to 0
        luma_offset_l0[ i ] = 0; //When luma_weight_l0_flag is equal to 0

        luma_weight_l0_flag = bs.readBits(1); //2 u(1)
        if (luma_weight_l0_flag)
        {
            luma_weight_l0[ i ] = gb.get_se_golomb(bs); //2 se(v)
            luma_offset_l0[ i ] = gb.get_se_golomb(bs); //2 se(v)
        }

        if (m_sps.ChromaArrayType != 0)
        {
            chroma_weight_l0[ i ][ 0 ] = 1 << chroma_log2_weight_denom; //When chroma_weight_l0_flag is equal to 0
            chroma_weight_l0[ i ][ 1 ] = 1 << chroma_log2_weight_denom; //When chroma_weight_l0_flag is equal to 0
            chroma_offset_l0[ i ][ 0 ] = 0;
            chroma_offset_l0[ i ][ 1 ] = 0;

            chroma_weight_l0_flag = bs.readBits(1); //2 u(1)
            if (chroma_weight_l0_flag)
            {
                for (j =0; j < 2; j++)
                {
                    chroma_weight_l0[ i ][ j ] = gb.get_se_golomb(bs); //2 se(v)
                    chroma_offset_l0[ i ][ j ] = gb.get_se_golomb(bs); //2 se(v)
                }
            }
        }
    }

    if (slice_type % 5 == 1)
    {
        for (i = 0; i <= num_ref_idx_l1_active_minus1; i++)
        {
            luma_weight_l1[ i ] = 1 << luma_log2_weight_denom; //When luma_weight_l1_flag is equal to 0
            luma_offset_l1[ i ] = 0; //When luma_weight_l1_flag is equal to 0

            luma_weight_l1_flag = bs.readBits(1); //2 u(1)
            if (luma_weight_l1_flag)
            {
                luma_weight_l1[ i ] = gb.get_se_golomb(bs); //2 se(v)
                luma_offset_l1[ i ] = gb.get_se_golomb(bs); //2 se(v)
            }

            if (m_sps.ChromaArrayType != 0)
            {
                chroma_weight_l1[ i ][ 0 ] = 1 << chroma_log2_weight_denom; //When chroma_weight_l1_flag is equal to 0
                chroma_weight_l1[ i ][ 1 ] = 1 << chroma_log2_weight_denom; //When chroma_weight_l1_flag is equal to 0
                chroma_offset_l1[ i ][ 0 ] = 0;
                chroma_offset_l1[ i ][ 1 ] = 0;

                chroma_weight_l1_flag = bs.readBits(1); //2 u(1)
                if (chroma_weight_l1_flag)
                {
                    for (j = 0; j < 2; j++)
                    {
                        chroma_weight_l1[ i ][ j ] = gb.get_se_golomb(bs); //2 se(v)
                        chroma_offset_l1[ i ][ j ] = gb.get_se_golomb(bs); //2 se(v)
                    }
                }
            }
        }
    }

    return 0;
}


/*
 * Page 54/76/812
 * 7.3.3.3 Decoded reference picture marking syntax
 */
int CH264SliceHeader::dec_ref_pic_marking(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;

    if (m_nal_unit.IdrPicFlag)
    {
        no_output_of_prior_pics_flag = bs.readBits(1); //2 | 5 u(1)
        long_term_reference_flag = bs.readBits(1); //2 | 5 u(1)
    }
    else
    {
        adaptive_ref_pic_marking_mode_flag = bs.readBits(1); //2 | 5 u(1)
        if (adaptive_ref_pic_marking_mode_flag)
        {
            int32_t i = 0;
            do
            {
                if (i >= 32)
                {
                    LOG_ERROR("%s: m_dec_ref_pic_marking[i]: i=%d, must be in [0,31]\n", __FUNCTION__, i);
                    break;
                }

                m_dec_ref_pic_marking[i].memory_management_control_operation = gb.get_ue_golomb(bs); //2 | 5 ue(v)
                if (m_dec_ref_pic_marking[i].memory_management_control_operation == 1 || m_dec_ref_pic_marking[i].memory_management_control_operation == 3)
                {
                    m_dec_ref_pic_marking[i].difference_of_pic_nums_minus1 = gb.get_ue_golomb(bs); //2 | 5 ue(v)
                }
                if (m_dec_ref_pic_marking[i].memory_management_control_operation == 2)
                {
                    m_dec_ref_pic_marking[i].long_term_pic_num_2 = gb.get_ue_golomb(bs); //2 | 5 ue(v)
                }
                if (m_dec_ref_pic_marking[i].memory_management_control_operation == 3 || m_dec_ref_pic_marking[i].memory_management_control_operation == 6)
                {
                    m_dec_ref_pic_marking[i].long_term_frame_idx = gb.get_ue_golomb(bs); //2 | 5 ue(v)
                }
                if (m_dec_ref_pic_marking[i].memory_management_control_operation == 4)
                {
                    m_dec_ref_pic_marking[i].max_long_term_frame_idx_plus1 = gb.get_ue_golomb(bs); //2 | 5 ue(v)
                }
                i++;
            } while(m_dec_ref_pic_marking[i - 1].memory_management_control_operation != 0);
            dec_ref_pic_marking_count = i;
        }
    }

    return 0;
}


//8.2.2.8 Specification for conversion of map unit to slice group map to macroblock to slice group map
int CH264SliceHeader::setMbToSliceGroupMap()
{
    int32_t i = 0;

    for (i = 0; i < PicSizeInMbs; i++)
    {
        if (m_sps.frame_mbs_only_flag == 1 || field_pic_flag == 1)
        {
            MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ i ];
        }
        else if (MbaffFrameFlag == 1)
        {
            MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ i / 2 ];
        }
        else //if (m_sps.frame_mbs_only_flag == 0 && m_sps.mb_adaptive_frame_field_flag == 0 && field_pic_flag == 0)
        {
            MbToSliceGroupMap[ i ] = mapUnitToSliceGroupMap[ ( i / ( 2 * m_sps.PicWidthInMbs ) ) * m_sps.PicWidthInMbs + ( i % m_sps.PicWidthInMbs ) ];
        }
    }

    return 0;
}


//8.2.2 Decoding process for macroblock to slice group map
int CH264SliceHeader::setMapUnitToSliceGroupMap()
{
    int32_t i = 0;
    int32_t j = 0;
    int32_t k = 0;
    int32_t x = 0;
    int32_t y = 0;
    int32_t iGroup = 0;

    if (m_pps.num_slice_groups_minus1 == 0)
    {
        for ( i = 0; i < m_sps.PicSizeInMapUnits; i++ )
        {
            mapUnitToSliceGroupMap[ i ] = 0;
        }
        return 0;
    }

    if (m_pps.slice_group_map_type == 0) //8.2.2.1 Specification for interleaved slice group map type 交叉型 slice组映射类型的描述
    {
        i = 0;
        do
        {
            for ( iGroup = 0; iGroup <= m_pps.num_slice_groups_minus1 && i < m_sps.PicSizeInMapUnits; i += m_pps.run_length_minus1[ iGroup++ ] + 1 )
            {
                for ( j = 0; j <= m_pps.run_length_minus1[ iGroup ] && i + j < m_sps.PicSizeInMapUnits; j++ )
                {
                    mapUnitToSliceGroupMap[ i + j ] = iGroup;
                }
            }
        }while( i < m_sps.PicSizeInMapUnits );
    }
    else if (m_pps.slice_group_map_type == 1) //8.2.2.2 Specification for dispersed slice group map type 分散型 slice 组映射类型的描述
    {
        for ( i = 0; i < m_sps.PicSizeInMapUnits; i++ )
        {
            mapUnitToSliceGroupMap[ i ] = ( ( i % m_sps.PicWidthInMbs ) + ( ( ( i / m_sps.PicWidthInMbs ) * ( m_pps.num_slice_groups_minus1 + 1 ) ) / 2 ) ) % ( m_pps.num_slice_groups_minus1 + 1 );
        }
    }
    else if (m_pps.slice_group_map_type == 2) //8.2.2.3 Specification for foreground with left-over slice group map type 前景加剩余型 slice 组映射类型的描述
    {
        for ( i = 0; i < m_sps.PicSizeInMapUnits; i++ )
        {
            mapUnitToSliceGroupMap[ i ] = m_pps.num_slice_groups_minus1;
        }
        for ( iGroup = m_pps.num_slice_groups_minus1 - 1; iGroup >= 0; iGroup-- )
        {
            int32_t yTopLeft = m_pps.top_left[ iGroup ] / m_sps.PicWidthInMbs;
            int32_t xTopLeft = m_pps.top_left[ iGroup ] % m_sps.PicWidthInMbs;
            int32_t yBottomRight = m_pps.bottom_right[ iGroup ] / m_sps.PicWidthInMbs;
            int32_t xBottomRight = m_pps.bottom_right[ iGroup ] % m_sps.PicWidthInMbs;
            for ( y = yTopLeft; y <= yBottomRight; y++ )
            {
                for ( x = xTopLeft; x <= xBottomRight; x++ )
                {
                    mapUnitToSliceGroupMap[ y * m_sps.PicWidthInMbs + x ] = iGroup;
                }
            }
        }
    }
    else if (m_pps.slice_group_map_type == 3) //8.2.2.4 Specification for box-out slice group map types 外旋盒子型 slice 组映射类型的描述
    {
        for ( i = 0; i < m_sps.PicSizeInMapUnits; i++ )
        {
            mapUnitToSliceGroupMap[ i ] = 1;
        }
        x = ( m_sps.PicWidthInMbs - m_pps.slice_group_change_direction_flag ) / 2;
        y = ( m_sps.PicHeightInMapUnits - m_pps.slice_group_change_direction_flag ) / 2;

        int32_t leftBound = x;
        int32_t topBound = y;
        int32_t rightBound = x;
        int32_t bottomBound = y;
        int32_t xDir = m_pps.slice_group_change_direction_flag - 1;
        int32_t yDir = m_pps.slice_group_change_direction_flag;
        int32_t mapUnitVacant = 0;

        for ( k = 0; k < MapUnitsInSliceGroup0; k += mapUnitVacant )
        {
            mapUnitVacant = ( mapUnitToSliceGroupMap[ y * m_sps.PicWidthInMbs + x ] == 1 );
            if ( mapUnitVacant )
            {
                mapUnitToSliceGroupMap[ y * m_sps.PicWidthInMbs + x ] = 0;
            }
            if ( xDir == -1 && x == leftBound )
            {
                leftBound = MAX( leftBound - 1, 0 );
                x = leftBound;
                xDir = 0;
                yDir = 2 * m_pps.slice_group_change_direction_flag - 1;
            }
            else if ( xDir == 1 && x == rightBound )
            {
                rightBound = MIN( rightBound + 1, m_sps.PicWidthInMbs - 1 );
                x = rightBound;
                xDir = 0;
                yDir = 1 - 2 * m_pps.slice_group_change_direction_flag;
            }
            else if ( yDir == -1 && y == topBound )
            {
                topBound = MAX( topBound - 1, 0 );
                y = topBound;
                xDir = 1 - 2 * m_pps.slice_group_change_direction_flag;
                yDir = 0;
            }
            else if ( yDir == 1 && y == bottomBound )
            {
                bottomBound = MIN( bottomBound + 1, m_sps.PicHeightInMapUnits - 1 );
                y = bottomBound;
                xDir = 2 * m_pps.slice_group_change_direction_flag - 1;
                yDir = 0;
            }
            else
            {
                ( x, y ) = ( x + xDir, y + yDir );
            }
        }
    }
    else if (m_pps.slice_group_map_type == 4) //8.2.2.5 Specification for raster scan slice group map types 栅格扫描型 slice 组映射类型的描述
    {
        int32_t sizeOfUpperLeftGroup = 0;
        if (m_pps.num_slice_groups_minus1 == 1)
        {
            sizeOfUpperLeftGroup = ( m_pps.slice_group_change_direction_flag ? ( m_sps.PicSizeInMapUnits - MapUnitsInSliceGroup0 ) : MapUnitsInSliceGroup0 );
        }

        for ( i = 0; i < m_sps.PicSizeInMapUnits; i++ )
        {
            if ( i < sizeOfUpperLeftGroup )
            {
                mapUnitToSliceGroupMap[ i ] = m_pps.slice_group_change_direction_flag;
            }
            else
            {
                mapUnitToSliceGroupMap[ i ] = 1 - m_pps.slice_group_change_direction_flag;
            }
        }
    }
    else if (m_pps.slice_group_map_type == 5) //8.2.2.6 Specification for wipe slice group map types 擦除型 slice 组映射类型的描述
    {
        int32_t sizeOfUpperLeftGroup = 0;
        if (m_pps.num_slice_groups_minus1 == 1)
        {
            sizeOfUpperLeftGroup = ( m_pps.slice_group_change_direction_flag ? ( m_sps.PicSizeInMapUnits - MapUnitsInSliceGroup0 ) : MapUnitsInSliceGroup0 );
        }
        
        k = 0;
        for ( j = 0; j < m_sps.PicWidthInMbs; j++ )
        {
            for ( i = 0; i < m_sps.PicHeightInMapUnits; i++ )
            {
                if ( k++ < sizeOfUpperLeftGroup )
                {
                    mapUnitToSliceGroupMap[ i * m_sps.PicWidthInMbs + j ] = m_pps.slice_group_change_direction_flag;
                }
                else
                {
                    mapUnitToSliceGroupMap[ i * m_sps.PicWidthInMbs + j ] = 1 - m_pps.slice_group_change_direction_flag;
                }
            }
        }
    }
    else if (m_pps.slice_group_map_type == 6) //8.2.2.7 Specification for explicit slice group map type 显式型 slice 组映射类型的描述
    {
        for (i = 0; i < m_sps.PicSizeInMapUnits; i++)
        {
            mapUnitToSliceGroupMap[ i ] = m_pps.slice_group_id[ i ];
        }
    }
    else
    {
        LOG_ERROR("m_pps.slice_group_map_type=%d, must be in [0..6];\n", m_pps.slice_group_map_type);
        return -1;
    }

    return 0;
}


int CH264SliceHeader::set_scaling_lists_values(const CH264SPS &sps, const CH264PPS &pps)
{
    int ret = 0;
    
    //--------------------------------
    static int32_t Flat_4x4_16[16] = {16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
    static int32_t Flat_8x8_16[64] = 
    {
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
        16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    };

    //--------------------------------
    //Table 7-3 – Specification of default scaling lists Default_4x4_Intra and Default_4x4_Inter
    static int32_t Default_4x4_Intra[16] = {6, 13, 13, 20, 20, 20, 28, 28, 28, 28, 32, 32, 32, 37, 37, 42};
    static int32_t Default_4x4_Inter[16] = {10, 14, 14, 20, 20, 20, 24, 24, 24, 24, 27, 27, 27, 30, 30, 34};

    //Table 7-4 – Specification of default scaling lists Default_8x8_Intra and Default_8x8_Inter
    static int32_t Default_8x8_Intra[64] = 
    {
        6, 10, 10, 13, 11, 13, 16, 16, 16, 16, 18, 18, 18, 18, 18, 23,
        23, 23, 23, 23, 23, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27,
        27, 27, 27, 27, 29, 29, 29, 29, 29, 29, 29, 31, 31, 31, 31, 31,
        31, 33, 33, 33, 33, 33, 36, 36, 36, 36, 38, 38, 38, 40, 40, 42,
    };
    
    static int32_t Default_8x8_Inter[64] = 
    {
        9, 13, 13, 15, 13, 15, 17, 17, 17, 17, 19, 19, 19, 19, 19, 21,
        21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24,
        24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27, 27,
        27, 28, 28, 28, 28, 28, 30, 30, 30, 30, 32, 32, 32, 33, 33, 35,
    };
    
    //--------------------------------
    int32_t i = 0;
    int32_t scaling_list_size = (sps.chroma_format_idc != 3) ? 8 : 12;

    if (sps.seq_scaling_matrix_present_flag == 0 && pps.pic_scaling_matrix_present_flag == 0)
    {
        //如果编码器未给出缩放矩阵值，则缩放矩阵值全部默认为16
        for (i = 0; i < scaling_list_size; i++)
        {
            if (i < 6)
            {
                memcpy(ScalingList4x4[i], Flat_4x4_16, sizeof(int32_t) * 16);
            }
            else //if (i >= 6)
            {
                memcpy(ScalingList8x8[i - 6], Flat_8x8_16, sizeof(int32_t) * 64);
            }
        }
    }
    else
    {
        if (sps.seq_scaling_matrix_present_flag == 1)
        {
            for (i = 0; i < scaling_list_size; i++)
            {
                if (i < 6)
                {
                    if (sps.seq_scaling_list_present_flag[i] == 0) //参照 Table 7-2 Scaling list fall-back rule A
                    {
                        if (i == 0)
                        {
                            memcpy(ScalingList4x4[i], Default_4x4_Intra, sizeof(int32_t) * 16);
                        }
                        else if (i == 3)
                        {
                            memcpy(ScalingList4x4[i], Default_4x4_Inter, sizeof(int32_t) * 16);
                        }
                        else
                        {
                            memcpy(ScalingList4x4[i], ScalingList4x4[i - 1], sizeof(int32_t) * 16);
                        }
                    }
                    else
                    {
                        if (sps.UseDefaultScalingMatrix4x4Flag[i] == 1)
                        {
                            if (i < 3)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Intra, sizeof(int32_t) * 16);
                            }
                            else //if (i >= 3)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Inter, sizeof(int32_t) * 16);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList4x4[i], sps.ScalingList4x4[i], sizeof(int32_t) * 16); //采用编码器传送过来的量化系数的缩放值
                        }
                    }
                }
                else //if (i >= 6)
                {
                    if (sps.seq_scaling_list_present_flag[i] == 0) //参照 Table 7-2 Scaling list fall-back rule A
                    {
                        if (i == 6)
                        {
                            memcpy(ScalingList8x8[i - 6], Default_8x8_Intra, sizeof(int32_t) * 64);
                        }
                        else if (i == 7)
                        {
                            memcpy(ScalingList8x8[i - 6], Default_8x8_Inter, sizeof(int32_t) * 64);
                        }
                        else
                        {
                            memcpy(ScalingList8x8[i - 6], ScalingList8x8[i - 8], sizeof(int32_t) * 64);
                        }
                    }
                    else
                    {
                        if (sps.UseDefaultScalingMatrix8x8Flag[i - 6] == 1)
                        {
                            if (i == 6 || i== 8 || i== 10)
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Intra, sizeof(int32_t) * 64);
                            }
                            else
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Inter, sizeof(int32_t) * 64);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList8x8[i - 6], sps.ScalingList8x8[i - 6], sizeof(int32_t) * 64); //采用编码器传送过来的量化系数的缩放值
                        }
                    }
                }
            }
        }
        
        //注意：此处不是"else if"，意即pps里面的值，可能会覆盖之前sps得到的值
        if (pps.pic_scaling_matrix_present_flag == 1)
        {
            for (i = 0; i < scaling_list_size; i++)
            {
                if (i < 6)
                {
                    if (pps.pic_scaling_list_present_flag[i] == 0) //参照 Table 7-2 Scaling list fall-back rule B
                    {
                        if (i == 0)
                        {
                            if (sps.seq_scaling_matrix_present_flag == 0)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Intra, sizeof(int32_t) * 16);
                            }
                        }
                        else if (i == 3)
                        {
                            if (sps.seq_scaling_matrix_present_flag == 0)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Inter, sizeof(int32_t) * 16);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList4x4[i], ScalingList4x4[i - 1], sizeof(int32_t) * 16);
                        }
                    }
                    else
                    {
                        if (pps.UseDefaultScalingMatrix4x4Flag[i] == 1)
                        {
                            if (i < 3)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Intra, sizeof(int32_t) * 16);
                            }
                            else //if (i >= 3)
                            {
                                memcpy(ScalingList4x4[i], Default_4x4_Inter, sizeof(int32_t) * 16);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList4x4[i], sps.ScalingList4x4[i], sizeof(int32_t) * 16); //采用编码器传送过来的量化系数的缩放值
                        }
                    }
                }
                else //if (i >= 6)
                {
                    if (pps.pic_scaling_list_present_flag[i] == 0) //参照 Table 7-2 Scaling list fall-back rule B
                    {
                        if (i == 6)
                        {
                            if (sps.seq_scaling_matrix_present_flag == 0)
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Intra, sizeof(int32_t) * 64);
                            }
                        }
                        else if (i == 7)
                        {
                            if (sps.seq_scaling_matrix_present_flag == 0)
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Inter, sizeof(int32_t) * 64);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList8x8[i - 6], ScalingList8x8[i - 8], sizeof(int32_t) * 64);
                        }
                    }
                    else
                    {
                        if (pps.UseDefaultScalingMatrix8x8Flag[i - 6] == 1)
                        {
                            if (i == 6 || i== 8 || i== 10)
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Intra, sizeof(int32_t) * 64);
                            }
                            else
                            {
                                memcpy(ScalingList8x8[i - 6], Default_8x8_Inter, sizeof(int32_t) * 64);
                            }
                        }
                        else
                        {
                            memcpy(ScalingList8x8[i - 6], sps.ScalingList8x8[i - 6], sizeof(int32_t) * 64); //采用编码器传送过来的量化系数的缩放值
                        }
                    }
                }
            }
        }
    }

    return ret;
}


bool CH264SliceHeader::is_first_VCL_NAL_unit_of_a_picture(const CH264SliceHeader &lastSliceHeader)
{
    int ret = 0;

    ret |= (this->pic_parameter_set_id != lastSliceHeader.pic_parameter_set_id);

    ret |= (this->frame_num != lastSliceHeader.frame_num);

    ret |= (this->field_pic_flag != lastSliceHeader.field_pic_flag);

    if (this->field_pic_flag && lastSliceHeader.field_pic_flag)
    {
        ret |= (this->bottom_field_flag != lastSliceHeader.bottom_field_flag);
    }

    ret |= (this->m_nal_unit.nal_ref_idc != lastSliceHeader.m_nal_unit.nal_ref_idc) && ((this->m_nal_unit.nal_ref_idc == 0) || (lastSliceHeader.m_nal_unit.nal_ref_idc == 0));
    ret |= (this->m_nal_unit.IdrPicFlag != lastSliceHeader.m_nal_unit.IdrPicFlag);

    if (this->m_nal_unit.IdrPicFlag && lastSliceHeader.m_nal_unit.IdrPicFlag)
    {
        ret |= (this->idr_pic_id != lastSliceHeader.idr_pic_id);
    }

    if (this->m_sps.pic_order_cnt_type == 0)
    {
        ret |= (this->pic_order_cnt_lsb != lastSliceHeader.pic_order_cnt_lsb);
        if (this->m_pps.bottom_field_pic_order_in_frame_present_flag == 1 && this->field_pic_flag == 0)
        {
            ret |= (this->delta_pic_order_cnt_bottom != lastSliceHeader.delta_pic_order_cnt_bottom);
        }
    }

    if (this->m_sps.pic_order_cnt_type == 1)
    {
        if (!this->m_sps.delta_pic_order_always_zero_flag)
        {
            ret |= (this->delta_pic_order_cnt[0] != lastSliceHeader.delta_pic_order_cnt[0]);
            if (this->m_pps.bottom_field_pic_order_in_frame_present_flag == 1 && this->field_pic_flag == 0)
            {
                ret |= (this->delta_pic_order_cnt[1] != lastSliceHeader.delta_pic_order_cnt[1]);
            }
        }
    }
    
    return (ret == 0) ? false : true;
}

