//
// CommonFunction.cpp
// h264_video_decoder_demo
//
// Created by: 386520874@qq.com
// Date: 2019.09.01 - 2021.02.14
//

#include "CommonFunction.h"

#if defined(_WIN32) || defined(_WIN64)
#include <Shlwapi.h>
#else

#endif


int RETURN_IF_FAILED3(int condition, int ret)
{
    if (condition)
    {
        printf("%s(%d): %s: Error: ret=%d;\n", __FILE__, __LINE__, __FUNCTION__, ret);
        return ret;
    }
    return 0;
}


void *my_malloc(size_t size)
{
    void * ptr = malloc(size);
    //printf("%s: size=%ld; ptr=%p;\n", __FUNCTION__, size, ptr);
    return ptr;
}


void my_free(void *ptr)
{
    //printf("%s: ptr=%p;\n", __FUNCTION__, ptr);
    return free(ptr);
}


int getFileDirnameAndBasenameAndExtname(const char *fileName, std::string &dirName, std::string &baseName, std::string &extensionName)
{
    if (fileName == NULL || strlen(fileName) <= 0)
    {
        return -1;
    }

    std::string filename = fileName;

    size_t pos1 = filename.rfind('.');

#if defined(_WIN32) || defined(_WIN64)
    size_t pos2 = filename.rfind('\\');
    size_t pos21 = filename.rfind('/');
    if (pos2 != std::string::npos && pos21 != std::string::npos)
    {
        if (pos21 > pos2)
        {
            pos2 = pos21;
        }
    }
    else if (pos2 == std::string::npos && pos21 != std::string::npos)
    {
        pos2 = pos21;
    }
#else
    size_t pos2 = filename.rfind('/');
#endif

    if (pos1 == std::string::npos)
    {
        extensionName = "";
    }else
    {
        extensionName = filename.substr(pos1 + 1);
    }
    
    if (pos2 == std::string::npos)
    {
        dirName = "";
        baseName = filename;
    }else
    {
        dirName = filename.substr(0, pos2);
        baseName = filename.substr(pos2 + 1);
    }

    return 0;
}

#if defined(_WIN32) || defined(_WIN64)
int createNestedDir(const char *dir) //创建嵌套目录
{
    if (strlen(dir) >= 600)
    {
        return -1;
    }

    char dir1[600] = { 0 };
    char dir2[600] = { 0 };
    int flag = 0;
    BOOL bRet = FALSE;

    strcpy(dir1, dir);

    int len = strlen(dir1);
    if (len <= 3) // 如果类似 "D:\" 则返回错误
    {
        return -1;
    }

    for (int i = 0; i < len; ++i)
    {
        if (dir1[i] == '/')
        {
            dir1[i] = '\\';
        }
    }

    if (dir1[len - 1] != '\\')
    {
        dir1[len] = '\\';
        dir1[len + 1] = '\0';
        len++;
    }

    for (int i = 0; i < len; ++i)
    {
        if (dir1[i] == '\\')
        {
            if (flag == 1)
            {
                bRet = PathFileExistsA(dir2);
                if (bRet == FALSE)
                {
                    bRet = CreateDirectoryA(dir2, NULL);
                    if (bRet == FALSE)
                    {
//                        DWORD error = GetLastError(); //ERROR_ALREADY_EXISTS
                        printf("%s: Cannot create dir '%s'\n", __FUNCTION__, dir2);
                        return -1;
                    }
//                    bRet = SetFileAttributes(dir2, FILE_ATTRIBUTE_NORMAL);
                }
            }
            dir2[i] = dir1[i];
            flag = 1;
        }
        else
        {
            dir2[i] = dir1[i];
        }
    }

    return 0;
}
#else
int createNestedDir(const char *dir) //创建嵌套目录
{
    if (strlen(dir) >= 600)
    {
        return -1;
    }

    char dir1[600] = { 0 };
    char dir2[600] = { 0 };
    int flag = 0;
    int bRet = 0;

    strcpy(dir1, dir);

    int len = strlen(dir1);

    for (int i = 0; i < len; ++i)
    {
        if (dir1[i] == '\\')
        {
            dir1[i] = '/';
        }
    }

    if (dir1[len - 1] != '/')
    {
        dir1[len] = '/';
        dir1[len + 1] = '\0';
        len++;
    }

    for (int i = 0; i < len; ++i)
    {
        if (dir1[i] == '/')
        {
            if (flag == 1)
            {
                bRet = access(dir2, F_OK);
                if (bRet != 0) //文件不存在
                {
                    bRet = mkdir(dir2, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    if (bRet != 0)
                    {
                        printf("%s: Cannot create dir '%s'\n", __FUNCTION__, dir2);
                        return -1;
                    }
                }
            }
            dir2[i] = dir1[i];
            flag = 1;
        }
        else
        {
            dir2[i] = dir1[i];
        }
    }

    return 0;
}
#endif

