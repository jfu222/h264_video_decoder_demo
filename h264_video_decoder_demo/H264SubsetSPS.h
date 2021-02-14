//
// H264SubsetSPS.h
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
#include "H264SPS.h"
#include "H264SVCVUIExt.h"
#include "H264SPSSVCExt.h"
#include "H264SPSMVCExt.h"
#include "H264MVCVUIExt.h"
#include "H264SPSMVCDExt.h"
#include "H264SPS3DavcExt.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 45/67/812
 * 7.3.2.1.3 Subset sequence parameter set RBSP syntax
 * #define    H264_NAL_SUB_SPS              15
 */
class CH264SubsetSPS
{
public:
    int32_t     svc_vui_parameters_present_flag; //0 u(1)
    int32_t     bit_equal_to_one; // /* equal to 1 */ 0 f(1)
    int32_t     mvc_vui_parameters_present_flag; //0 u(1)
    int32_t     additional_extension2_flag; //0 u(1)
    int32_t     additional_extension2_data_flag; //0 u(1)

    CH264SPS          m_sps;
    CH264SPSSVCExt    m_sps_svc_ext;
    CH264SVCVUIExt    m_svc_vui_ext;
    CH264SPSMVCExt    m_sps_mvc_ext;
    CH264MVCVUIExt    m_mvc_vui_ext;
    CH264SPSMVCDExt   m_sps_mvcd_ext;
    CH264SPS3DavcExt  m_sps_3d_avc_ext;

public:
    CH264SubsetSPS();
    ~CH264SubsetSPS();

    int printInfo();
    
    int subset_seq_parameter_set_rbsp(CBitstream &bs);
};

#endif //__H264_SPS_EXT_H__
