//
// H264CommonFunc.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "H264CommonFunc.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Bitstream.h"
#include "H264Golomb.h"


int h264_log2(int32_t value)
{
    assert(value > 0);
    int log2 = 0;
    while(value)
    {
        value >>= 1;
        log2++;
    }
    return log2;
}


int32_t h264_power2(int32_t value)
{
    int32_t power2 = 1;
    for (int32_t i = 0; i < value; ++i)
    {
        power2 *= 2;
    }
    return power2;
//    return 1 << value;
}


/*
 * T-REC-H.264-201704-S!!PDF-E.pdf
 * Page 44/66/812
 * 7.3.2.1.1.1 Scaling list syntax
 */
int scaling_list(CBitstream &bs, int32_t *scalingList, int sizeOfScalingList, int32_t &useDefaultScalingMatrixFlag)
{
    int32_t lastScale = 8;
    int32_t nextScale = 8;
    CH264Golomb gb;

    for (int j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            int delta_scale = gb.get_se_golomb(bs); //delta_scale 0 | 1 se(v)
            nextScale = (lastScale + delta_scale + 256) % 256;
            useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
        }
        //FIXE: What meaning 'When useDefaultScalingMatrixFlag is derived to be equal to 1, 
        //the scaling list shall be inferred to be equal to the default scaling list as specified in Table 7-2.'
        scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
        lastScale = scalingList[j];
    }
    return 0;
}


int more_rbsp_data(CBitstream &bs)
{
    // If there is no more data in the RBSP, the return value of more_rbsp_data( ) is equal to FALSE.
    if (bs.m_p > bs.m_end
        || (bs.m_p == bs.m_end && bs.m_bits_left == 0)
        )
    {
        return 0;
    }

    // Otherwise, the RBSP data is searched for the last (least significant, right-most) bit equal to 1 that is present in the
    // RBSP. Given the position of this bit, which is the first bit (rbsp_stop_one_bit) of the rbsp_trailing_bits( ) syntax
    // structure, the following applies:
    // If there is more data in an RBSP before the rbsp_trailing_bits( ) syntax structure, the return value of
    // more_rbsp_data( ) is equal to TRUE.
    // Otherwise, the return value of more_rbsp_data( ) is equal to FALSE.
    uint8_t * p1 = bs.m_end;
    while(p1 > bs.m_p && *p1 == 0) //从后往前找，直到找到第一个非0值字节位置为止
    {
        p1--;
    }

    if (p1 > bs.m_p)
    {
        return 1; //说明当前位置bs.m_p后面还有码流数据
    }
    else //if (p1 == bs.m_p)
    {
        int flag = 0;
        int i = 0;
        for (i = 0; i < 8; i++) //在单个字节的8个比特位中，从后往前找，找到rbsp_stop_one_bit位置
        {
            int v = ((*(bs.m_p)) >> i) & 0x01;
            if (v == 1)
            {
                i++;
                flag = 1;
                break;
            }
        }

        if (flag == 1 && i < bs.m_bits_left)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    return 1;
}


int more_rbsp_trailing_data(CBitstream &bs)
{
    // If there is more data in an RBSP, the return value of more_rbsp_trailing_data( ) is equal to TRUE.
    // Otherwise, the return value of more_rbsp_trailing_data( ) is equal to FALSE.
    if (bs.m_p < bs.m_end)
    {
        return 1;
    }
    return 0;
}


int byte_aligned(CBitstream &bs)
{
    // If the current position in the bitstream is on a byte boundary, 
    // i.e., the next bit in the bitstream is the first bit in a byte, 
    // the return value of byte_aligned( ) is equal to TRUE.
    // Otherwise, the return value of byte_aligned( ) is equal to FALSE.
    if (bs.m_bits_left % 8 == 0)
    {
        return 1;
    }
    return 0;
}


int more_data_in_byte_stream(CBitstream &bs)
{
    // If more data follow in the byte stream, the return value of more_data_in_byte_stream( ) is equal to TRUE.
    // Otherwise, the return value of more_data_in_byte_stream( ) is equal to FALSE.
    if (bs.m_p < bs.m_end)
    {
        return 1;
    }
    return 0;
}


int rbsp_trailing_bits(CBitstream &bs)
{
    if (bs.m_p >= bs.m_end)
    {
        return 0;
    }

    int32_t rbsp_stop_one_bit = bs.readBits(1); // /* equal to 1 */ All f(1)
    while(!byte_aligned(bs))
    {
        int32_t rbsp_alignment_zero_bit = bs.readBits(1);; // /* equal to 0 */ All f(1)
    }
    return 0;
}


int access_unit_delimiter_rbsp(CBitstream &bs)
{
    int ret = 0;
    int32_t primary_pic_type = bs.readBits(3); //6 u(3)
    ret = rbsp_trailing_bits(bs);
    return 0;
}


int end_of_seq_rbsp(CBitstream &bs)
{
    return 0;
}


int end_of_stream_rbsp(CBitstream &bs)
{
    return 0;
}


int filler_data_rbsp(CBitstream &bs)
{
    int ret = 0;
    
    while( bs.getBits( 8 ) == 0xFF ) // while( next_bits( 8 ) == 0xFF )
        int32_t ff_byte = bs.readBits(8); // /* equal to 0xFF */ 9 f(8)
    ret = rbsp_trailing_bits(bs);
    return 0;
}


/*
 * Page 49/71/812
 * 7.3.2.10 RBSP slice trailing bits syntax
 */
int rbsp_slice_trailing_bits(CBitstream &bs, int32_t entropy_coding_mode_flag)
{
    int ret = 0;
    ret = rbsp_trailing_bits(bs); // All
    if (entropy_coding_mode_flag)
    {
        while(more_rbsp_trailing_data(bs))
        {
            int32_t cabac_zero_word = bs.readBits(16); // /* equal to 0x0000 */ All f(16)
        }
    }

    return 0;
}


int InverseRasterScan(int32_t a, int32_t b, int32_t c, int32_t d, int32_t e)
{
    int ret = 0;

    if (e == 0)
    {
        ret = (a % (d / b) ) * b;
    }
    else //if (e == 1)
    {
        ret = (a / (d / b) ) * c;
    }
    return ret;
}
