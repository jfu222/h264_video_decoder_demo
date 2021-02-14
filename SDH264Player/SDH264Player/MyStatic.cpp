// CMyStatic.cpp : 实现文件
//

#include "stdafx.h"
#include "MyStatic.h"
#include "resource.h"


#define DX_SCALE 0.1f //滚轮每滚一次图像缩放比例间隔
#define DY_SCALE 0.1f //滚轮每滚一次图像缩放比例间隔


// CMyStatic

IMPLEMENT_DYNAMIC(CMyStatic, CStatic)

CMyStatic::CMyStatic()
{
    m_total_num_frames = 0;
    m_player.SetFrameCallback(H264VideoPlayerFrameCallback, this);

    memset(&m_bitmap, 0, sizeof(BITMAP));
    memset(&m_bitmap_copy, 0, sizeof(BITMAP));
    m_hbitmap = NULL;

    m_guid_index = 2; //默认输出png
    m_hWnd = NULL;
    m_zoom_scale = 1.0f;
    m_zoom_scale_will = 1.0f;

    memset(&m_st_rect_scale, 0, sizeof(KRectScale));

    m_bLButtonDown = FALSE;

    m_pt_mouse_move.x = 0;
    m_pt_mouse_move.y = 0;

    m_mouse_move_last_x = -1;
    m_mouse_move_last_y = -1;
    m_mouse_move_delta_x = 0;
    m_mouse_move_delta_y = 0;

    m_mouse_scale_point_x = 0;
    m_mouse_scale_point_y = 0;

    m_Is_Popup_Right_Menu = TRUE;
    m_Draw_Mode = DRAW_MODE_NONE;
    m_Draw_Shape = DRAW_SHAPE_RECT;

    m_max_points_num = 3; //最多能画3个点

    m_pAnotherClass = NULL;

    memset(&m_rect_render, 0, sizeof(RECT));

    m_IsAcceptDropFiles = TRUE;

    m_file_path_open = _T("");

    m_color_background.r = 0;
    m_color_background.g = 0;
    m_color_background.b = 0;

    m_color_draw_shape.r = 255;
    m_color_draw_shape.g = 0;
    m_color_draw_shape.b = 0;

    m_currMbAddr = -1;
}

CMyStatic::~CMyStatic()
{
    if (m_hbitmap != NULL)
    {
        ::DeleteObject(m_hbitmap);
        m_hbitmap = NULL;
    }

    m_outPicture.m_picture_frame.m_pic_buff_luma = NULL; //因为是其他结构体申请的内存，为避免二次释放内存，此处需要置空
}


//-----------播放器相关-----------------
int CMyStatic::SetHwndParent(HWND hwnd_parent)
{
    this->m_player.SetHwndParent(hwnd_parent);

    return 0;
}


int CMyStatic::start_play(const char *url)
{
    int ret = m_player.player_open(url);
    if (ret != 0)
    {
        this->MessageBox(_T("打开文件失败"), _T("错误"), MB_OK);
        return ret;
    }

    m_frame_num_now = 0;

    ret = m_player.begin_thread();

    return ret;
}


int CMyStatic::stop_play()
{
    int ret = 0;

    ret = m_player.player_close();

    return ret;
}


int CMyStatic::pause_play(bool isPause)
{
    int ret = 0;

    ret = m_player.player_pause(isPause);

    return ret;
}


//-----------------------------------------------------
//------------ 视频播放控制函数 -----------------------
//-----------------------------------------------------
int CMyStatic::H264VideoPlayerFrameCallback(CH264Picture *outPicture, void *userData, int errorCode)
{
    int ret = 0;
    CMyStatic * player = (CMyStatic *)userData;

    if (errorCode == H264_DECODE_ERROR_CODE_FILE_END) //视频文件达到文件末尾
    {
        return 0; //返回1表示阻塞
    }
    else if (errorCode == -1) //视频文件未知错误(一般是视频解码错误)
    {
        return 0; //返回1表示阻塞
    }

    HWND hwnd = player->m_hWnd;
    if (!::IsWindow(hwnd))
    {
        return 1;
    }
    
    //----------------yuv420p到brg24的格式转换-------------------------
    int W = outPicture->m_picture_frame.PicWidthInSamplesL;
    int H = outPicture->m_picture_frame.PicHeightInSamplesL;

    if (player->m_bitmap.bmBits == NULL)
    {
        ret = player->CreateEmptyImage(player->m_bitmap, W, H, 24);
        
        ret = player->CenterVideo();
    }

    ret = outPicture->m_picture_frame.convertYuv420pToBgr24FlipLines(W, H, outPicture->m_picture_frame.m_pic_buff_luma, 
                                            (unsigned char *)player->m_bitmap.bmBits, player->m_bitmap.bmWidthBytes);
    if (ret != 0)
    {
        return -1;
    }
    
    if (player->m_outPicture.m_picture_frame.m_is_malloc_mem_by_myself == 0)
    {
        player->m_outPicture.m_picture_frame.m_picture_coded_type = H264_PICTURE_CODED_TYPE_FRAME;
        player->m_outPicture.m_picture_frame.init(outPicture->m_picture_frame.m_h264_slice_header); //申请内存
        player->m_outPicture.m_picture_frame.m_is_malloc_mem_by_myself = 1;
    }

    //player->m_outPicture = *outPicture;
    int32_t copyMbsDataFlag = 1;
    ret = player->m_outPicture.m_picture_frame.copyData2(outPicture->m_picture_frame, copyMbsDataFlag);

    if (player->m_hWnd)
    {
        player->Invalidate(0);
        //player->UpdateWindow(); //该API会阻塞回调线程
    }

    //---------设置帧信息----------------
    CString frame_info = _T("");
    CString profile_str = _T("");
    int & profile_idc = outPicture->m_picture_frame.m_h264_slice_header.m_sps.profile_idc;
    int & level_idc = outPicture->m_picture_frame.m_h264_slice_header.m_sps.level_idc;
    
    switch (outPicture->m_picture_frame.m_h264_slice_header.m_sps.profile_idc)
    {
    case 66:  { profile_str.Format(_T("H264 Baseline (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 77:  { profile_str.Format(_T("H264 Main (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 88:  { profile_str.Format(_T("H264 Extended (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 100: { profile_str.Format(_T("H264 High (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 110: { profile_str.Format(_T("H264 High 10 (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 122: { profile_str.Format(_T("H264 High 4:2:2 (Intra) (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    case 244: { profile_str.Format(_T("H264 High 4:4:4 (Predictive/Intra) (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    default:  { profile_str.Format(_T("H264 Unkown (profile=%d, level=%.1f)"), profile_idc, level_idc / 10.0); break; }
    }

    //-------------------------------------------
    frame_info.Format(_T("第 %d/%d 帧  |  %s 帧  |  分辨率：%dx%d  |  fps：%.2f  |  帧场自适应模式：%s  |  %s  | %s"), player->m_frame_num_now, player->m_total_num_frames, 
        H264_SLIECE_TYPE_TO_STR(outPicture->m_picture_frame.m_h264_slice_header.slice_type), 
        outPicture->m_picture_frame.PicWidthInSamplesL, outPicture->m_picture_frame.PicHeightInSamplesL,
        outPicture->m_picture_frame.m_h264_slice_header.m_sps.fps,
        outPicture->m_picture_frame.m_h264_slice_header.MbaffFrameFlag ? _T("是") : _T("不是"),
        outPicture->m_picture_frame.m_h264_slice_header.m_pps.entropy_coding_mode_flag ? _T("cabac") : _T("cavlc"),
        profile_str);

    player->GetParent()->GetDlgItem(IDC_STATIC3)->SetWindowText(frame_info);

    player->m_frame_num_now++;

    return 0; //返回1表示阻塞
}


//鼠标击中测试
int CMyStatic::mb_hit_test(POINT pt, CH264Picture *Pic, CH264MacroBlock &mb)
{


    return 1;
}


//----------------------------
int CMyStatic::SetAnotherClass(CMyStatic * p)
{
    m_pAnotherClass = p;

    return 1;
}

int CMyStatic::GetAnotherClass(CMyStatic * p)
{
    p = m_pAnotherClass;

    return 1;
}


//设置渲染区域(即感兴趣区域)
int CMyStatic::image_set_render_rect(int left, int top, int right, int bottom)
{
    int x = min(left, right);
    int y = min(top, bottom);
    int w = abs(left - right);
    int h = abs(top - bottom);

    m_rect_render.left = x;
    m_rect_render.top = y;
    m_rect_render.right = m_rect_render.left + w;
    m_rect_render.bottom = m_rect_render.top + h;

    return 1;
}


//设置渲染区域(即感兴趣区域)
int CMyStatic::image_set_render_rect_by_ROI()
{
    int cnt = m_vector_draw_shapes.size();
    int flag = 0;

    if (cnt != 0)
    {
        int cnt2 = m_vector_draw_shapes[0].vector_point.size();
        if (cnt2 == 2 && m_vector_draw_shapes[0].draw_shape == DRAW_SHAPE_RECT)
        {
            image_set_render_rect(m_vector_draw_shapes[0].vector_point[0].x, m_vector_draw_shapes[0].vector_point[0].y, m_vector_draw_shapes[0].vector_point[1].x, m_vector_draw_shapes[0].vector_point[1].y);
            flag = 1;
        }
    }

    if (flag == 0)
    {
        image_set_render_rect(0, 0, m_bitmap.bmWidth - 1, m_bitmap.bmHeight - 1);
    }

    return 1;
}


//=================MFC消息机制==================================

BEGIN_MESSAGE_MAP(CMyStatic, CStatic)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_MOUSEWHEEL()
    ON_WM_RBUTTONDOWN()
    ON_WM_RBUTTONUP()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_SIZE()
    ON_WM_LBUTTONDBLCLK()
    ON_WM_SHOWWINDOW()
    ON_WM_DROPFILES()
    ON_WM_CHAR()
    ON_WM_GETDLGCODE()
    ON_WM_KEYDOWN()
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CMyStatic 消息处理程序


//----------------计算矩形交集------------------------------
/* 函数名称：CalcIntersectRect()
 * 函数功能：计算两个矩形交集，以便StretchBlt(...)调用
 * 算法描述：
 *              A +---------------+ B
 *                |               |
 *              J |               | K
 *        E +-----|---------------|-------------+ F
 *          |     |               |             |
 *          |     |    + M        |             |
 *          |     |               |             |
 *          |   C +---------------+ D           |
 *          |                                   |
 *          |                                   |
 *        G +-----------------------------------+ H
 *
 *    如图"矩形ABCD"和"矩形EFGH"的交集为"矩形JKCD"，其中，
 * "矩形ABCD"为窗口客户区矩形，"矩形EFGH"为原始位图缩放后
 * 的矩形，"矩形JKCD"是需要求取的交集矩形。
 * 计算过程为：
 *             S = S_will / S_now
 *             AE = AM + ME = AM + S * ME' (矢量加法)
 *             EJ = EM + MJ = EM + S * MJ' (矢量加法)
 *             E''J'' = EJ / S_will
 * 其他点坐标以此类推。
 *
 * 说明:
 *      (1) A点为所有点的坐标原点
 *      (2) M点为缩放的中心，即该点缩放前后保持不动,
 *          也即鼠标滚轮滚动时的坐标点
 *      (3) E''J''为缩放后，交集矩形的左上角反向映射
 *          回原始位图中的点
 *      (4) KRectScale.src 对应 "矩形JKCD" 缩放前的矩形
 *      (5) KRectScale.dst 对应 "矩形ABCD"
 *      (6) KRectScale.org_left_top 对应 AE矢量
 */
int CMyStatic::CalcIntersectRect()
{
    CRect rcClient;
    this->GetClientRect(&rcClient);

    if (m_zoom_scale < 0.1f){ m_zoom_scale = 0.1f; }

    float s = m_zoom_scale_will / m_zoom_scale;

    m_st_rect_scale.org_left_top.x = m_mouse_scale_point_x + s * (m_st_rect_scale.org_left_top.x - m_mouse_scale_point_x);
    m_st_rect_scale.org_left_top.y = m_mouse_scale_point_y + s * (m_st_rect_scale.org_left_top.y - m_mouse_scale_point_y);

    //-----------------------------------------
    RECT rc1 = { 0 };
    RECT rc2 = { 0 };
    RECT rc3 = { 0 };

    rc1.left = 0;
    rc1.right = rc1.left + rcClient.Width();
    rc1.top = 0;
    rc1.bottom = rc1.top + rcClient.Height();

    rc2.left = m_st_rect_scale.org_left_top.x;
    rc2.right = rc2.left + m_bitmap.bmWidth * m_zoom_scale_will;
    rc2.top = m_st_rect_scale.org_left_top.y;
    rc2.bottom = rc2.top + m_bitmap.bmHeight * m_zoom_scale_will;

    ::IntersectRect(&rc3, &rc1, &rc2); //求两个矩形的交集

    m_st_rect_scale.dst = rc3;

    m_st_rect_scale.src.left = (m_st_rect_scale.dst.left - m_st_rect_scale.org_left_top.x) / m_zoom_scale_will;
    m_st_rect_scale.src.right = (m_st_rect_scale.dst.right - m_st_rect_scale.org_left_top.x) / m_zoom_scale_will;

    m_st_rect_scale.src.top = (m_st_rect_scale.dst.top - m_st_rect_scale.org_left_top.y) / m_zoom_scale_will;
    m_st_rect_scale.src.bottom = (m_st_rect_scale.dst.bottom - m_st_rect_scale.org_left_top.y) / m_zoom_scale_will;

    m_zoom_scale = m_zoom_scale_will;

    return 1;
}


//----------------将视频画面调整到屏幕中央(必要时缩放到屏幕窗口大小)------------------------------
int CMyStatic::CenterVideo()
{
    if (m_bitmap.bmWidth <= 0 || m_bitmap.bmHeight <= 0)
    {
        return -1;
    }

    CRect rcClient;
    this->GetClientRect(&rcClient);

    int x;
    int y;

    float scale_x = 1.0 * rcClient.Width() / m_bitmap.bmWidth;
    float scale_y = 1.0 * rcClient.Height() / m_bitmap.bmHeight;

    float scale = min(scale_x, scale_y);

    m_zoom_scale = 1.0f;
    m_zoom_scale_will = scale;

    //------------------------------------------
    if (scale == scale_x)
    {
        x = 0;
        y = (rcClient.Height() - m_bitmap.bmHeight * scale) / 2; //视频居中
    }
    else
    {
        x = (rcClient.Width() - m_bitmap.bmWidth * scale) / 2; //视频居中
        y = 0;
    }

    //------------------------------------------
    m_mouse_scale_point_x = x;
    m_mouse_scale_point_y = y;

    m_st_rect_scale.org_left_top.x = x;
    m_st_rect_scale.org_left_top.y = y;

    CalcIntersectRect();

    return 1;
}


//将窗口客户区坐标转换成原始图片坐标
int CMyStatic::ConvertPt2Pt(POINT in, POINT &out)
{
    out.x = (in.x - m_st_rect_scale.org_left_top.x) / m_zoom_scale;
    out.y = (in.y - m_st_rect_scale.org_left_top.y) / m_zoom_scale;

    return 1;
}


//设置画线的参数
int CMyStatic::SetDrawShapeParam(KDraw_Shape draw_shape, std::vector<POINT> vector_point)
{
    //m_Draw_Shape = draw_shape;
    //m_vector_point = vector_point;

    return 1;
}


//获取画线的参数
int CMyStatic::GetDrawShapeParam(KDraw_Shape &draw_shape, std::vector<POINT> &vector_point)
{
    //draw_shape = m_Draw_Shape;
    //vector_point = m_vector_point;

    return 1;
}


void CMyStatic::PreSubclassWindow()
{
    // TODO: 在此添加专用代码和/或调用基类

    CStatic::PreSubclassWindow();
}


void CMyStatic::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CStatic::OnChar(nChar, nRepCnt, nFlags);
}


void CMyStatic::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    CStatic::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CMyStatic::OnTimer(UINT_PTR nIDEvent)
{
    CStatic::OnTimer(nIDEvent);
}


UINT CMyStatic::OnGetDlgCode()
{
    UINT res = CStatic::OnGetDlgCode();
    res |= DLGC_WANTALLKEYS | DLGC_WANTCHARS;
    return res;

    //return CStatic::OnGetDlgCode();
}


BOOL CMyStatic::PreTranslateMessage(MSG* pMsg)
{
    // TODO: 在此添加专用代码和/或调用基类

    return CStatic::PreTranslateMessage(pMsg);
}


int CMyStatic::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CStatic::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码
    //this->ModifyStyle(0, SS_NOTIFY); //如果是对话框模板资源控件，则对话框不会调用OnCreate(...)

    return 0;
}


void CMyStatic::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: 在此处添加消息处理程序代码
    // 不为绘图消息调用 CStatic::OnPaint()
    CRect rcClient, rcWindow;
    ::GetWindowRect(this->m_hWnd, &rcWindow);
    this->GetClientRect(&rcClient);

    HDC hdc_mem = ::CreateCompatibleDC(dc.m_hDC);
    HBITMAP hBitmap = ::CreateCompatibleBitmap(dc.m_hDC, rcClient.Width(), rcClient.Height());
    HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdc_mem, hBitmap);

    HBRUSH hbr;
    hbr = ::CreateSolidBrush(RGB(m_color_background.r, m_color_background.g, m_color_background.b)); //黑色背景画刷
    ::FillRect(hdc_mem, &rcClient, hbr);
    ::DeleteObject(hbr);

    picture_paint(hdc_mem, 0, 0, 0, 0, m_bitmap); //画帧画面

    ::BitBlt(dc.m_hDC, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), hdc_mem, 0, 0, SRCCOPY); //内存位图拷贝

    ::SelectObject(hdc_mem, hBitmapOld);
    ::DeleteDC(hdc_mem);
    //::DeleteObject(hBitmapOld);
    ::DeleteObject(hBitmap);
}


BOOL CMyStatic::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
    //return CStatic::OnEraseBkgnd(pDC);
}


BOOL CMyStatic::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    //if(m_zm.m_hbitmap == NULL){break;}
    //int xPos = ((int)(short)LOWORD(lParam));
    //int yPos = ((int)(short)HIWORD(lParam));

    //int fwKeys = LOWORD(wParam);
    //short zDelta = (short)HIWORD(wParam); //滚轮向前滚动zDelta>0; 向后滚动时zDelta<0;
    if (m_Draw_Mode != DRAW_MODE_NONE)
    {
        return CStatic::OnMouseWheel(nFlags, zDelta, pt);
    }

    //if(m_handle_player || m_bPicture == TRUE)
    {
        if (zDelta > 0) //放大图像
        {
            this->ScreenToClient(&pt);
            if (::PtInRect(&m_st_rect_scale.dst, pt)) //如果用户在图像之外的空白区域滚动鼠标滚轮，则不做响应
            {
                m_zoom_scale_will = m_zoom_scale + DX_SCALE; //在之前的基础上放大10%
                m_mouse_scale_point_x = pt.x;
                m_mouse_scale_point_y = pt.y;

                CalcIntersectRect();
                this->Invalidate(0);
                this->UpdateWindow();
            }
        }
        else if (zDelta < 0) //缩小图像
        {
            if (m_zoom_scale - DX_SCALE >= 0)
            {
                this->ScreenToClient(&pt);
                if (::PtInRect(&m_st_rect_scale.dst, pt)) //如果用户在图像之外的空白区域滚动鼠标滚轮，则不做响应
                {
                    m_zoom_scale_will = m_zoom_scale - DX_SCALE; //在之前的基础上缩小10%
                    if (m_zoom_scale_will < 0.1f){ m_zoom_scale_will = 0.1f; }

                    m_mouse_scale_point_x = pt.x;
                    m_mouse_scale_point_y = pt.y;

                    CalcIntersectRect();
                    this->Invalidate(0);
                    this->UpdateWindow();
                }
            }
        }
    }
    return CStatic::OnMouseWheel(nFlags, zDelta, pt);
}


void CMyStatic::OnRButtonDown(UINT nFlags, CPoint point)
{
    if (m_bitmap.bmBits == NULL){ return; }

    if (m_Draw_Shape == DRAW_SHAPE_POLYGON) //如果正在处于"画封闭多边形"模式
    {
        if (m_Is_Popup_Right_Menu == FALSE)
        {
            int cnt = m_vector_draw_shapes.size();
            if (cnt > 0)
            {
                int cnt2 = m_vector_draw_shapes[cnt - 1].vector_point.size();
                if (cnt2 >= 3)
                {
                    m_Is_Popup_Right_Menu = TRUE;

                    POINT pt = m_vector_draw_shapes[cnt - 1].vector_point[0]; //如果大于等于3个点，并且点击了右键，则将最后一个点和第一个点连接起来
                    //m_vector_draw_shapes[cnt - 1].vector_point.push_back(pt);

                    this->Invalidate(0);
                }
            }
            return;
        }
    }
    else if (m_Draw_Shape == DRAW_SHAPE_LINES) //如果正在处于"画一组首尾相接的线"模式
    {
        if (m_Is_Popup_Right_Menu == FALSE)
        {
            m_Is_Popup_Right_Menu = TRUE;
            this->Invalidate(0);
            return;
        }
    }

    //-----------------------------------
    CMenu hMenu;
    if (hMenu.CreatePopupMenu())
    {
        //hMenu.SetBkDisableColor(RGB(60,60,60));
        /*
        hMenu.AppendMenu(MF_STRING, 1, _T("设置为画图模式"));
        hMenu.AppendMenu(MF_STRING | MF_GRAYED, 2, _T("后退一步"));
        hMenu.AppendMenu(MF_STRING | MF_GRAYED, 3, _T("全部清除"));
        hMenu.AppendMenu(MF_STRING, 4, _T("设置线条颜色"));

        //------插入一个弹出菜单----------
        CMenu hSubmenu0;
        if (!hSubmenu0.CreatePopupMenu()){ return; }
        hSubmenu0.AppendMenu(MF_STRING, 50, _T("什么都不画"));
        hSubmenu0.AppendMenu(MF_STRING, 51, _T("画点"));
        hSubmenu0.AppendMenu(MF_STRING, 52, _T("画线段"));
        hSubmenu0.AppendMenu(MF_STRING, 53, _T("画折线"));
        hSubmenu0.AppendMenu(MF_STRING, 54, _T("画矩形"));
        hSubmenu0.AppendMenu(MF_STRING, 55, _T("画椭圆"));
        hSubmenu0.AppendMenu(MF_STRING, 56, _T("画封闭多边形"));
        hMenu.AppendMenu(MF_POPUP | MF_GRAYED, (UINT_PTR)hSubmenu0.m_hMenu, _T("设置画图形状"));
        */
        hMenu.AppendMenu(MF_SEPARATOR); //菜单分隔线

        char str[200];
        sprintf(str, _T("图像原始大小:%dx%d"), m_bitmap.bmWidth, m_bitmap.bmHeight);
        hMenu.AppendMenu(MF_STRING | MF_DISABLED, 20, str); //在右键菜单中显示图像原始大小
        sprintf(str, _T("当前缩放比例为%.0f%%"), m_zoom_scale * 100);
        hMenu.AppendMenu(MF_STRING | MF_DISABLED, 21, str); //在右键菜单中显示放大比例
        
        CMenu hSubmenu1;
        if (!hSubmenu1.CreatePopupMenu()){ return; }
        double zoom_scale_float[] = { 0.5, 1.0, 1.5, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0 };
        char zoom_scale[][20] = { _T("50%"), _T("100%"), _T("150%"), _T("200%"), _T("300%"), _T("400%"), _T("500%"), _T("600%"), _T("700%"), _T("800%"), _T("900%") };
        int cnt = sizeof(zoom_scale_float) / sizeof(double);
        for (int i = 0; i < cnt; i++)
        {
            hSubmenu1.AppendMenu(MF_STRING, 100 + i, zoom_scale[i]);
        }
        hMenu.AppendMenu(MF_POPUP, (UINT_PTR)hSubmenu1.m_hMenu, _T("设置缩放比例"));
        hMenu.AppendMenu(MF_STRING, 201, _T("调整到适合窗口大小"));
        hMenu.AppendMenu(MF_STRING, 211, _T("图片另存为"));

        //-----------设置各个菜单项是否被checked-----------------------
        if (m_Draw_Mode != DRAW_MODE_NONE)
        {
            hMenu.CheckMenuItem(1, MF_BYCOMMAND | MF_CHECKED); //设置为画线模式
            hMenu.EnableMenuItem(2, MF_ENABLED); //变为可用
            hMenu.EnableMenuItem(3, MF_ENABLED); //变为可用
            //hMenu.EnableMenuItem((UINT)hSubmenu0.m_hMenu, MF_ENABLED); //变为可用
            //hSubmenu0.CheckMenuItem(m_Draw_Shape + 50, MF_BYCOMMAND | MF_CHECKED); //设置为画线段模式
        }

        //-----------正式弹出右键菜单-----------------------
        POINT point;
        ::GetCursorPos(&point);
        UINT uCmdID = (UINT)hMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, point.x, point.y, this);//TPM_RETURNCMD:函数将用户所选菜单项的标识符返回到返回值里;
        hMenu.DestroyMenu(); //记得销毁资源
        hSubmenu1.DestroyMenu(); //记得销毁资源

        switch (uCmdID)
        {
        case 0:{break; } //用户并未点击任何菜单项
        case 1: //设置为画线模式
            {
                m_Draw_Mode = (m_Draw_Mode == DRAW_MODE_NONE) ? DRAW_MODE_BEGIN : DRAW_MODE_NONE;
                break;
            }
        case 2: //后退一步
            {
                int cnt = m_vector_draw_shapes.size();
                if (cnt > 0)
                {
                    int cnt2 = m_vector_draw_shapes[cnt - 1].vector_point.size();
                    if (cnt2 > 0)
                    {
                        m_Draw_Shape = m_vector_draw_shapes[cnt - 1].draw_shape;
                        m_vector_draw_shapes[cnt - 1].vector_point.pop_back(); //移除最后画的一个点
                        if (cnt2 == 1){ m_vector_draw_shapes.pop_back(); }
                        if (m_Draw_Shape == DRAW_SHAPE_POLYGON){ m_Is_Popup_Right_Menu = FALSE; } //画封闭多边形比较特殊
                        this->Invalidate();
                    }
                }
                break;
            }
        case 3: //全部清除
            {
                m_vector_draw_shapes.clear(); //清空数组
                this->Invalidate();
                break;
            }
        case 4: //设置线条颜色
            {
                CColorDialog dlg;
                dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT; //CC_RGBINIT可以让上次选择的颜色作为初始颜色显示出来
                dlg.m_cc.rgbResult = RGB(m_color_draw_shape.r, m_color_draw_shape.g, m_color_draw_shape.b); //记录上次选择的颜色
                if (IDOK == dlg.DoModal())
                {
                    m_color_draw_shape.r = GetRValue(dlg.m_cc.rgbResult); //保存用户选择的颜色
                    m_color_draw_shape.g = GetGValue(dlg.m_cc.rgbResult); //保存用户选择的颜色
                    m_color_draw_shape.b = GetBValue(dlg.m_cc.rgbResult); //保存用户选择的颜色
                }
                break;
            }
        case 20:{break; }
        case 21:{break; }
        case 50: //什么都不画
        case 51: //画点
        case 52: //画线段
        case 53: //画折线
        case 54: //画矩形
        case 55: //画椭圆
        case 56: //画封闭多边形
            {
                int type = (uCmdID - 50);
                if (m_Draw_Shape != type)
                {
                    m_Draw_Shape = (KDraw_Shape)type;
                    //m_vector_point.clear(); //清空之前画的点
                }
                if (uCmdID == 56){ m_Is_Popup_Right_Menu = FALSE; }
                else{ m_Is_Popup_Right_Menu = TRUE; }
                break;
            }
        case 100: // 50%
        case 101: // 100%
        case 102: // 150%
        case 103: // 200%
        case 104: // 300%
        case 105: // 400%
        case 106: // 500%
        case 107: // 600%
        case 108: // 700%
        case 109: // 800%
        case 110: // 900%
            {
                m_zoom_scale_will = zoom_scale_float[uCmdID - 100]; //在之前的基础上放大100%

                m_mouse_scale_point_x = 0;
                m_mouse_scale_point_y = 0;

                m_st_rect_scale.org_left_top.x = 0; //调整到窗口客户区左上角
                m_st_rect_scale.org_left_top.y = 0;

                CalcIntersectRect(); //计算矩形交集
                this->Invalidate(0);
                this->UpdateWindow();
                break;
            }
        case 201: //调整到适合窗口大小
            {
                CenterVideo(); //将视频画面调整到屏幕中央(必要时缩放到屏幕窗口大小)
                this->Invalidate(0);
                this->UpdateWindow();
                break;
            }
        case 211: //图片另存为
            {
                BITMAP bitmap;
                this->GetMergeBitmap(bitmap);

                if (bitmap.bmBits == NULL){ break; }

                CFileDialog dlg(FALSE, _T("png"), _T("capture_1.png"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("png(*.png)|*.png||"), this);
                dlg.m_ofn.lpstrTitle = _T("图片另存为");
                if (dlg.DoModal() == IDOK)
                {
                    CString strFileName = dlg.GetPathName();

                    char file[300];
                    wsprintf(file, _T("%s"), strFileName);
                    SaveImage(file, bitmap);
                }

                if (bitmap.bmBits != NULL){ delete[] bitmap.bmBits; bitmap.bmBits = NULL; }

                break;
            }
        }
    }

    CStatic::OnRButtonDown(nFlags, point);
}


void CMyStatic::OnRButtonUp(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    CStatic::OnRButtonUp(nFlags, point);
}


BOOL CMyStatic::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: 在此添加专用代码和/或调用基类

    return CStatic::PreCreateWindow(cs);
}


void CMyStatic::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    m_bLButtonDown = TRUE;
    this->SetCapture();
    this->SetFocus();

    if (m_Draw_Mode == DRAW_MODE_NONE) //如果是非画线模式，则WM_MOUSEMOVE处理的是在界面区移动图像
    {
        if (::IsWindow(m_menu.m_hWnd))
        {
            m_menu.ShowWindow(SW_HIDE);
        }
    }
    else
    {
        POINT pt;
        //pt.x = point.x;
        //pt.y = point.y;
        ConvertPt2Pt(point, pt);

        if (m_Draw_Shape == DRAW_SHAPE_POLYGON || m_Draw_Shape == DRAW_SHAPE_LINES) //画一个封闭多边形
        {
            //如果正处于画多边形模式，则当用鼠标左键画了3个以上点的点后，
            //再点击鼠标右键，则表示结束画多边形，再点击鼠标右键，则表示
            //弹出右键菜单
            if (m_Is_Popup_Right_Menu == TRUE){ m_Is_Popup_Right_Menu = FALSE; }
        }

        //------------------------------------
        int cnt = m_vector_draw_shapes.size();
        if (cnt == 0)
        {
            DrawShapes shape;
            shape.draw_shape = m_Draw_Shape;
            shape.vector_point.push_back(pt);
            shape.vector_color.push_back(m_color_draw_shape);
            m_vector_draw_shapes.push_back(shape);
        }
        else
        {
            if (m_Draw_Shape == m_vector_draw_shapes[cnt - 1].draw_shape) //画图模式未更改
            {
                m_vector_draw_shapes[cnt - 1].vector_color.push_back(m_color_draw_shape);
                m_vector_draw_shapes[cnt - 1].vector_point.push_back(pt);
            }
            else
            {
                DrawShapes shape;
                shape.draw_shape = m_Draw_Shape;
                shape.vector_point.push_back(pt);
                shape.vector_color.push_back(m_color_draw_shape);
                m_vector_draw_shapes.push_back(shape);
            }
        }

        this->Invalidate(0);
    }

    CStatic::OnLButtonDown(nFlags, point);
}


void CMyStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    m_bLButtonDown = FALSE;
    ::ReleaseCapture();

    m_mouse_move_last_x = -1;
    m_mouse_move_last_y = -1;

    if (m_Draw_Mode == DRAW_MODE_NONE) //如果是非画线模式，则WM_MOUSEMOVE处理的是在界面区移动图像
    {

    }
    else
    {
        if (m_Draw_Shape == DRAW_SHAPE_RECT || m_Draw_Shape == DRAW_SHAPE_ELLIPSE)
        {
            int cnt = m_vector_point.size();
            if (cnt <= 1 || cnt > 2) //说明用户在同一个点按下了又松开了，此应该为画点模式
            {
                //m_vector_point.clear(); //清空
            }
        }
        else if (m_Draw_Shape == DRAW_SHAPE_LINES)
        {

        }
        else if (m_Draw_Shape == DRAW_SHAPE_POINTS)
        {

        }
        else if (m_Draw_Shape == DRAW_SHAPE_POLYGON)
        {

        }
    }
    this->Invalidate();

    CStatic::OnLButtonUp(nFlags, point);
}


void CMyStatic::OnMouseMove(UINT nFlags, CPoint point)
{
    if (m_Draw_Mode == DRAW_MODE_NONE) //如果是非画线模式，则WM_MOUSEMOVE处理的是在界面区移动图像
    {
        if (m_bLButtonDown == TRUE)
        {
            if (m_bitmap.bmBits != NULL)
            {
                if (m_mouse_move_last_x == -1 && m_mouse_move_last_y == -1)
                {
                    m_mouse_move_last_x = point.x;
                    m_mouse_move_last_y = point.y;
                    return;
                }
                if (m_mouse_move_last_x != point.x || m_mouse_move_last_y != point.y)
                {
                    m_mouse_move_delta_x = point.x - m_mouse_move_last_x;
                    m_mouse_move_delta_y = point.y - m_mouse_move_last_y;

                    m_mouse_move_last_x = point.x;
                    m_mouse_move_last_y = point.y;

                    m_st_rect_scale.org_left_top.x += m_mouse_move_delta_x;
                    m_st_rect_scale.org_left_top.y += m_mouse_move_delta_y;
                    CalcIntersectRect();

                    this->Invalidate(0);
                }
            }
            else
            {
                m_mouse_move_delta_x = 0;
                m_mouse_move_delta_y = 0;
                m_mouse_move_last_x = -1;
                m_mouse_move_last_y = -1;

                this->Invalidate(0);
            }
        }
        else //鼠标在界面正常移动
        {
            if ( ((CButton *)(this->GetParent()->GetDlgItem(IDC_CHECK2)))->GetCheck() == BST_UNCHECKED ) //未选中“显示宏块信息”单选框
            {
                return CStatic::OnMouseMove(nFlags, point);
            }

            if (m_outPicture.m_picture_frame.PicSizeInMbs > 0)
            {
                POINT pt;
                ConvertPt2Pt(point, pt);

                int mb_width = 16;
                int mb_height = 16;

                int mb_x = pt.x / 16;
                int mb_y = pt.y / 16;

                int mbAddr = m_outPicture.m_picture_frame.PicWidthInMbs * mb_y + mb_x;

                if (m_outPicture.m_picture_frame.m_h264_slice_header.MbaffFrameFlag)
                {
                    mbAddr = m_outPicture.m_picture_frame.PicWidthInMbs * (mb_y / 2) * 2 + mb_x * 2;
                    
                    if (mb_y % 2 == 1)
                    {
                        mbAddr += 1;
                        if (m_outPicture.m_picture_frame.m_h264_slice_data.mb_field_decoding_flag)
                        {
                            mb_height = 32;
                            mb_y -= 16; //如果是底场宏块，则暂时和顶场宏块一起画出来
                        }
                    }
                    else
                    {

                    }
                }

                if (mbAddr >= 0 && mbAddr <= m_outPicture.m_picture_frame.PicSizeInMbs) //击中图片中某个目标
                {
                    if (mbAddr != m_currMbAddr)
                    {
                        m_currMbAddr = mbAddr;

                        if (m_menu.m_bitmap.bmBits != NULL)
                        {
                            delete[] m_menu.m_bitmap.bmBits;
                            m_menu.m_bitmap.bmBits = NULL;
                            memset(&m_menu.m_bitmap, 0, sizeof(BITMAP));
                        }

                        memset(&m_menu.m_mb, 0, sizeof(CH264MacroBlock));

                        if (::IsWindow(m_menu.m_hWnd))
                        {
                            m_menu.ShowWindow(SW_HIDE);
                        }
                        
                        CH264MacroBlock & mb = m_outPicture.m_picture_frame.m_mbs[mbAddr];
                        int ret = 0;
                        BITMAP bitmap;

                        ret = CreateEmptyImage(bitmap, mb_width, mb_height, 24);
                        ret = Cut_Image(m_bitmap, bitmap, mb_x * 16, mb_y * 16, mb_width, mb_height);

                        m_menu.MyTrackPopupMenu(this, point, mb, bitmap);
                    }
                    else
                    {

                    }
                }
                else
                {
                    m_currMbAddr = -1;

                    if (m_menu.m_bitmap.bmBits != NULL)
                    {
                        delete[] m_menu.m_bitmap.bmBits;
                        m_menu.m_bitmap.bmBits = NULL;
                        memset(&m_menu.m_bitmap, 0, sizeof(BITMAP));
                    }

                    memset(&m_menu.m_mb, 0, sizeof(CH264MacroBlock));

                    if (::IsWindow(m_menu.m_hWnd))
                    {
                        m_menu.ShowWindow(SW_HIDE);
                    }
                }
            }
        }
    }
    else //如果是画线模式，则WM_MOUSEMOVE处理的是在界面区画点/线/矩形/椭圆/封闭多边形
    {
        POINT pt;
        //pt.x = point.x;
        //pt.y = point.y;
        ConvertPt2Pt(point, pt);

        m_pt_mouse_move = pt;
        this->Invalidate(0);

        if (m_Draw_Shape == DRAW_SHAPE_RECT || m_Draw_Shape == DRAW_SHAPE_ELLIPSE)
        {
            if (m_bLButtonDown == TRUE)
            {
                int cnt = m_vector_point.size(); //画矩形只需要两个点就够了
                if (cnt >= 2){ POINT pt_temp = m_vector_point.at(0); m_vector_point.clear(); m_vector_point.push_back(pt_temp); }
                m_vector_point.push_back(pt);
                this->Invalidate(0);
            }
        }
        else if (m_Draw_Shape == DRAW_SHAPE_LINES)
        {
            int cnt = m_vector_point.size(); //画折线最多3个点
            if (cnt <= m_max_points_num)
            {
                if (cnt == 1){ m_vector_point.push_back(pt); }
                else if (cnt >= 2){ m_vector_point.at(cnt - 1) = pt; }
                this->Invalidate(0);
            }
        }
        else if (m_Draw_Shape == DRAW_SHAPE_POINTS)
        {
            int cnt = m_vector_point.size(); //画最多3个点
            if (cnt <= m_max_points_num)
            {
                if (cnt == 1){ m_vector_point.push_back(pt); }
                else if (cnt >= 2){ m_vector_point.at(cnt - 1) = pt; }
                this->Invalidate(0);
            }
        }
        else if (m_Draw_Shape == DRAW_SHAPE_POLYGON) //画一个封闭多边形
        {
            int cnt = m_vector_point.size(); //至少3个点
            if (m_Is_Popup_Right_Menu == FALSE)
            {
                if (cnt == 1){ m_vector_point.push_back(pt); }
                else if (cnt >= 2){ m_vector_point.at(cnt - 1) = pt; }
                this->Invalidate(0);
            }
        }
    }

    CStatic::OnMouseMove(nFlags, point);
}


void CMyStatic::OnSize(UINT nType, int cx, int cy)
{
    CStatic::OnSize(nType, cx, cy);

    // TODO: 在此处添加消息处理程序代码
    if (m_st_rect_scale.dst.left == 0 && m_st_rect_scale.dst.right == 0 && m_st_rect_scale.dst.top == 0 && m_st_rect_scale.dst.bottom == 0)
    {
        CenterVideo(); //将视频画面调整到屏幕中央(必要时缩放到屏幕窗口大小)
    }
    else
    {
        CalcIntersectRect();
    }
    this->Invalidate(0);
}


void CMyStatic::OnLButtonDblClk(UINT nFlags, CPoint point)
{
    if (m_Draw_Mode == DRAW_MODE_NONE) //如果是非画线模式，则处理进行鼠标是否击中线段测试
    {

    }

    CStatic::OnLButtonDblClk(nFlags, point);
}


void CMyStatic::OnDropFiles(HDROP hDropInfo)
{
    if (m_IsAcceptDropFiles == FALSE){ return; }

    char file[300];

    int cnt = ::DragQueryFile(hDropInfo, -1, file, 300); //拖拉的文件个数
    for (int i = 0; i < cnt; i++)
    {
        ::DragQueryFile(hDropInfo, i, file, 300); //取得每个文件的文件名

        this->GetParent()->SendMessage(WM_LOOK_PICTURE_OPEN, (WPARAM)file, (LPARAM)this->m_hWnd);

        break;
    }

    ::DragFinish(hDropInfo);

    CStatic::OnDropFiles(hDropInfo);
}


int CMyStatic::Set_Is_Accept_Drop_Files(BOOL isAcceptDropFiles)
{
    m_IsAcceptDropFiles = isAcceptDropFiles;
    return 1;
}


//-----------------------------------------

int CMyStatic::ReleaseHandle()
{
    if (m_hbitmap != NULL)
    {
        ::DeleteObject(m_hbitmap);
        m_hbitmap = NULL;
    }
    if (strlen(m_temp_filename) != 0)
    {
        CFileFind filefind;
        BOOL IsFileFind = filefind.FindFile(m_temp_filename);
        if (IsFileFind)
        {
            ::DeleteFile(m_temp_filename); //删除临时文件
        }
    }

    return 1;
}


int CMyStatic::ReleaseBitmap(BITMAP &bitmap)
{
    unsigned char* pBits = static_cast<unsigned char*>(bitmap.bmBits);

    if (pBits != NULL)
    {
        delete[] pBits; //释放用new申请的资源
        pBits = NULL;
        memset(&bitmap, 0, sizeof(BITMAP));
    }

    return 1;
}


int CMyStatic::OpenImage(char* filename, BITMAP &bitmap, HBITMAP &hBitmap)
{
    CFileFind filefind;
    BOOL IsFileFind = filefind.FindFile(filename);
    if (!IsFileFind)
    {
        //printf("Error: Can not find file: %s;\n", filename);
        char err_str[200];
        wsprintf(err_str, _T("您打开的图片文件[%s]不存在!"), filename);
        ::MessageBox(m_hWnd, err_str, _T("错误"), MB_OK | MB_ICONERROR);
        return 0;
    }

    //----------------------------------------
    CImage img;
    img.Load(filename);

    int width = img.GetWidth();
    int height = img.GetHeight();

    hBitmap = img.Detach(); //如果用Detach()，则CImage析构后，hBitmap仍可使用。

    int nBytes = ::GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    if (!(bitmap.bmBitsPixel == 24 || bitmap.bmBitsPixel == 32))
    {
        char err_str[260];
        wsprintf(err_str, _T("您打开的图片文件bitmap.bmBitsPixel=%d;不是24|32位位图。"), bitmap.bmBitsPixel);
        //printf("Error: OpenImage: bitmap.bmBitsPixel != 24|32\n");
        //::MessageBox(m_hWnd, err_str, _T("警告"), MB_OK|MB_ICONWARNING);

        //-----------------------------------
        if (bitmap.bmBitsPixel == 8)
        {
            BITMAP bitmap_24;
            Convert_Bitmap_BitsPixel_8_to_24(bitmap_24, bitmap);
            //char temp_filename[MAX_PATH];
            int pos = strlen(filename);
            while (pos--){ if (filename[pos] == _T('.')){ break; } } //获取后缀名: bmp|jpg|png|gif
            wsprintf(m_temp_filename, _T("%s~.%s"), filename, filename + pos + 1);
            SaveImage(m_temp_filename, bitmap_24);
            ReleaseBitmap(bitmap_24);
            //ReleaseHandle(); //主动释放资源
            if (m_hbitmap != NULL)
            {
                ::DeleteObject(m_hbitmap);
                m_hbitmap = NULL;
            }

            //--------重新加载临时文件------------------
            CImage img;
            HRESULT hres = img.Load(m_temp_filename);
            BOOL res = ::DeleteFile(m_temp_filename); //删除临时文件

            int width = img.GetWidth();
            int height = img.GetHeight();

            hBitmap = img.Detach(); //如果用Detach()，则CImage析构后，hBitmap仍可使用。

            int nBytes = ::GetObject(hBitmap, sizeof(BITMAP), &bitmap);
            return 1;
        }
        return 0;
    }

    return 1;
}


int CMyStatic::SaveImage(char* filename, HBITMAP &hBitmap)
{
    GUID guid[4] =
    {
        Gdiplus::ImageFormatBMP,
        Gdiplus::ImageFormatJPEG,
        Gdiplus::ImageFormatPNG,
        Gdiplus::ImageFormatGIF
    };
    if (m_guid_index < 0 || m_guid_index>3)
    {
        printf("Erorr: SaveImage: m_guid_index must in [0,3]\n");
    }

    CImage img;
    img.Attach(hBitmap);
    img.Save(filename, guid[m_guid_index]); //可以从guid看出，CImage的Save()实际上是通过GDI+实现的。
    hBitmap = img.Detach(); //如果用Detach()，则CImage析构后，hBitmap仍可使用。

    return 1;
}


int CMyStatic::SaveImage(char* filename, BITMAP &bitmap)
{
    int width = bitmap.bmWidth;
    int height = bitmap.bmHeight;
    int biBitCount = bitmap.bmBitsPixel;
    RGBQUAD *pColorTable = NULL;

    int colorTablesize = 0; //颜色表大小，以字节为单位，灰度图像颜色表为1024字节，彩色图像颜色表大小为0
    //if(biBitCount == 8){colorTablesize=1024;}

    //int lineByte = (width * biBitCount/8+3)/4*4; //待存储图像数据每行字节数为4的倍数
    int lineByte = bitmap.bmWidthBytes; //待存储图像数据每行字节数为4的倍数

    int size_1 = sizeof(BITMAPFILEHEADER); // size_1 = 14
    int size_2 = sizeof(BITMAPINFOHEADER); // size_2 = 40
    int size_3 = lineByte * height; //计算位图尺寸

    //int bpp = bitmap.bmBitsPixel/8; //bpp代表通道的数目，一般 bpp = 3

    //--------------1. 位图文件头结构-----------------------------------------------------
    BITMAPFILEHEADER fileHead;

    fileHead.bfType = 0x4D42; //bmp类型
    fileHead.bfSize = size_1 + size_2 + colorTablesize + lineByte * height; //bfSize是图像文件4个组成部分之和
    fileHead.bfReserved1 = 0;
    fileHead.bfReserved2 = 0;
    fileHead.bfOffBits = 54 + colorTablesize; //bfOffBits是图像文件前3个部分所需空间之和

    //--------------2. 位图信息头结构-----------------------------------------------------
    BITMAPINFOHEADER head;

    head.biBitCount = biBitCount; // 8,24,32
    head.biClrImportant = 0;
    head.biClrUsed = 0;
    head.biCompression = 0; //BI_RGB = 0L
    head.biHeight = height;
    head.biPlanes = 1;
    head.biSize = 40;
    head.biSizeImage = lineByte * height;
    head.biWidth = width;
    head.biXPelsPerMeter = 0;
    head.biYPelsPerMeter = 0;

    //---------------3. 内存中的文件读写操作-------------------------------
    long file_size = fileHead.bfSize; //计算位图文件尺寸

    unsigned char* pBits = static_cast<unsigned char*>(bitmap.bmBits);

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, file_size);
    if (hMem == NULL){ printf("Erorr: GlobalAlloc: hMem == NULL\n"); return 0; }
    unsigned char *pbuff = static_cast<unsigned char*>(GlobalLock(hMem)); // get the actual pointer for the HGLOBAL
    memcpy(pbuff, &fileHead, size_1); //内存复制
    pbuff += size_1;
    memcpy(pbuff, &head, size_2); //内存复制
    pbuff += size_2;
    memcpy(pbuff, pBits, size_3); //内存复制

    GlobalUnlock(hMem);

    IStream *pStream = 0;
    HRESULT hr = CreateStreamOnHGlobal(hMem, FALSE, &pStream); //此函数是内存数据到文件流的关键API函数
    if (hr != S_OK)
    {
        printf("Erorr: CreateStreamOnHGlobal: hr != S_OK\n");
        return 0;
    }

    //--------------4. 将文件流数据正式保存到磁盘文件中----------------------------------------
    GUID guid[4] =
    {
        Gdiplus::ImageFormatBMP,
        Gdiplus::ImageFormatJPEG,
        Gdiplus::ImageFormatPNG,
        Gdiplus::ImageFormatGIF
    };
    if (m_guid_index < 0 || m_guid_index>3)
    {
        printf("Erorr: SaveImage: m_guid_index must in [0,3]\n");
        //return 0;
    }

    CImage img;
    img.Load(pStream);
    //img.Save(filename, guid[m_guid_index]); //可以从guid看出，CImage的Save()实际上是通过GDI+实现的。
    img.Save(filename, guid[2]); //在构造函数中m_guid_index=2，但在dll的调用中并未执行该构造函数，目前不清楚是什么原因导致的
    //img.Detach(); //不要调用该函数，否则内存释放不正确
    img.Destroy();
    pStream->Release();
    GlobalFree(hMem); //释放GlobalAlloc(...)申请的内存

    return 1;
}


int CMyStatic::CreateEmptyImage(BITMAP &bitmap, int width, int height, int bmBitsPixel)
{
    //bmBitsPixel = 32;

    bitmap.bmWidth = width;
    bitmap.bmHeight = height;
    bitmap.bmBitsPixel = bmBitsPixel;

    bitmap.bmType = 0;
    bitmap.bmPlanes = 1;

    bitmap.bmWidthBytes = (width * bmBitsPixel / 8 + 3) / 4 * 4;

    printf("CreateEmptyImage: [%d x %d] memory = %d bytes;\n", width, height, bitmap.bmHeight * bitmap.bmWidthBytes);

    unsigned char *pBits = new unsigned char[bitmap.bmHeight * bitmap.bmWidthBytes]; //在堆上申请
    if (pBits == NULL){ printf("CreateEmptyImage: pBits == NULL\n"); return 0; }
    memset(pBits, 0, sizeof(unsigned char) * bitmap.bmHeight * bitmap.bmWidthBytes); //初始化为黑色背景

    bitmap.bmBits = pBits;

    return 1;
}


//----------将8位位图转换成24位----------------------
int CMyStatic::Convert_Bitmap_BitsPixel_8_to_24(BITMAP &bitmap_24, BITMAP &bitmap_8)
{
    //------------1. 检查输入参数-------------------------
    BITMAP bitmap_src = bitmap_8;
    //BITMAP bitmap_dst = bitmap_24;

    unsigned char* pBits = static_cast<unsigned char*>(bitmap_src.bmBits);
    int bpp = bitmap_src.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3

    if (bpp != 1){ return 0; }

    int width2 = bitmap_src.bmWidth;
    int height2 = bitmap_src.bmHeight;
    CreateEmptyImage(bitmap_24, width2, height2, 24);
    int bpp2 = bitmap_24.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3
    unsigned char* pBits2 = static_cast<unsigned char*>(bitmap_24.bmBits);

    //------------2. 正式转换---------------------------------------
    for (int y = 0; y < bitmap_src.bmHeight; y++)
    {
        for (int x = 0; x < bitmap_src.bmWidth; x++)
        {
            int B = pBits[y * bitmap_src.bmWidthBytes + x * bpp + 0];
            //int G = pBits[y * bitmap_src.bmWidthBytes + x * bpp + 1];
            //int R = pBits[y * bitmap_src.bmWidthBytes + x * bpp + 2];
            int G = B;
            int R = B;

            pBits2[y * bitmap_24.bmWidthBytes + x * bpp2 + 0] = B; //Blue
            pBits2[y * bitmap_24.bmWidthBytes + x * bpp2 + 1] = G; //Green
            pBits2[y * bitmap_24.bmWidthBytes + x * bpp2 + 2] = R; //Red
        }
    }

    return 1;
}


//裁剪位图的一个矩形区域
int CMyStatic::Cut_Image(BITMAP &bitmap_src, BITMAP &bitmap_dst, int x, int y, int w, int h)
{
    //------------1. 检查输入参数-------------------------
    if (x < 0 || x > bitmap_src.bmWidth - 1){ return 0; }
    if (y < 0 || y > bitmap_src.bmHeight - 1){ return 0; }
    if (w < 0 || w > bitmap_src.bmWidth){ return 0; }
    if (h < 0 || h > bitmap_src.bmHeight){ return 0; }

    unsigned char* pBits = static_cast<unsigned char*>(bitmap_src.bmBits);
    int bpp = bitmap_src.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3

    int width2 = w;
    int height2 = h;
    //	CreateEmptyImage(bitmap_dst, width2, height2, bitmap_src.bmBitsPixel);
    int bpp2 = bitmap_dst.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3
    unsigned char* pBits2 = static_cast<unsigned char*>(bitmap_dst.bmBits);

    //------------2. 正式裁剪位图---------------------------------------
    for (int Y = 0; Y < bitmap_src.bmHeight; Y++)
    {
        for (int X = 0; X < bitmap_src.bmWidth; X++)
        {
            if (X >= x && X < x + w && Y >= y && Y < y + h)
            {
                int YY = bitmap_src.bmHeight - 1 - Y;

                int B = pBits[YY * bitmap_src.bmWidthBytes + X * bpp + 0];
                int G = pBits[YY * bitmap_src.bmWidthBytes + X * bpp + 1];
                int R = pBits[YY * bitmap_src.bmWidthBytes + X * bpp + 2];

                int y2 = bitmap_dst.bmHeight - 1 - (Y - y); //计算坐标值 (注意: bitmap第一行像素值是放在内存中最后一行的，第二行以此类推)
                int x2 = X - x; //计算坐标值

                pBits2[y2 * bitmap_dst.bmWidthBytes + x2 * bpp2 + 0] = B; //Blue
                pBits2[y2 * bitmap_dst.bmWidthBytes + x2 * bpp2 + 1] = G; //Green
                pBits2[y2 * bitmap_dst.bmWidthBytes + x2 * bpp2 + 2] = R; //Red
            }
        }
    }

    return 1;
}


/*--------------在位图DC中画矩形----------------------------------
 * 原理:
 *      可以直接利用 ::Rectangle(...); 画矩形
 */
int CMyStatic::Draw_Rect_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/)
{
    //-------------1. 检查输入参数-------------------------------------------------
    if (w < 0){ return 0; }
    if (h < 0){ return 0; }

    if (r > 255){ r = 255; }
    else if (r < 0){ r = 0; }
    if (g > 255){ g = 255; }
    else if (g < 0){ g = 0; }
    if (b > 255){ b = 255; }
    else if (b < 0){ b = 0; }

    if (line_width <= 0){ line_width = 1; }

    //--------采用win32 API画图--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_SOLID, line_width, RGB(r, g, b));
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);

    ::Rectangle(hdc, x, y, x + w, y + h);

    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen);

    return 1;
}


/*--------------在位图DC的指定位置画指定颜色的椭圆----------------------------------
 * 原理:
 *      可以直接利用 ::Ellipse(...); 画椭圆
 *                    / x = x0 + a * cos(A)
 *     椭圆参数方程:  |
 *                    \ y = y0 + b * sin(A)
 */
int CMyStatic::Draw_Ellipse_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/)
{
    //-------------1. 检查输入参数-------------------------------------------------
    if (w < 0){ return 0; }
    if (h < 0){ return 0; }

    if (r > 255){ r = 255; }
    else if (r < 0){ r = 0; }
    if (g > 255){ g = 255; }
    else if (g < 0){ g = 0; }
    if (b > 255){ b = 255; }
    else if (b < 0){ b = 0; }

    if (line_width <= 0){ line_width = 1; }

    //--------采用win32 API画图--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_SOLID, line_width, RGB(r, g, b));
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);

    ::Ellipse(hdc, x, y, x + w, y + h);

    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen);

    return 1;
}


//画直线
int CMyStatic::Draw_Line_DC(HDC hdc, POINT pt1, POINT pt2, int r, int g, int b, int line_width/*=1*/)
{
    //-------------1. 检查输入参数-------------------------------------------------
    //if(pt1.x == pt2.x){pt2.x += 1;} //暂时这样处理

    if (r > 255){ r = 255; }
    else if (r < 0){ r = 0; }
    if (g > 255){ g = 255; }
    else if (g < 0){ g = 0; }
    if (b > 255){ b = 255; }
    else if (b < 0){ b = 0; }

    if (line_width <= 0){ line_width = 1; }

    //--------采用win32 API画图--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_SOLID, line_width, RGB(r, g, b));
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);

    ::MoveToEx(hdc, pt1.x, pt1.y, NULL);
    ::LineTo(hdc, pt2.x, pt2.y);

    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen);

    return 1;
}


/*--------------在位图DC的指定位置画指定颜色的多边形----------------------------------
 * 原理:
 *      可以直接利用 ::Polyline(...); 画椭圆
 */
int CMyStatic::Draw_Polygon_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/)
{
    //-------------1. 检查输入参数-------------------------------------------------
    int cnt = vector_point.size();
    if (cnt < 2){ return 0; } //数组中至少有两个元素

    if (r > 255){ r = 255; }
    else if (r < 0){ r = 0; }
    if (g > 255){ g = 255; }
    else if (g < 0){ g = 0; }
    if (b > 255){ b = 255; }
    else if (b < 0){ b = 0; }

    if (line_width <= 0){ line_width = 1; }

    //--------采用win32 API画图--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_SOLID, line_width, RGB(r, g, b));
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);

    POINT *pt = new POINT[cnt];
    for (int i = 0; i < cnt; i++){ pt[i] = vector_point.at(i); }
    ::Polyline(hdc, pt, cnt); //画折线

    delete[] pt;
    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen);

    return 1;
}


//在位图的指定位置画指定颜色的三角形
int CMyStatic::Draw_Triangle_DC(HDC hdc, POINT pt1, POINT pt2, POINT pt3, int r, int g, int b, int line_width/*=1*/)
{
    Draw_Line_DC(hdc, pt1, pt2, r, g, b, line_width);
    Draw_Line_DC(hdc, pt2, pt3, r, g, b, line_width);
    Draw_Line_DC(hdc, pt3, pt1, r, g, b, line_width);

    return 1;
}


/*--------------在位图DC的指定位置画指定颜色的点----------------------------------
 * 原理:
 *      可以直接利用 ::MoveToEx(...); 画点
 */
int CMyStatic::Draw_Points_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/)
{
    //-------------1. 检查输入参数-------------------------------------------------
    int cnt = vector_point.size();
    if (cnt < 2){ return 0; } //数组中至少有两个元素

    if (r > 255){ r = 255; }
    else if (r < 0){ r = 0; }
    if (g > 255){ g = 255; }
    else if (g < 0){ g = 0; }
    if (b > 255){ b = 255; }
    else if (b < 0){ b = 0; }

    if (line_width <= 0){ line_width = 1; }

    //--------采用win32 API画图--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_SOLID, line_width, RGB(r, g, b));
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen);

    for (int i = 0; i < cnt; i++)
    {
        POINT pt = vector_point.at(i);
        ::MoveToEx(hdc, pt.x, pt.y, NULL);
        ::LineTo(hdc, pt.x + 1, pt.y);
    }

    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen);

    return 1;
}


//-----------------------------------------------
int CMyStatic::picture_open(CString filename)
{
    picture_close(); //先关闭已打开图片

    m_file_path_open = filename;

    char file[600];
    wsprintf(file, _T("%s"), filename);

    int res = OpenImage(file, m_bitmap, m_hbitmap);
    if (res == 0){ return 0; }

    m_mouse_scale_point_x = 0;
    m_mouse_scale_point_y = 0;
    memset(&m_st_rect_scale, 0, sizeof(KRectScale));
    //m_vector_point.clear();
    m_vector_draw_shapes.clear();
    m_Draw_Mode = DRAW_MODE_NONE;
    //m_Draw_Shape = DRAW_SHAPE_POLYGON; //默认设置为画封闭多边形
    //m_Draw_Shape = DRAW_SHAPE_RECT; //默认设置为画矩形

    m_rect_render.left = 0;
    m_rect_render.right = m_bitmap.bmWidth - 1;
    m_rect_render.top = 0;
    m_rect_render.bottom = m_bitmap.bmHeight - 1;

    CenterVideo();
    //this->CreateEmptyImage(m_bitmap_copy, m_bitmap.bmWidth, m_bitmap.bmHeight, m_bitmap.bmBitsPixel); //同样尺寸的缓存位图
    this->Invalidate();

    this->GetParent()->SendMessage(WM_LOOK_PICTURE_OPEN, (WPARAM)file, (LPARAM)this->m_hWnd);

    return 1;
}


int CMyStatic::picture_open_bitmap(BITMAP bitmap)
{
    if (bitmap.bmBits == NULL){ return 0; }

    picture_close(); //先关闭已打开图片

    m_bitmap = bitmap;

    m_mouse_scale_point_x = 0;
    m_mouse_scale_point_y = 0;
    memset(&m_st_rect_scale, 0, sizeof(KRectScale));
    //m_vector_point.clear();
    m_vector_draw_shapes.clear();
    m_Draw_Mode = DRAW_MODE_NONE;
    //m_Draw_Shape = DRAW_SHAPE_POLYGON; //默认设置为画封闭多边形
    //m_Draw_Shape = DRAW_SHAPE_RECT; //默认设置为画矩形

    m_rect_render.left = 0;
    m_rect_render.right = m_bitmap.bmWidth - 1;
    m_rect_render.top = 0;
    m_rect_render.bottom = m_bitmap.bmHeight - 1;

    CenterVideo();
    //this->CreateEmptyImage(m_bitmap_copy, m_bitmap.bmWidth, m_bitmap.bmHeight, m_bitmap.bmBitsPixel); //同样尺寸的缓存位图
    this->Invalidate();

    return 1;
}


int CMyStatic::picture_open_bitmap_fresh(BITMAP &bitmap)
{
    if (bitmap.bmBits == NULL){ return 0; }

    //picture_close(); //先关闭已打开图片

    //m_bitmap = bitmap;

    if (m_bitmap.bmWidth != bitmap.bmWidth || m_bitmap.bmHeight != bitmap.bmHeight)
    {
        if (m_bitmap.bmBits != NULL){ delete[] m_bitmap.bmBits; m_bitmap.bmBits = NULL; }
        CreateEmptyImage(m_bitmap, bitmap.bmWidth, bitmap.bmHeight, bitmap.bmBitsPixel);
    }

    memcpy(m_bitmap.bmBits, bitmap.bmBits, sizeof(unsigned char) * bitmap.bmHeight * bitmap.bmWidthBytes);

    m_mouse_scale_point_x = 0;
    m_mouse_scale_point_y = 0;
    memset(&m_st_rect_scale, 0, sizeof(KRectScale));
    //m_vector_point.clear();
    m_vector_draw_shapes.clear();
    m_Draw_Mode = DRAW_MODE_NONE;
    //m_Draw_Shape = DRAW_SHAPE_POLYGON; //默认设置为画封闭多边形
    //m_Draw_Shape = DRAW_SHAPE_RECT; //默认设置为画矩形

    m_rect_render.left = 0;
    m_rect_render.right = m_bitmap.bmWidth - 1;
    m_rect_render.top = 0;
    m_rect_render.bottom = m_bitmap.bmHeight - 1;

    CenterVideo();
    //this->CreateEmptyImage(m_bitmap_copy, m_bitmap.bmWidth, m_bitmap.bmHeight, m_bitmap.bmBitsPixel); //同样尺寸的缓存位图
    this->Invalidate();

    return 1;
}


int CMyStatic::picture_close()
{
    if (m_hbitmap != NULL)
    {
        ::DeleteObject(m_hbitmap);
        m_hbitmap = NULL;
    }
    else
    {
        if (m_bitmap.bmBits != NULL)
        {
            delete[] m_bitmap.bmBits;
            m_bitmap.bmBits = NULL;
        }
    }
    memset(&m_bitmap, 0, sizeof(BITMAP));

    if (m_bitmap_copy.bmBits != NULL)
    {
        delete[] m_bitmap_copy.bmBits;
        m_bitmap_copy.bmBits = NULL;
    }

    return 1;
}


//复制一张位图
int CMyStatic::picture_copy(BITMAP bitmap_src, BITMAP &bitmap_dst)
{
    if (bitmap_src.bmBits == NULL)
    {
        return 0;
    }

    CreateEmptyImage(bitmap_dst, bitmap_src.bmWidth, bitmap_src.bmHeight, bitmap_src.bmBitsPixel);

    memcpy(bitmap_dst.bmBits, bitmap_src.bmBits, sizeof(unsigned char) * bitmap_src.bmHeight * bitmap_src.bmWidthBytes);

    return 1;
}


//画图
int CMyStatic::picture_paint(HDC hdc, int dst_x, int dst_y, int dst_width, int dst_height, BITMAP &bitmap)
{
    BITMAPINFO bmpInfo; //创建位图
    void *ppvBits;

    //-----------------------------------------------------
    memset(&bmpInfo, 0, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = bitmap.bmWidth;//宽度
    bmpInfo.bmiHeader.biHeight = bitmap.bmHeight;//高度
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24; //24
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    HBITMAP hBitmap2 = ::CreateDIBSection(NULL, &bmpInfo, DIB_RGB_COLORS, &ppvBits, NULL, 0); //创建DIB, 可以只在初始时创建一次
    if (hBitmap2 == NULL){ return 0; }
    //ASSERT(hBitmap2 != NULL);

    if (bitmap.bmBitsPixel == 24)
    {
        memcpy(ppvBits, bitmap.bmBits, bitmap.bmHeight * bitmap.bmWidthBytes); //将裸数据复制到bitmap关联的像素区域
    }
    else if (bitmap.bmBitsPixel == 32)
    {
        //------将32位转成24位-------------
        unsigned char* pBits = static_cast<unsigned char*>(bitmap.bmBits);
        int bpp = bitmap.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3
        unsigned char* pBits2 = static_cast<unsigned char*>(ppvBits);
        int bmWidthBytes = (bitmap.bmWidth * 24 / 8 + 3) / 4 * 4;
        for (int y = 0; y < bitmap.bmHeight; y++)
        {
            for (int x = 0; x < bitmap.bmWidth; x++)
            {
                int B = pBits[y * bitmap.bmWidthBytes + x * bpp + 0];
                int G = pBits[y * bitmap.bmWidthBytes + x * bpp + 1];
                int R = pBits[y * bitmap.bmWidthBytes + x * bpp + 2];

                pBits2[y * bmWidthBytes + x * 3 + 0] = B;
                pBits2[y * bmWidthBytes + x * 3 + 1] = G;
                pBits2[y * bmWidthBytes + x * 3 + 2] = R;
            }
        }
    }

    HDC hdc_mem;
    hdc_mem = ::CreateCompatibleDC(hdc);
    HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdc_mem, hBitmap2);

    picture_draw_callback(hdc_mem); //给外部一次处理机会
    picture_draw_shape(hdc_mem); //先画各种图形

    //::BitBlt(hdc, dst_x, dst_y, dst_width, dst_height, hdc_mem, 0, 0, SRCCOPY); //内存位图拷贝

    ::SetStretchBltMode(hdc, COLORONCOLOR); //设置指定设备环境中的位图拉伸模式
    //::StretchBlt(hdc, dst_x, dst_y, dst_width, dst_height, hdc_mem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY); //内存位图拷贝(有拉伸)

    ::StretchBlt(hdc, m_st_rect_scale.dst.left, m_st_rect_scale.dst.top, m_st_rect_scale.dst.right - m_st_rect_scale.dst.left + 1, m_st_rect_scale.dst.bottom - m_st_rect_scale.dst.top + 1,
        hdc_mem, m_st_rect_scale.src.left, m_st_rect_scale.src.top, m_st_rect_scale.src.right - m_st_rect_scale.src.left + 1, m_st_rect_scale.src.bottom - m_st_rect_scale.src.top + 1, SRCCOPY); //内存位图拷贝(有拉伸)

    //char str[100];
    //wsprintf(str, _T("%d/%d"), m_frame_num_now, m_total_num_frames);
    //::TextOut(hdc, 0, 0, str, wcslen(str)); //显示当前帧号

    ::SelectObject(hdc_mem, hBitmapOld);
    ::DeleteDC(hdc_mem);
    ::DeleteObject(hBitmap2);

    return 1;
}


//供继承类调用
int CMyStatic::picture_draw_callback(HDC hdc)
{
    if ( ((CButton *)(this->GetParent()->GetDlgItem(IDC_CHECK1)))->GetCheck() == BST_UNCHECKED ) //未选中“画宏块网格”单选框
    {
        return 0;
    }

    int line_width = 1;

    //--------画16x16宏块--------------
    HBRUSH hBrush = (HBRUSH)::GetStockObject(NULL_BRUSH);
    HBRUSH hBrush_old = (HBRUSH)::SelectObject(hdc, hBrush);
    HPEN hPen1 = ::CreatePen(PS_SOLID, line_width, RGB(255, 0, 0)); //画16x16宏块网格线的笔的颜色 红色
    HPEN hPen2 = ::CreatePen(PS_SOLID, line_width, RGB(255, 255, 0)); //画 P/B 16x16宏块的子宏块网格线的笔的颜色 黄色
    HPEN hPen3 = ::CreatePen(PS_SOLID, line_width, RGB(0, 255, 0)); //Intra_16x16 绿色
    HPEN hPen4 = ::CreatePen(PS_SOLID, line_width, RGB(0, 0, 255)); //底场宏块 蓝色
    HPEN hPenOld = (HPEN)::SelectObject(hdc, hPen1);

    int W = m_outPicture.m_picture_frame.PicWidthInSamplesL;
    int H = m_outPicture.m_picture_frame.PicHeightInSamplesL;

    //------先画横线-------------
    for (int y = 0; y <= H; y += 16)
    {
        ::MoveToEx(hdc, 0, y, NULL);
        ::LineTo(hdc, W, y);
    }

    //------再画竖线-------------
    for (int x = 0; x <= W; x += 16)
    {
        ::MoveToEx(hdc, x, 0, NULL);
        ::LineTo(hdc, x, H);
    }

    //-------------------------------
    int & MbaffFrameFlag = m_outPicture.m_picture_frame.m_h264_slice_header.MbaffFrameFlag;
    int & PicWidthInMbs = m_outPicture.m_picture_frame.PicWidthInMbs;
    
    for (int CurrMbAddr = 0; CurrMbAddr < m_outPicture.m_picture_frame.mb_cnt; ++CurrMbAddr)
    {
        CH264MacroBlock & mb = m_outPicture.m_picture_frame.m_mbs[CurrMbAddr];

        int isMbAff = (MbaffFrameFlag == 1 && mb.mb_field_decoding_flag == 1) ? 1 : 0;

        int x = mb.m_mb_position_x;
        int y = mb.m_mb_position_y;

        if (isMbAff && y % 2 == 1) //底场宏块
        {
            y += 15; //由于顶场宏块和底场宏块每行的像素是隔行交错的，并不好用网格线表现出来，此处暂时用画帧宏块对的方式来画网格
            ::SelectObject(hdc, hPen4);
        }

        if (mb.m_mb_pred_mode == Intra_4x4)
        {
            ::SelectObject(hdc, hPen2);

            ::MoveToEx(hdc, x, y + 4, NULL); //第一条横线
            ::LineTo(hdc, x + 16, y + 4);
            
            ::MoveToEx(hdc, x, y + 8, NULL); //第二条横线
            ::LineTo(hdc, x + 16, y + 8);

            ::MoveToEx(hdc, x, y + 12, NULL); //第三条横线
            ::LineTo(hdc, x + 16, y + 12);
            
            ::MoveToEx(hdc, x + 4, y, NULL); //第一条竖线
            ::LineTo(hdc, x + 4, y + 16);

            ::MoveToEx(hdc, x + 8, y, NULL); //第二条竖线
            ::LineTo(hdc, x + 8, y + 16);

            ::MoveToEx(hdc, x + 12, y, NULL); //第三条竖线
            ::LineTo(hdc, x + 12, y + 16);
        }
        else if (mb.m_mb_pred_mode == Intra_8x8)
        {
            ::SelectObject(hdc, hPen2);

            ::MoveToEx(hdc, x, y + 8, NULL); //第一条横线
            ::LineTo(hdc, x + 16, y + 8);
            
            ::MoveToEx(hdc, x + 8, y, NULL); //第一条竖线
            ::LineTo(hdc, x + 8, y + 16);
        }
        else if (mb.m_mb_pred_mode == Intra_16x16)
        {
            ::SelectObject(hdc, hPen3);
            ::Rectangle(hdc, x, y, x + 16, y + 16);
        }
        else if (mb.m_mb_pred_mode == I_PCM)
        {
            //do nothing
        }
        else //if(IS_INTER_Prediction_Mode(mb.m_mb_pred_mode)) //帧间预测
        {
            ::SelectObject(hdc, hPen2);

            int NumMbPart =  mb.m_NumMbPart;
            if(mb.m_name_of_mb_type == B_Skip || mb.m_name_of_mb_type == B_Direct_16x16)
            {
                NumMbPart = 4;
            }
            
            //------------------子宏块-----------------------
            for (int mbPartIdx = 0; mbPartIdx <= NumMbPart - 1; mbPartIdx++)
            {
                int NumSubMbPart = 0;
                int partWidth = 0;
                int partHeight = 0;

                if(mb.m_name_of_mb_type != P_8x8 
                    && mb.m_name_of_mb_type != P_8x8ref0 
                    && mb.m_name_of_mb_type != B_Skip 
                    && mb.m_name_of_mb_type != B_Direct_16x16 
                    && mb.m_name_of_mb_type != B_8x8)
                {
                    NumSubMbPart = 1;
                    partWidth = mb.MbPartWidth;
                    partHeight = mb.MbPartHeight;
                }
                else if(mb.m_name_of_mb_type == P_8x8 
                        || mb.m_name_of_mb_type == P_8x8ref0 
                        || (mb.m_name_of_mb_type == B_8x8 && mb.m_name_of_sub_mb_type[ mbPartIdx ] != B_Direct_8x8)
                        )
                {
                    NumSubMbPart = mb.NumSubMbPart[mbPartIdx];
                    partWidth = mb.SubMbPartWidth[mbPartIdx];
                    partHeight = mb.SubMbPartHeight[mbPartIdx];
                }
                else //if(mb.m_name_of_mb_type == B_Skip 
                        //|| mb.m_name_of_mb_type == B_Direct_16x16 
                        //|| (mb.m_name_of_mb_type == B_8x8 && mb.m_name_of_sub_mb_type[ mbPartIdx ] == B_Direct_8x8)
                        //)
                {
                    NumSubMbPart = 4;
                    partWidth = 4;
                    partHeight = 4;
                }
                
                int xP = InverseRasterScan(mbPartIdx, mb.MbPartWidth, mb.MbPartHeight, 16, 0);
                int yP = InverseRasterScan(mbPartIdx, mb.MbPartWidth, mb.MbPartHeight, 16, 1);
                
                //------------------子宏块的子宏块-----------------------
                for (int subMbPartIdx = 0; subMbPartIdx <= NumSubMbPart - 1; subMbPartIdx++)
                {
                    int xS = 0;
                    int yS = 0;

                    //--------------------------------------
                    if (mb.m_name_of_mb_type == P_8x8 
                        || mb.m_name_of_mb_type == P_8x8ref0 
                        || mb.m_name_of_mb_type == B_8x8
                       )
                    {
                        xS = InverseRasterScan( subMbPartIdx, partWidth, partHeight, 8, 0 );
                        yS = InverseRasterScan( subMbPartIdx, partWidth, partHeight, 8, 1 );
                    }
                    else
                    {
                        xS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 0 );
                        yS = InverseRasterScan( subMbPartIdx, 4, 4, 8, 1 );
                    }
                    
                    int xAL = mb.m_mb_position_x + xP + xS;
                    int yAL = mb.m_mb_position_y + yP + yS;

                    if (isMbAff && mb.m_mb_position_y % 2 == 1) //底场宏块
                    {
                        yAL += 15; //由于顶场宏块和底场宏块每行的像素是隔行交错的，并不好用网格线表现出来，此处暂时用画帧宏块对的方式来画网格
                        ::SelectObject(hdc, hPen4);
                    }

                    //-----------画子宏块的子宏块-------------------
                    ::Rectangle(hdc, xAL, yAL, xAL + partWidth, yAL + partHeight);
                }
            }
        }
    }

    ::SelectObject(hdc, hBrush_old);
    ::SelectObject(hdc, hPenOld);
    ::DeleteObject(hPen1);
    ::DeleteObject(hPen2);
    ::DeleteObject(hPen3);
    ::DeleteObject(hPen4);

    return 1;
}


int CMyStatic::GetBitampFromDC(BITMAP &Bitmap, BOOL isDrawShape/*=FALSE*/)
{
    //if(Bitmap.bmBits == NULL){return 0;}
    //if(m_bitmap.bmWidth > Bitmap.bmWidth || m_bitmap.bmHeight > Bitmap.bmHeight){return 0;}

    BITMAP bitmap = m_bitmap;
    BITMAPINFO bmpInfo; //创建位图
    void *ppvBits;

    //CreateEmptyImage(Bitmap, bitmap.bmWidth, bitmap.bmHeight, bitmap.bmBitsPixel); //在外部已经创建

    //-----------------------------------------------------
    memset(&bmpInfo, 0, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = bitmap.bmWidth;//宽度
    bmpInfo.bmiHeader.biHeight = bitmap.bmHeight;//高度
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24; //24
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    HBITMAP hBitmap2 = ::CreateDIBSection(NULL, &bmpInfo, DIB_RGB_COLORS, &ppvBits, NULL, 0); //创建DIB, 可以只在初始时创建一次
    if (hBitmap2 == NULL){ return 0; }
    //ASSERT(hBitmap2 != NULL);

    if (bitmap.bmBitsPixel == 24)
    {
        memcpy(ppvBits, bitmap.bmBits, bitmap.bmHeight * bitmap.bmWidthBytes); //将裸数据复制到bitmap关联的像素区域
    }
    else if (bitmap.bmBitsPixel == 32)
    {
        //------将32位转成24位-------------
        unsigned char* pBits = static_cast<unsigned char*>(bitmap.bmBits);
        int bpp = bitmap.bmBitsPixel / 8; //bpp代表通道的数目，一般 bpp = 3
        unsigned char* pBits2 = static_cast<unsigned char*>(ppvBits);
        int bmWidthBytes = (bitmap.bmWidth * 24 / 8 + 3) / 4 * 4;
        for (int y = 0; y < bitmap.bmHeight; y++)
        {
            for (int x = 0; x < bitmap.bmWidth; x++)
            {
                int B = pBits[y * bitmap.bmWidthBytes + x * bpp + 0];
                int G = pBits[y * bitmap.bmWidthBytes + x * bpp + 1];
                int R = pBits[y * bitmap.bmWidthBytes + x * bpp + 2];

                pBits2[y * bmWidthBytes + x * 3 + 0] = B;
                pBits2[y * bmWidthBytes + x * 3 + 1] = G;
                pBits2[y * bmWidthBytes + x * 3 + 2] = R;
            }
        }
    }

    //HDC hdc = ::GetDC(this->m_hWnd);
    HDC hdc_mem;
    //hdc_mem = ::CreateCompatibleDC(hdc);
    hdc_mem = ::CreateCompatibleDC(NULL);

    HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdc_mem, hBitmap2);

    picture_draw_callback(hdc_mem); //给外部一次处理机会
    if (isDrawShape)
    {
        picture_draw_shape(hdc_mem); //先画各种图形
    }

    //------------收尾工作---------------------------
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bitmap.bmWidth;
    bi.biHeight = bitmap.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = bitmap.bmBitsPixel;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD size = bitmap.bmHeight * bitmap.bmWidthBytes;
    //unsigned char *pBits2 = new unsigned char[size]; //在堆上申请

    //if(Bitmap.bmBits == NULL || Bitmap.bmWidth != bitmap.bmWidth || Bitmap.bmHeight != bitmap.bmHeight)
    {
        //if(Bitmap.bmBits != NULL){delete [] Bitmap.bmBits; Bitmap.bmBits = NULL;}
        Bitmap = bitmap;
        Bitmap.bmBits = new unsigned char[size]; //在堆上申请
    }

    int nRet = ::GetDIBits(hdc_mem, hBitmap2, 0, (UINT)bitmap.bmHeight, Bitmap.bmBits, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    //LONG lll = ::GetBitmapBits(hBitmap2, size, pBits2); //当图像尺寸为[249x180]和[250x180]时，该函数返回值有问题

    ::SelectObject(hdc_mem, hBitmapOld);
    ::DeleteDC(hdc_mem);
    ::DeleteObject(hBitmap2);
    //::ReleaseDC(this->m_hWnd, hdc);

    return 1;
}


//相当于合并图层
int CMyStatic::GetMergeBitmap(BITMAP &Bitmap, BOOL isDrawShape/*=FALSE*/)
{
    //this->CreateEmptyImage(Bitmap, m_bitmap.bmWidth, m_bitmap.bmHeight, m_bitmap.bmBitsPixel);
    this->GetBitampFromDC(Bitmap, isDrawShape);
    //this->picture_open_bitmap(Bitmap);

    return 1;
}


//画各种图形
int CMyStatic::picture_draw_shape(HDC hdc)
{
    int cnt = m_vector_draw_shapes.size();
    for (int i = 0; i < cnt; i++)
    {
        int cnt2 = m_vector_draw_shapes[i].vector_point.size();

        switch (m_vector_draw_shapes[i].draw_shape)
        {
        case DRAW_SHAPE_RECT:
        case DRAW_SHAPE_ELLIPSE:
            {
                for (int j = 0; j < cnt2; j += 2) //画每个矩形和椭圆都只需要2个点
                {
                    POINT pt1 = m_vector_draw_shapes[i].vector_point[j];
                    POINT pt2 = m_pt_mouse_move;
                    if (j + 1 < cnt2)
                    {
                        pt2 = m_vector_draw_shapes[i].vector_point[j + 1];
                    }
                    //默认pt1在pt2的左边
                    int x = min(pt1.x, pt2.x);
                    int y = min(pt1.y, pt2.y);
                    int w = abs(pt2.x - pt1.x) + 1;
                    int h = abs(pt2.y - pt1.y) + 1;

                    int r = m_vector_draw_shapes[i].vector_color[j].r;
                    int g = m_vector_draw_shapes[i].vector_color[j].g;
                    int b = m_vector_draw_shapes[i].vector_color[j].b;

                    if (m_vector_draw_shapes[i].draw_shape == DRAW_SHAPE_RECT)
                    {
                        Draw_Rect_DC(hdc, x, y, w, h, r, g, b, 1);
                    }
                    else if (m_vector_draw_shapes[i].draw_shape == DRAW_SHAPE_ELLIPSE)
                    {
                        Draw_Ellipse_DC(hdc, x, y, w, h, r, g, b, 1);
                    }
                }
                break;
            }
        case DRAW_SHAPE_LINE:
            {
                for (int j = 0; j < cnt2; j += 2) //画每条线段都只需要2个点
                {
                    POINT pt1 = m_vector_draw_shapes[i].vector_point[j];
                    POINT pt2 = m_pt_mouse_move;
                    if (j + 1 < cnt2)
                    {
                        pt2 = m_vector_draw_shapes[i].vector_point[j + 1];
                    }

                    int r = m_vector_draw_shapes[i].vector_color[j].r;
                    int g = m_vector_draw_shapes[i].vector_color[j].g;
                    int b = m_vector_draw_shapes[i].vector_color[j].b;

                    Draw_Line_DC(hdc, pt1, pt2, r, g, b, 1);
                }
                break;
            }
        case DRAW_SHAPE_LINES: //画折线
            {
                std::vector<POINT> vector_point = m_vector_draw_shapes[i].vector_point;
                if (m_Is_Popup_Right_Menu == FALSE && i == cnt - 1 && cnt2 > 0)
                {
                    vector_point.push_back(m_pt_mouse_move);
                }

                int r = m_vector_draw_shapes[i].vector_color[0].r;
                int g = m_vector_draw_shapes[i].vector_color[0].g;
                int b = m_vector_draw_shapes[i].vector_color[0].b;

                Draw_Polygon_DC(hdc, vector_point, r, g, b, 1);
                break;
            }
        case DRAW_SHAPE_POINTS: //画点
            {
                int r = m_vector_draw_shapes[i].vector_color[0].r;
                int g = m_vector_draw_shapes[i].vector_color[0].g;
                int b = m_vector_draw_shapes[i].vector_color[0].b;

                Draw_Points_DC(hdc, m_vector_draw_shapes[i].vector_point, r, g, b, 1);
                break;
            }
        case DRAW_SHAPE_POLYGON: //画一个封闭多边形
            {
                std::vector<POINT> vector_point = m_vector_draw_shapes[i].vector_point;
                if (m_Is_Popup_Right_Menu == FALSE && i == cnt - 1 && cnt2 > 0)
                {
                    vector_point.push_back(m_pt_mouse_move);
                }

                if (cnt2 >= 3)
                {
                    POINT pt1 = m_vector_draw_shapes[i].vector_point[cnt2 - 1];
                    POINT pt0 = m_vector_draw_shapes[i].vector_point[0]; //如果大于等于3个点，并且点击了右键，则将最后一个点和第一个点连接起来
                    if (!(pt0.x == pt1.x && pt0.y == pt1.y)) //暂时这么处理
                    {
                        vector_point.push_back(pt0);
                    }
                }
                int r = m_vector_draw_shapes[i].vector_color[0].r;
                int g = m_vector_draw_shapes[i].vector_color[0].g;
                int b = m_vector_draw_shapes[i].vector_color[0].b;

                Draw_Polygon_DC(hdc, vector_point, r, g, b, 1);
                break;
            }
        }
    }

    return 1;
}


//设置背景颜色
int CMyStatic::Set_Background_Color(int r, int g, int b)
{
    m_color_background.r = r;
    m_color_background.g = g;
    m_color_background.b = b;

    return 1;
}


//设置画线的颜色
int CMyStatic::Set_Draw_Shape_Line_Color(int r, int g, int b)
{
    m_color_draw_shape.r = r;
    m_color_draw_shape.g = g;
    m_color_draw_shape.b = b;

    return 1;
}


//设置画线形状
int CMyStatic::Set_Draw_Shape_Mode(KDraw_Shape draw_shape)
{
    m_Draw_Shape = draw_shape;

    return 1;
}
