#pragma once


#define WM_DECODER_THREAD_END    (WM_USER + 100)


#include "H264VideoDecoder.h" //H264视频解码器


//typedef int(_stdcall* PlayCallback)(CH264Picture *outPicture, void *userData, int errorCode);
typedef int (* PlayCallback)(CH264Picture *outPicture, void *userData, int errorCode);


class CMyVideoPlayer
{
public:
    char m_filename[600];

    CH264VideoDecoder m_H264VideoDecoder;

    int m_need_decoded_total_frames; //设置总共需要解码多少帧(默认为0，即无限制) 若按25fps计算，一天24小时总帧数 = 24 * 60 * 60 * 25 = 2160000（帧）

    DWORD m_thread_id_play_process;
    HANDLE m_handle_thread_process;

    PlayCallback m_callback;
    void * m_user_data;

    HANDLE m_handle_event; //事件句柄

    bool m_isForceEnd; //是否强制结束
    bool m_isPause; //是否暂停
    bool m_is_thread_end; //线程是否结束

    HWND m_hwnd_parent;

public:
    CMyVideoPlayer();
    ~CMyVideoPlayer();

    int SetHwndParent(HWND hwnd_parent);

    int SetFrameCallback(PlayCallback callback, void * user_data); //设置回调函数

    int player_open(const char *url);
    int player_close();
    int player_pause(bool isPause);

    int begin_thread();

    int get_video_info(CH264Picture *outPicture);

    static unsigned long __stdcall ThreadFunc_For_PlayProcess(void *lpParam);
    
    static int my_output_frame_callback(CH264Picture *outPicture, void *userData, int errorCode);
};

