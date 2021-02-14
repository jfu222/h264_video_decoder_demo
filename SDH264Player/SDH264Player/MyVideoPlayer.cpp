#include "StdAfx.h"
#include "MyVideoPlayer.h"
#include <stdio.h>


CMyVideoPlayer::CMyVideoPlayer()
{
    m_thread_id_play_process = NULL;
    m_handle_thread_process = NULL;

    m_callback = NULL;
    m_user_data = NULL;
    m_handle_event = NULL;

    m_isForceEnd = false;
    m_isPause = false;
    m_is_thread_end = true;

    m_hwnd_parent = NULL;
}


CMyVideoPlayer::~CMyVideoPlayer()
{
    if (m_handle_event != NULL)
    {
        BOOL bRet = ::CloseHandle(m_handle_event);
    }
}


int CMyVideoPlayer::SetHwndParent(HWND hwnd_parent)
{
    m_hwnd_parent = hwnd_parent;

    return 0;
}


int CMyVideoPlayer::SetFrameCallback(PlayCallback callback, void * user_data)
{
    m_callback = callback;
    m_user_data = user_data;
    
    int ret= m_H264VideoDecoder.set_output_frame_callback_functuin(my_output_frame_callback, this); //设置回调函数

    return ret;
}


int CMyVideoPlayer::player_open(const char *url)
{
    int ret = 0;

    m_isForceEnd = false;
    m_isPause = false;
    m_is_thread_end = true;

    if (m_is_thread_end == true)
    {
        //--------------------------------------
        m_handle_event = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_handle_event == NULL)
        {
            return -104;
        }

        _snprintf_s(m_filename, 600, "%s", url); //strlen(url) < 600
    }
    else
    {
        if (!SetEvent(m_handle_event)) //set thread start event 可能是用户点击了暂停按钮，然后点击了开始按钮
        {
            printf("Error: set start event failed, errno = %ld\n", GetLastError());
            return -1;
        }
    }

end:
    if (ret != 0)
    {
        ret = player_close();
        if (ret != 0){ printf("Error: %s: player_close() failed!\n", __FUNCTION__); return -3; }
        return -4;
    }

    return 0;
}


int CMyVideoPlayer::player_close()
{
    int ret = 0;

    m_isForceEnd = true;
    BOOL bRet = SetEvent(m_handle_event); //发送信号

    while (1)
    {
        if (m_is_thread_end == true){ break; }
        Sleep(100);
    }

    return ret;
}


int CMyVideoPlayer::player_pause(bool isPause)
{
    if (m_isPause == isPause){ return -1; }

    if (m_isPause == true) //已经处于暂停状态了
    {
        BOOL bRet = SetEvent(m_handle_event); //发送信号
        m_isPause = false;
    }
    else
    {
        m_isPause = true;
    }

    return 0;
}


int CMyVideoPlayer::begin_thread()
{
    int ret = 0;

    if (m_is_thread_end == true)
    {
        m_is_thread_end = false;

        //----------------------------------------
        m_handle_thread_process = CreateThread(NULL, 0, ThreadFunc_For_PlayProcess, (LPVOID)this, 0, &m_thread_id_play_process);
        if (m_handle_thread_process == NULL)
        {
            return -105;
        }
    }

    return ret;
}


int CMyVideoPlayer::get_video_info(CH264Picture *outPicture)
{
    return 0;
}


unsigned long __stdcall CMyVideoPlayer::ThreadFunc_For_PlayProcess(void *lpParam)
{
    printf("CMyVideoPlayer::ThreadFunc_For_PlayProcess(): st\n");

    CMyVideoPlayer * p = (CMyVideoPlayer *)lpParam;

    //----------------------------------------
    int ret = 0;

    ret = p->m_H264VideoDecoder.open(p->m_filename); //注意：该open函数，内部是一个循环，并且和CMyVideoPlayer::my_output_frame_callback()是同一个线程
    printf("p->m_H264VideoDecoder.open(url): ret=%d;\n", ret);

end:
    printf("CMyVideoPlayer::ThreadFunc_For_PlayProcess(): ed: +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

    p->m_is_thread_end = true;

    ::SendMessageW(p->m_hwnd_parent, WM_DECODER_THREAD_END, ret, 0);

    return ret;
}


int CMyVideoPlayer::my_output_frame_callback(CH264Picture *outPicture, void *userData, int errorCode)
{
    int ret = 0;
    
    CMyVideoPlayer * p = (CMyVideoPlayer *)userData;
    
    //----------------------------------------
    int cnt = 0;
    int err_state = 0;
    unsigned long long time_to_refresh = 0; //刷新解码的时间到了
    unsigned long long last_time_to_refresh = 0;
    unsigned long long duration = 0;
    unsigned long long org_duration = 0;
    float current_set_play_speed = 1.0;
    
    while (1)
    {
        if (p->m_isForceEnd == true) //外部强制结束解码
        {
            ret = -1; //返回非0值，表示用户设置解码结束
            break;
        }

        if (p->m_isPause == true) //外部暂停解码
        {
            WaitForSingleObject(p->m_handle_event, INFINITE); //等待
            time_to_refresh = GetTickCount();
        }

        if (time_to_refresh == 0)
        {
            if (errorCode != 0)
            {
                if (p->m_callback)
                {
                    p->m_callback(outPicture, p->m_user_data, H264_DECODE_ERROR_CODE_FILE_END); //视频解码完毕(或其他未知错误)
                }
                break; //跳出循环
            }

            duration = 1000 / outPicture->m_picture_frame.m_h264_slice_header.m_sps.fps; //40 ms
            if (duration < 0)
            {
                duration = 40;
            }

            org_duration = duration;
            duration = (unsigned long long)((float)duration * current_set_play_speed);
            if (last_time_to_refresh == 0)
            {
                time_to_refresh = GetTickCount();
            }
            else
            {
                time_to_refresh = last_time_to_refresh + duration;
            }
        }

        if (time_to_refresh > 0 && GetTickCount() > time_to_refresh)
        {
            if (p->m_callback)
            {
                p->m_callback(outPicture, p->m_user_data, 0); //正常回调
                break;
            }
            else //发生错误
            {
                break;
            }

            last_time_to_refresh = time_to_refresh;
            time_to_refresh = 0;
            if (duration < 10){ continue; }
        }

        cnt++;

        Sleep(1);
    }

    return ret;
}

