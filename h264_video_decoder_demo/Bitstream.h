//
// Bitstream.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __BIT_STREAM_H__
#define __BIT_STREAM_H__

#include <stdio.h>
#include <stdint.h>


class CBitstream
{
public:
    uint8_t * m_start;
    uint8_t * m_end;
    uint8_t * m_p;
    int32_t m_bits_left;

public:
    CBitstream();
    CBitstream(uint8_t *buffer, int32_t bufferSize);
    ~CBitstream();

    int32_t init();

    bool isEnd();
    int32_t readOneBit();
    int32_t skipOneBit();
    int32_t readBits(int32_t n);
    int32_t getBits(int32_t n);
    int32_t skipBits(int32_t n);
};

#endif //__BIT_STREAM_H__
