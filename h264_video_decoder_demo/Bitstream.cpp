//
// Bitstream.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "Bitstream.h"
#include "CommonFunction.h"


CBitstream::CBitstream()
{
    int32_t ret = init();
}


CBitstream::CBitstream(uint8_t *buffer, int32_t bufferSize)
{
    m_start = buffer;
    m_p = buffer;
    m_end = buffer + bufferSize - 1;
    m_bits_left = 8;
}


CBitstream::~CBitstream()
{

}


int32_t CBitstream::init()
{
    m_start = NULL;
    m_end = NULL;
    m_p = NULL;
    m_bits_left = 0;

    return 0;
}


bool CBitstream::isEnd()
{
    return (m_p == m_end) && (m_bits_left == 0);
}


int32_t CBitstream::readOneBit()
{
    int32_t ret = 0;

    m_bits_left--;

    if (!isEnd())
    {
        ret = ((*(m_p)) >> m_bits_left) & 0x01;
    }

    if (m_bits_left == 0 && m_p < m_end)
    {
        m_p++;
        m_bits_left = 8;
    }

    return ret;
}


int32_t CBitstream::skipOneBit()
{
    int32_t ret = 0;

    m_bits_left--;
    if (m_bits_left == 0 && m_p < m_end)
    {
        m_p++;
        m_bits_left = 8;
    }

    return ret;
}


int32_t CBitstream::readBits(int32_t n)
{
    int32_t ret = 0;

    for (int32_t i = 0; i < n; ++i)
    {
        ret |= (readOneBit() << (n - i - 1));
    }
    return ret;
}


int32_t CBitstream::getBits(int32_t n)
{
    int32_t ret = 0;

    uint8_t * p2 = m_p;
    int32_t bits_left2 = m_bits_left;

    ret = readBits(n);

    m_p = p2;
    m_bits_left = bits_left2;

    return ret;
}


int32_t CBitstream::skipBits(int32_t n)
{
    int32_t ret = 0;

    for (int32_t i = 0; i < n; ++i)
    {
        ret = skipOneBit();
    }
    return ret;
}
