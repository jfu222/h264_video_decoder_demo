//
// H264SPS3DavcExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SPS_3D_AVC_EXT_H__
#define __H264_SPS_3D_AVC_EXT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 739/761/812
 * J.7.3.2.1.5 Sequence parameter set 3D-AVC extension syntax
 */
class CH264SPS3DavcExt
{
public:
    int32_t     m_3dv_acquisition_idc; //0 ue(v)

public:
    CH264SPS3DavcExt();
    ~CH264SPS3DavcExt();

    int printInfo();
    
    int seq_parameter_set_3davc_extension(CBitstream &bs);
};

#endif //__H264_SPS_3D_AVC_EXT_H__
