//
// H264SVCVUIEx.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264SVCVUIExt.h"
#include "H264Golomb.h"
#include "Bitstream.h"
#include "CommonFunction.h"


CH264SVCVUIExt::CH264SVCVUIExt()
{
    vui_ext_num_entries_minus1 = 0;

    memset(vui_ext_dependency_id, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_quality_id, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_temporal_id, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_timing_info_present_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_num_units_in_tick, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_time_scale, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_fixed_frame_rate_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_nal_hrd_parameters_present_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_vcl_hrd_parameters_present_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_low_delay_hrd_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
    memset(vui_ext_pic_struct_present_flag, 0, sizeof(int32_t) * VUI_EXT_MAX_NUM_ENTRIES);
}


CH264SVCVUIExt::~CH264SVCVUIExt()
{

}


int CH264SVCVUIExt::printInfo()
{
    printf("---------SVC-VUI-Ext------------\n");
    printf("vui_ext_num_entries_minus1=%d;\n", vui_ext_num_entries_minus1);

    for (int32_t i = 0; i <= vui_ext_num_entries_minus1; ++i)
    {
        printf("vui_ext_dependency_id[%d]=%d; ", i, vui_ext_dependency_id[i]);
        printf("vui_ext_quality_id[%d]=%d; ", i, vui_ext_quality_id[i]);
        printf("vui_ext_temporal_id[%d]=%d; ", i, vui_ext_temporal_id[i]);
        printf("vui_ext_timing_info_present_flag[%d]=%d; ", i, vui_ext_timing_info_present_flag[i]);
        printf("vui_ext_num_units_in_tick[%d]=%d; ", i, vui_ext_num_units_in_tick[i]);
        printf("vui_ext_time_scale[%d]=%d; ", i, vui_ext_time_scale[i]);
        printf("vui_ext_fixed_frame_rate_flag[%d]=%d; ", i, vui_ext_fixed_frame_rate_flag[i]);
        printf("vui_ext_nal_hrd_parameters_present_flag[%d]=%d; ", i, vui_ext_nal_hrd_parameters_present_flag[i]);
        printf("vui_ext_vcl_hrd_parameters_present_flag[%d]=%d; ", i, vui_ext_vcl_hrd_parameters_present_flag[i]);
        printf("vui_ext_low_delay_hrd_flag[%d]=%d; ", i, vui_ext_low_delay_hrd_flag[i]);
        printf("vui_ext_pic_struct_present_flag[%d]=%d; ", i, vui_ext_pic_struct_present_flag[i]);
        printf("\n");
    }

    int ret2 = m_vui_ext_hrd_parameter_nal.printInfo();
    int ret3 = m_vui_ext_hrd_parameter_vcl.printInfo();

    return 0;
}


int CH264SVCVUIExt::svc_vui_parameters_extension(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;

    this->vui_ext_num_entries_minus1 = gb.get_ue_golomb(bs); //0 ue(v)
    for (int32_t i = 0; i <= vui_ext_num_entries_minus1; i++)
    {
        this->vui_ext_dependency_id[i] = bs.readBits(3); //0 u(3)
        this->vui_ext_quality_id[i] = bs.readBits(4); //0 u(4)
        this->vui_ext_temporal_id[i]= bs.readBits(3); // 0 u(3)
        this->vui_ext_timing_info_present_flag[i] = bs.readBits(1); //0 u(1)
        if (this->vui_ext_timing_info_present_flag[i])
        {
            this->vui_ext_num_units_in_tick[i] = bs.readBits(32); //0 u(32)
            this->vui_ext_time_scale[i]= bs.readBits(32); // 0 u(32)
            this->vui_ext_fixed_frame_rate_flag[i] = bs.readBits(1); //0 u(1)
        }
        this->vui_ext_nal_hrd_parameters_present_flag[i] = bs.readBits(1); //0 u(1)
        if (this->vui_ext_nal_hrd_parameters_present_flag[i])
        {
            ret = m_vui_ext_hrd_parameter_nal.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        this->vui_ext_vcl_hrd_parameters_present_flag[i] = bs.readBits(1); //0 u(1)
        if (this->vui_ext_vcl_hrd_parameters_present_flag[i])
        {
            ret = m_vui_ext_hrd_parameter_vcl.hrd_parameters(bs);
            RETURN_IF_FAILED(ret != 0, ret);
        }
        if (this->vui_ext_nal_hrd_parameters_present_flag[i] || vui_ext_vcl_hrd_parameters_present_flag[i])
        {
            this->vui_ext_low_delay_hrd_flag[i] = bs.readBits(1); //0 u(1)
        }
        this->vui_ext_pic_struct_present_flag[i] = bs.readBits(1); //0 u(1)
    }
    
    int ret2 = printInfo();

    return ret;
}
