//
// H264HrdParameters.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264HrdParameters.h"
#include "H264Golomb.h"
#include "CommonFunction.h"


CHrdParameters::CHrdParameters()
{
    cpb_cnt_minus1 = 0;
    bit_rate_scale = 0;
    cpb_size_scale = 0;
    memset(bit_rate_value_minus1, 0, sizeof(int32_t) * H264_MAX_CPB_CNT);
    memset(cpb_size_value_minus1, 0, sizeof(int32_t) * H264_MAX_CPB_CNT);
    memset(cbr_flag, 0, sizeof(int32_t) * H264_MAX_CPB_CNT);
    initial_cpb_removal_delay_length_minus1 = 0;
    cpb_removal_delay_length_minus1 = 0;
    dpb_output_delay_length_minus1 = 0;
    time_offset_length = 0;
}


CHrdParameters::~CHrdParameters()
{

}


int CHrdParameters::printInfo()
{
    printf("---------CHrdParameters info------------\n");
    printf("cpb_cnt_minus1=%d;\n", cpb_cnt_minus1);
    printf("bit_rate_scale=%d;\n", bit_rate_scale);
    printf("cpb_size_scale=%d;\n", cpb_size_scale);

    printf("bit_rate_value_minus1[0..31]: ");
    for (int i = 0; i < H264_MAX_CPB_CNT; ++i)
    {
        printf("%d ", bit_rate_value_minus1[i]);
    }
    printf("\n");

    printf("cpb_size_value_minus1[0..31]: ");
    for (int i = 0; i < H264_MAX_CPB_CNT; ++i)
    {
        printf("%d ", cpb_size_value_minus1[i]);
    }
    printf("\n");

    printf("cbr_flag[0..31]: ");
    for (int i = 0; i < H264_MAX_CPB_CNT; ++i)
    {
        printf("%d ", cbr_flag[i]);
    }
    printf("\n");

    printf("initial_cpb_removal_delay_length_minus1=%d;\n", initial_cpb_removal_delay_length_minus1);
    printf("cpb_removal_delay_length_minus1=%d;\n", cpb_removal_delay_length_minus1);
    printf("dpb_output_delay_length_minus1=%d;\n", dpb_output_delay_length_minus1);
    printf("time_offset_length=%d;\n", time_offset_length);

    return 0;
}


// E.1.2 HRD parameters syntax
int CHrdParameters::hrd_parameters(CBitstream &bs)
{
    int ret = 0;
    CH264Golomb gb;

    this->cpb_cnt_minus1 = gb.get_ue_golomb(bs); //0 | 5 ue(v)
    this->bit_rate_scale = bs.readBits(4); //0 | 5 u(4)
    this->cpb_size_scale = bs.readBits(4); //0 | 5 u(4)
    for (int32_t SchedSelIdx = 0; SchedSelIdx <= this->cpb_cnt_minus1; SchedSelIdx++)
    {
        this->bit_rate_value_minus1[SchedSelIdx] = gb.get_ue_golomb(bs); //0 | 5 ue(v)
        this->cpb_size_value_minus1[SchedSelIdx] = gb.get_ue_golomb(bs); //0 | 5 ue(v)
        this->cbr_flag[SchedSelIdx] = bs.readBits(1); //0 | 5 u(1)
    }
    this->initial_cpb_removal_delay_length_minus1 = bs.readBits(5); //0 | 5 u(5)
    this->cpb_removal_delay_length_minus1 = bs.readBits(5); //0 | 5 u(5)
    this->dpb_output_delay_length_minus1 = bs.readBits(5); //0 | 5 u(5)
    this->time_offset_length = bs.readBits(5); //0 | 5 u(5)

    return 0;
}
