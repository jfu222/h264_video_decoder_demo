//
// H264SPSSVCExt.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SVC_H__
#define __H264_SVC_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"
#include "CommonFunction.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 423/445/812
 * G.7.3.2.1.4 Sequence parameter set SVC extension syntax
 * SVC(Scalable video coding)
 */
class CH264SPSSVCExt
{
public:
    int32_t     inter_layer_deblocking_filter_control_present_flag; //0 u(1)
    int32_t     extended_spatial_scalability_idc; //0 u(2)
    int32_t     chroma_phase_x_plus1_flag; //0 u(1)
    int32_t     chroma_phase_y_plus1; //0 u(2)
    int32_t     seq_ref_layer_chroma_phase_x_plus1_flag; //0 u(1)
    int32_t     seq_ref_layer_chroma_phase_y_plus1; //0 u(2)
    int32_t     seq_scaled_ref_layer_left_offset; //0 se(v)
    int32_t     seq_scaled_ref_layer_top_offset; //0 se(v)
    int32_t     seq_scaled_ref_layer_right_offset; //0 se(v)
    int32_t     seq_scaled_ref_layer_bottom_offset; //0 se(v)
    int32_t     seq_tcoeff_level_prediction_flag; //0 u(1)
    int32_t     adaptive_tcoeff_level_prediction_flag; //0 u(1)
    int32_t     slice_header_restriction_flag; //0 u(1)

public:
    CH264SPSSVCExt();
    ~CH264SPSSVCExt();

    int printInfo();
    
    int seq_parameter_set_svc_extension(CBitstream &bs, int ChromaArrayType);
    int svc_vui_parameters_extension(CBitstream &bs);
};

#endif //__H264_SVC_H__
