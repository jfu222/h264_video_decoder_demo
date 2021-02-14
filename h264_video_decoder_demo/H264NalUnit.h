//
// H264NalUnit.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __H264_NAL_UNIT_H__
#define __H264_NAL_UNIT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "H264CommonFunc.h"


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 42/64/812
 * 7.3.1 NAL unit syntax
 */
class CH264NalUnit
{
public:
    int32_t     forbidden_zero_bit; // All f(1)
    int32_t     nal_ref_idc; // All u(2)
    int32_t     nal_unit_type; // All u(5)
    int32_t     svc_extension_flag; // All u(1)
    int32_t     avc_3d_extension_flag; // All u(1)
    int32_t     emulation_prevention_three_byte; // /* equal to 0x03 */ All f(8)
    uint8_t *   rbsp_byte; //my_malloc(NumBytesInNALunit);    Raw byte sequence payloads.
    int32_t     NumBytesInNALunit;
    int32_t     NumBytesInRBSP;
    int32_t     IdrPicFlag; //是否是立即刷新帧的标记
    int32_t     m_is_malloc_mem_self;

public:
    CH264NalUnit();
    ~CH264NalUnit();
    
    int printInfo();

    int init(int numBytesInNALunit);
    int unInit();
    CH264NalUnit & operator = (const CH264NalUnit &src); //重载等号运算符
    int copyData(const CH264NalUnit &src, bool isMallocAndCopyData);
    int getH264RbspFromNalUnit(unsigned char *srcData, int srcSize);
};

#endif //__H264_NAL_UNIT_H__
