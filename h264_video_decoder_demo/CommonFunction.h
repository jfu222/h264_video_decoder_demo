//
// CommonFunction.h
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#ifndef __COMMON_FUNCTION_H__
#define __COMMON_FUNCTION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string>
#include <vector>


void *my_malloc(size_t size);
void my_free(void *ptr);


#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

//Ceil( x ) the smallest integer greater than or equal to x.
#define CEIL(x)    (int(x))

#define CLIP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define CLIP3(x, y, z)  (((z) < (x)) ? (x) : (((z) > (y)) ? (y) : (z)))
#define ROUND(x) ((int) ((x) + 0.5))
#define ABS(x) ((int) (((x) >= (0)) ? (x) : (-(x))))

#define SAFE_FREE(x)    do { if (x){ my_free(x); x = NULL; } } while(0)
#define SAFE_DELETE(x)    do { if (x){ delete (x); x = NULL; } } while(0)
#define SAFE_DELETE_ARRAY(x)    do { if (x){ delete [] (x); x = NULL; } } while(0)


#define RETURN_IF_FAILED(condition, ret)                                                      \
    do                                                                                        \
    {                                                                                         \
        if (condition)                                                                        \
        {                                                                                     \
            printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
            return ret;                                                                       \
        }                                                                                     \
    } while (0)

int RETURN_IF_FAILED3(int condition, int ret);

#define NOT_RETURN_IF_FAILED(condition, ret)                                                      \
    do                                                                                        \
    {                                                                                         \
        if (condition)                                                                        \
        {                                                                                     \
            printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
        }                                                                                     \
    } while (0)

#define BREAK_IF_FAILED(condition)                                                        \
    if (condition)                                                                        \
    {                                                                                     \
        printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
        break;                                                                            \
    }

#define CONTINUE_IF_FAILED(condition)                                                        \
    if (condition)                                                                        \
    {                                                                                     \
        printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);    \
        continue;                                                                            \
    }


#define    LOG_INFO(pszFormat, ...)     printf("[INFO] %s:(%d) %s: "pszFormat, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define    LOG_ERROR(pszFormat, ...)    printf("[ERR] %s:(%d) %s: "pszFormat, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define    LOG_WARN(pszFormat, ...)     printf("[WARN] %s:(%d) %s: "pszFormat, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)


typedef enum _PAYLOAD_TYPE_
{
    PAYLOAD_TYPE_UNKNOWN = 0,
    PAYLOAD_TYPE_ETHERNET_II,
    PAYLOAD_TYPE_IP,
    PAYLOAD_TYPE_TCP,
    PAYLOAD_TYPE_UDP,
    PAYLOAD_TYPE_RTSP,
    PAYLOAD_TYPE_RTP,
    PAYLOAD_TYPE_RTCP,
    PAYLOAD_TYPE_VIDEO_H264,
    PAYLOAD_TYPE_VIDEO_H265,
    PAYLOAD_TYPE_AUDIO_PCMU,
    PAYLOAD_TYPE_SDP,
    PAYLOAD_TYPE_RTMP,
    PAYLOAD_TYPE_HTTP,
    PAYLOAD_TYPE_WEBSOCKET,
}PAYLOAD_TYPE;


//-----------------------------
int getFileDirnameAndBasenameAndExtname(const char *fileName, std::string &dirName, std::string &baseName, std::string &extensionName);
int createNestedDir(const char *dir); //创建嵌套目录

#endif //__COMMON_FUNCTION_H__
