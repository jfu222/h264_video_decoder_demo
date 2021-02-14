//
// H264SPSExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SPS_EXT_H__
#define __H264_SPS_EXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 45/67/812
 * 7.3.2.1.2 Sequence parameter set extension RBSP syntax
 * #define    H264_NAL_SPS_EXT              13
 */
class CH264SPSExt
{
public:
    int32_t     profile_idc; //0 u(8)
    int32_t     seq_parameter_set_id; //10 ue(v)
    int32_t     aux_format_idc; //10 ue(v)
    int32_t     bit_depth_aux_minus8; //10 ue(v)
    int32_t     alpha_incr_flag; //10 u(1)
    int32_t     alpha_opaque_value; //10 u(v)    v = bit_depth_aux_minus8 + 9
    int32_t     alpha_transparent_value; //10 u(v)    v = bit_depth_aux_minus8 + 9
    int32_t     additional_extension_flag; //10 u(1)

public:
    CH264SPSExt();
    ~CH264SPSExt();

    int printInfo();
    
    int seq_parameter_set_extension_rbsp(CBitstream &bs);
};

#endif //__H264_SPS_EXT_H__
