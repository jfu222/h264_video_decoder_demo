//
// H264SEI.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_SEI_H__
#define __H264_SEI_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Bitstream.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 47/69/812
 * 7.3.2.3 Supplemental enhancement information RBSP syntax
 * #define    H264_NAL_SEI                  6
 */
class CH264SEI
{
public:
    int32_t     last_payload_type_byte; // 5 u(8)
    int32_t     last_payload_size_byte; // 5 u(8)
    int32_t     payloadType;
    int32_t     payloadSize;

public:
    CH264SEI();
    ~CH264SEI();
    
    int printInfo();
    
    int sei_rbsp(CBitstream &bs);
    int sei_message(CBitstream &bs);
    int sei_payload(CBitstream &bs, int32_t payloadType,int32_t payloadSize);
};

#endif //__H264_SEI_H__
