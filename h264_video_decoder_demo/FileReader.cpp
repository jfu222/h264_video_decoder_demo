//
// FileReader.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "FileReader.h"
#include "Bitstream.h"
#include "CommonFunction.h"

#define    BUFFER_SIZE    10 * 1024 * 1024    // 10 MB


CFileReader::CFileReader()
{
    m_filename = "";
    m_fp = NULL;
    m_file_offset = 0;
    m_buffer_size = 0;
    m_buffer_start = NULL;
    m_buffer_end = NULL;
    m_buffer_pointer_pos = NULL;
}


CFileReader::~CFileReader()
{
    int ret = unInit();
}


int CFileReader::init(const char *filename)
{
    int ret = 0;
    
    RETURN_IF_FAILED(filename == NULL, -1);
    
    m_filename = filename;

    m_fp = fopen(filename, "rb");
    RETURN_IF_FAILED(m_fp == NULL, -1);
    
    m_buffer_size = BUFFER_SIZE;
    m_buffer_start = (unsigned char *)my_malloc(m_buffer_size);
    RETURN_IF_FAILED(m_buffer_start == NULL, -1);
    memset(m_buffer_start, 0, m_buffer_size);

    m_buffer_pointer_pos = m_buffer_start;
    m_buffer_end = NULL;

    return ret;
}


int CFileReader::unInit()
{
    int ret = 0;
    
    if (m_buffer_start)
    {
        my_free(m_buffer_start);
        m_buffer_start = NULL;
    }
    
    if (m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }
    
    return ret;
}


int CFileReader::getNextStartCode(unsigned char *data, int size, unsigned char *&pos1, unsigned char *&pos2,int &startCodeLength1,int &startCodeLength2)
{
    int ret = -1;

    startCodeLength1 = 0;
    startCodeLength2 = 0;

    unsigned char * p1 = data;
    unsigned char * p2 = p1;
    unsigned char * p3 = data + size;
    
    while (p2 <= p3 - 3)
    {
        if (p2[0] == 0x00 && p2[1] == 0x00 && p2[2] == 0x01)
        {
            pos1 = p2;
            startCodeLength1 = 3;
            if (p2 - 1 >= p1 && *(p2 - 1) == 0x00)
            {
                pos1 = p2 - 1;
                startCodeLength1 = 4;
            }
            p2 += startCodeLength1;
            ret = 1;
            break;
        }
        p2++;
    }

    while (p2 <= p3 - 3)
    {
        if (p2[0] == 0x00 && p2[1] == 0x00 && p2[2] == 0x01)
        {
            pos2 = p2;
            startCodeLength2 = 3;
            if (p2 - 1 >= p1 && *(p2 - 1) == 0x00)
            {
                pos2 = p2 - 1;
                startCodeLength2 = 4;
            }
            ret = 2;
            break;
        }
        p2++;
    }

    return ret;
}


int CFileReader::getNextH264NalUnitByStartCode(unsigned char *&data, int &size)
{
    int ret = 0;
    
    if (m_buffer_end == NULL)
    {
        size_t readBytes = fread(m_buffer_start, 1, m_buffer_size, m_fp);
        if (readBytes <= 0)
        {
            LOG_ERROR("fread(): readBytes=%d;\n", readBytes);
            return -1;
        }
        m_file_offset += readBytes;
        m_buffer_end = m_buffer_start + readBytes;
    }

    //确保m_buffer_pointer_pos[0..3]一定是00 00 00 01起始码
    while(1)
    {
        unsigned char * pos1 = NULL;
        unsigned char * pos2 = NULL;
        int startCodeLength1 = 0;
        int startCodeLength2 = 0;

        ret = getNextStartCode(m_buffer_pointer_pos, m_buffer_end - m_buffer_pointer_pos + 1, pos1, pos2, startCodeLength1, startCodeLength2);
        if (ret == 2) //在m_buffer_pointer_pos[0..end]中，从左到右找到了两个连续的起始码，则结束查找（即m_buffer_start[]数组必须能够容纳尺寸最大的I帧）
        {
            data = pos1 + startCodeLength1;
            size = pos2 - pos1 - startCodeLength1;
            m_buffer_pointer_pos = data + size;

            ret = 0;
            break;
        }
        else if (ret == 1) //在m_buffer_pointer_pos[0..end]中，从左到右只找到了第一个起始码
        {
            // 将00 00 00 01起始码以及之后的数据，向前移动，保持与m_buffer_start[0]对齐
            if (pos1 > m_buffer_pointer_pos)
            {
                //类似 memmove(m_buffer_start, pos1, m_buffer_end - (pos1 - m_buffer_start) + 1);
                for (int i = 0; i < m_buffer_end - pos1 + 1; ++i)
                {
                    m_buffer_start[i] = pos1[i];
                }
                m_buffer_end -= (pos1 - m_buffer_start) + 1;
                m_buffer_pointer_pos = m_buffer_start;
            }

            int left_bytes = m_buffer_size - (m_buffer_end - m_buffer_pointer_pos + 2);
            if (left_bytes > 0)
            {
                size_t readBytes = fread(m_buffer_end + 1, 1, left_bytes, m_fp); //从文件中读取数据
                if (readBytes <= 0) //说明已经读到文件末尾了
                {
                    pos2 = m_buffer_start + m_file_offset;

                    data = pos1 + startCodeLength1;
                    size = pos2 - pos1 - startCodeLength1;
                    m_buffer_pointer_pos = data + size;

                    long file_pos = ftell(m_fp);
                    LOG_ERROR("fread(): readBytes=%d; left_bytes=%d; file_pos=%d;\n", readBytes, left_bytes, file_pos);

                    ret = 0;
                    break;
                }
                m_file_offset += readBytes;
                m_buffer_end = m_buffer_start + m_buffer_size - 1;
            }
            else
            {
                m_buffer_end = m_buffer_start;
                LOG_ERROR("This nal unit is too large. 1111: m_buffer_size=%d; m_file_offset=%d;\n", m_buffer_size, m_file_offset);
            }
        }
        else //在m_buffer_start[0..end]中，从左到右未找到一个起始码
        {
            m_buffer_end = m_buffer_start;
            LOG_ERROR("Cannot found start code [00 00 01]. 2222: m_buffer_size=%d; m_file_offset=%d;\n", m_buffer_size, m_file_offset);
            return -1;
        }
    }

    return ret;
}
