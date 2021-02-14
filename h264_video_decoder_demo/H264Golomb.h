//
// H264Golomb.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_GOLOMB_H__
#define __H264_GOLOMB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "CommonFunction.h"
#include "H264CommonFunc.h"
#include "Bitstream.h"


typedef enum
{
    CODE_BLOCK_PATTERN_UNKNOWN,
    CODE_BLOCK_PATTERN_INTRA_4x4,
    CODE_BLOCK_PATTERN_INTRA_8x8,
    CODE_BLOCK_PATTERN_INTRA_INTER,
}CODE_BLOCK_PATTERN;


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 208/230/812
 * 9.1 Parsing process for Exp-Golomb codes
 * ¸çÂ×²¼±àÂë
 */
class CH264Golomb
{
public:

public:
    CH264Golomb();
    ~CH264Golomb();

    int get_ue_golomb(CBitstream &bs);
    int get_se_golomb(CBitstream &bs);
    int get_me_golomb(CBitstream &bs, int32_t ChromaArrayType, H264_MB_PART_PRED_MODE MbPartPredMode);
    int get_te_golomb(CBitstream &bs, int range);
};

#endif //__H264_GOLOMB_H__
