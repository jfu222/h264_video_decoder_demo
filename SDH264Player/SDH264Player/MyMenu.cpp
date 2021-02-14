// MyListBoxMenu.cpp : 实现文件
//

#include "stdafx.h"
#include "MyMenu.h"


//=====================================================================
// CMyMenu

IMPLEMENT_DYNAMIC(CMyMenu, CWnd)

CMyMenu::CMyMenu()
{
    m_Wnd_Owner = NULL;
    m_rcWindow.SetRect(0, 0, 0, 0);
    m_is_show = FALSE;

    m_hFont = NULL;

    memset(&m_bitmap, 0, sizeof(BITMAP));
}


CMyMenu::~CMyMenu()
{
    if (m_hFont)
    {
        ::DeleteObject(m_hFont);
    }

    if (m_bitmap.bmBits != NULL)
    {
        delete[] m_bitmap.bmBits;
        m_bitmap.bmBits = NULL;
        memset(&m_bitmap, 0, sizeof(BITMAP));
    }
}


BEGIN_MESSAGE_MAP(CMyMenu, CWnd)
    ON_WM_PAINT()
    ON_WM_SHOWWINDOW()
    ON_WM_ACTIVATEAPP()
    ON_WM_CREATE()
    ON_MESSAGE(WM_MY_LBUTTONDOWN, &CMyMenu::OnMyLbuttondown)
    ON_WM_ERASEBKGND()
    ON_WM_KILLFOCUS()
    ON_WM_SIZE()
    ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// CMyMenu 消息处理程序


void CMyMenu::MyTrackPopupMenu(CWnd *pWnd, CPoint pt, CH264MacroBlock &mb, BITMAP &bitmap)
{
    m_Wnd_Owner = pWnd;

    m_mb = mb;
    m_bitmap = bitmap;

    CRect rc, rc2, rc3;
    pWnd->GetClientRect(rc2);
    pWnd->GetWindowRect(rc3);

    rc.top = rc3.top + 0; //pt.y;
    rc.bottom = rc.top + 680;
    rc.left = rc3.left - 310;
    rc.right = rc3.left;

    if (rc.left < 0)
    {
        rc.left = 0;
        rc.right = 310;
    }

    if (!::IsWindow(m_hWnd))
    {
        this->CreateEx(NULL, AfxRegisterWndClass(1), _T(""), WS_POPUP | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rc.left, rc.top, rc.Width(), rc.Height(), pWnd->GetParent()->GetSafeHwnd(), NULL);
    }
    else
    {
        if (m_is_show == FALSE)
        {
            CRect rcWindow, rc5;
            this->GetWindowRect(rcWindow);
            if (rcWindow.Height() != 0)
            {
                rc5 = rcWindow;

                rcWindow.top = rc3.top + 0; //pt.y;
                rcWindow.bottom = rcWindow.top + 680;
                rcWindow.left = rc3.left - 310;
                rcWindow.right = rc3.left;

                if (rcWindow.left < 0)
                {
                    rcWindow.left = 0;
                    rcWindow.right = 310;
                }

                this->MoveWindow(rcWindow, 0);
                this->ShowWindow(SW_SHOW);
            }
        }
    }

    this->SetFocus();
}


void CMyMenu::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: 在此处添加消息处理程序代码
    // 不为绘图消息调用 CWnd::OnPaint()

    CRect rcClient, rcWindow;
    ::GetWindowRect(this->m_hWnd, &rcWindow);
    this->GetClientRect(&rcClient);

    HDC hdc_mem = ::CreateCompatibleDC(dc.m_hDC);
    HBITMAP hBitmap = ::CreateCompatibleBitmap(dc.m_hDC, rcClient.Width(), rcClient.Height());
    HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdc_mem, hBitmap);

    HBRUSH hbr;
    hbr = ::CreateSolidBrush(RGB(15, 54, 123)); //黑色背景画刷
    ::FillRect(hdc_mem, &rcClient, hbr);
    ::DeleteObject(hbr);

    int ret = hdc_paint(hdc_mem, 0, 0, 112, 112, m_bitmap);

    ::BitBlt(dc.m_hDC, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), hdc_mem, 0, 0, SRCCOPY); //内存位图拷贝

    ::SelectObject(hdc_mem, hBitmapOld);
    ::DeleteDC(hdc_mem);
    //::DeleteObject(hBitmapOld);
    ::DeleteObject(hBitmap);
}


//视频画图
int CMyMenu::hdc_paint(HDC hdc, int dst_x, int dst_y, int dst_width, int dst_height, BITMAP &bitmap)
{
    BITMAP bmp;
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

    //::BitBlt(hdc, dst_x, dst_y, dst_width, dst_height, hdc_mem, 0, 0, SRCCOPY); //内存位图拷贝

    ::SetStretchBltMode(hdc, COLORONCOLOR); //设置指定设备环境中的位图拉伸模式
    ::StretchBlt(hdc, dst_x, dst_y, dst_width, dst_height, hdc_mem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, SRCCOPY); //内存位图拷贝(有拉伸)

    //---------------------------------
    SetBkMode(hdc, TRANSPARENT);

    HFONT hFontOld = (HFONT)::SelectObject(hdc, m_hFont);
    COLORREF ColorOld = ::SetTextColor(hdc, RGB(255, 255, 255));

    char str[100] = { 0 };
    std::string str2 = "";
    int x = dst_x + 0 + 5;
    int y = dst_y + 0 + 5;
    int text_h = 14;
    int text_h2 = 0;
    int x_offset = 116;
    
    str2 = "16x16宏块坐标：x=" + std::to_string(m_mb.m_mb_position_x) + "; y=" + std::to_string(m_mb.m_mb_position_y);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "CurrMbAddr：" + std::to_string(m_mb.CurrMbAddr);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "mb_type：" + std::to_string(m_mb.mb_type) + " = " + m_mb.getNameOfMbTypeStr(m_mb.m_name_of_mb_type);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = std::string("mb_pred_mode：") + (H264_MB_PART_PRED_MODE_TO_STR(m_mb.m_mb_pred_mode));
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "transform_size_8x8_flag：" + std::to_string(m_mb.transform_size_8x8_flag);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "coded_block_pattern：" + std::to_string(m_mb.coded_block_pattern);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "mb_qp_delta：" + std::to_string(m_mb.mb_qp_delta);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "mb_field_decoding_flag：" + std::to_string(m_mb.mb_field_decoding_flag);
    ::TextOut(hdc, x + x_offset, y, str2.c_str(), str2.length());
    
    //-------------------------
    y = dst_y + dst_height + 5;
    str2 = "prev_intra4x4_pred_mode_flag: ";
    for (int i = 0; i < 16; ++i)
    {
        str2 += std::to_string(m_mb.prev_intra4x4_pred_mode_flag[i]);
    }
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "rem_intra4x4_pred_mode: {" 
        + std::to_string(m_mb.rem_intra4x4_pred_mode[0]) + ","
        + std::to_string(m_mb.rem_intra4x4_pred_mode[1]) + ","
        + std::to_string(m_mb.rem_intra4x4_pred_mode[2]) + ","
        + std::to_string(m_mb.rem_intra4x4_pred_mode[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "rem_intra8x8_pred_mode: {" 
        + std::to_string(m_mb.rem_intra8x8_pred_mode[0]) + ","
        + std::to_string(m_mb.rem_intra8x8_pred_mode[1]) + ","
        + std::to_string(m_mb.rem_intra8x8_pred_mode[2]) + ","
        + std::to_string(m_mb.rem_intra8x8_pred_mode[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "intra_chroma_pred_mode：" + std::to_string(m_mb.intra_chroma_pred_mode);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "ref_idx_l0: {" 
        + std::to_string(m_mb.ref_idx_l0[0]) + ","
        + std::to_string(m_mb.ref_idx_l0[1]) + ","
        + std::to_string(m_mb.ref_idx_l0[2]) + ","
        + std::to_string(m_mb.ref_idx_l0[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "ref_idx_l1: {" 
        + std::to_string(m_mb.ref_idx_l1[0]) + ","
        + std::to_string(m_mb.ref_idx_l1[1]) + ","
        + std::to_string(m_mb.ref_idx_l1[2]) + ","
        + std::to_string(m_mb.ref_idx_l1[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "mvd_l0: ";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    for (int i = 0; i < 4; ++i)
    {
        str2 = "|";
        for (int j = 0; j < 4; ++j)
        {
            str2 += std::to_string(m_mb.mvd_l0[i][j][0]) + "_" + std::to_string(m_mb.mvd_l0[i][j][1]);
            if (j != 3)
            {
                str2 += ",";
            }
        }
        str2 += "|";
        ::TextOut(hdc, x + 50, y, str2.c_str(), str2.length());
        y += text_h;
    }
    
    y += 2;
    str2 = "mvd_l1: ";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    for (int i = 0; i < 4; ++i)
    {
        str2 = "|";
        for (int j = 0; j < 4; ++j)
        {
            str2 += std::to_string(m_mb.mvd_l1[i][j][0]) + "_" + std::to_string(m_mb.mvd_l1[i][j][1]);
            if (j != 3)
            {
                str2 += ",";
            }
        }
        str2 += "|";
        ::TextOut(hdc, x + 50, y, str2.c_str(), str2.length());
        y += text_h;
    }
    
    y += 2;
    str2 = "sub_mb_type: {" 
        + std::to_string(m_mb.sub_mb_type[0]) + "=" + m_mb.getNameOfMbTypeStr(m_mb.m_name_of_sub_mb_type[0]) + ","
        + std::to_string(m_mb.sub_mb_type[1]) + "=" + m_mb.getNameOfMbTypeStr(m_mb.m_name_of_sub_mb_type[1]) + ",";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    y += text_h;
      str2 = std::to_string(m_mb.sub_mb_type[2]) + "=" + m_mb.getNameOfMbTypeStr(m_mb.m_name_of_sub_mb_type[2]) + ","
        + std::to_string(m_mb.sub_mb_type[3]) + "=" + m_mb.getNameOfMbTypeStr(m_mb.m_name_of_sub_mb_type[3]) + "}";
    ::TextOut(hdc, x + 50, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "field_pic_flag：" + std::to_string(m_mb.field_pic_flag);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "bottom_field_flag：" + std::to_string(m_mb.bottom_field_flag);
    ::TextOut(hdc, x + 120, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "mb_skip_flag：" + std::to_string(m_mb.mb_skip_flag);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "MbaffFrameFlag：" + std::to_string(m_mb.MbaffFrameFlag);
    ::TextOut(hdc, x + 120, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "constrained_intra_pred_flag：" + std::to_string(m_mb.constrained_intra_pred_flag);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "disable_deblocking_filter_idc：" + std::to_string(m_mb.disable_deblocking_filter_idc);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "slice_number：" + std::to_string(m_mb.slice_number);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "m_NumMbPart：" + std::to_string(m_mb.m_NumMbPart);
    ::TextOut(hdc, x+ 120, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "MbPartWidth：" + std::to_string(m_mb.MbPartWidth);
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "MbPartHeight：" + std::to_string(m_mb.MbPartHeight);
    ::TextOut(hdc, x+ 120, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "NumSubMbPart: {" 
        + std::to_string(m_mb.NumSubMbPart[0]) + ","
        + std::to_string(m_mb.NumSubMbPart[1]) + ","
        + std::to_string(m_mb.NumSubMbPart[2]) + ","
        + std::to_string(m_mb.NumSubMbPart[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "SubMbPartWidth: {" 
        + std::to_string(m_mb.SubMbPartWidth[0]) + ","
        + std::to_string(m_mb.SubMbPartWidth[1]) + ","
        + std::to_string(m_mb.SubMbPartWidth[2]) + ","
        + std::to_string(m_mb.SubMbPartWidth[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "SubMbPartHeight: {" 
        + std::to_string(m_mb.SubMbPartHeight[0]) + ","
        + std::to_string(m_mb.SubMbPartHeight[1]) + ","
        + std::to_string(m_mb.SubMbPartHeight[2]) + ","
        + std::to_string(m_mb.SubMbPartHeight[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = std::string("name_of_slice_type_fixed：") + (H264_SLIECE_TYPE_TO_STR(m_mb.m_slice_type_fixed));
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = std::string("sub_mb_pred_mode:{") 
        + (H264_MB_PART_PRED_MODE_TO_STR(m_mb.m_sub_mb_pred_mode[0])) + ","
        + (H264_MB_PART_PRED_MODE_TO_STR(m_mb.m_sub_mb_pred_mode[1])) + ",";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    y += text_h;
    str2 = std::string("") +  (H264_MB_PART_PRED_MODE_TO_STR(m_mb.m_sub_mb_pred_mode[2])) + ","
        + (H264_MB_PART_PRED_MODE_TO_STR(m_mb.m_sub_mb_pred_mode[3])) + "}";
    ::TextOut(hdc, x + 60, y, str2.c_str(), str2.length());
    
    //-------------------------------
    y += text_h;
    str2 = "m_MvL0: ";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    for (int i = 0; i < 4; ++i)
    {
        str2 = "|";
        for (int j = 0; j < 4; ++j)
        {
            str2 += std::to_string(m_mb.m_MvL0[i][j][0]) + "_" + std::to_string(m_mb.m_MvL0[i][j][1]);
            if (j != 3)
            {
                str2 += ",";
            }
        }
        str2 += "|";
        ::TextOut(hdc, x + 50, y, str2.c_str(), str2.length());
        y += text_h;
    }
    
    //-------------------------------
    y += 2;
    str2 = "m_MvL1: ";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    for (int i = 0; i < 4; ++i)
    {
        str2 = "|";
        for (int j = 0; j < 4; ++j)
        {
            str2 += std::to_string(m_mb.m_MvL1[i][j][0]) + "_" + std::to_string(m_mb.m_MvL1[i][j][1]);
            if (j != 3)
            {
                str2 += ",";
            }
        }
        str2 += "|";
        ::TextOut(hdc, x + 50, y, str2.c_str(), str2.length());
        y += text_h;
    }
    
    //------------------
    y += 2;
    str2 = "m_RefIdxL0: {" 
        + std::to_string(m_mb.m_RefIdxL0[0]) + ","
        + std::to_string(m_mb.m_RefIdxL0[1]) + ","
        + std::to_string(m_mb.m_RefIdxL0[2]) + ","
        + std::to_string(m_mb.m_RefIdxL0[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "m_RefIdxL1: {" 
        + std::to_string(m_mb.m_RefIdxL1[0]) + ","
        + std::to_string(m_mb.m_RefIdxL1[1]) + ","
        + std::to_string(m_mb.m_RefIdxL1[2]) + ","
        + std::to_string(m_mb.m_RefIdxL1[3]) + "}";
    ::TextOut(hdc, x + 150, y, str2.c_str(), str2.length());
    
    y += text_h;
    str2 = "m_PredFlagL0: {" 
        + std::to_string(m_mb.m_PredFlagL0[0]) + ","
        + std::to_string(m_mb.m_PredFlagL0[1]) + ","
        + std::to_string(m_mb.m_PredFlagL0[2]) + ","
        + std::to_string(m_mb.m_PredFlagL0[3]) + "}";
    ::TextOut(hdc, x, y, str2.c_str(), str2.length());
    
    str2 = "m_PredFlagL1: {" 
        + std::to_string(m_mb.m_PredFlagL1[0]) + ","
        + std::to_string(m_mb.m_PredFlagL1[1]) + ","
        + std::to_string(m_mb.m_PredFlagL1[2]) + ","
        + std::to_string(m_mb.m_PredFlagL1[3]) + "}";
    ::TextOut(hdc, x + 150, y, str2.c_str(), str2.length());
    
    //------------------------------------
    ::SelectObject(hdc, hFontOld);
    ::SetTextColor(hdc, ColorOld);

    ::SelectObject(hdc_mem, hBitmapOld);
    ::DeleteDC(hdc_mem);
    ::DeleteObject(hBitmap2);

    return 1;
}


void CMyMenu::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CWnd::OnShowWindow(bShow, nStatus);

    // TODO: 在此处添加消息处理程序代码
    if (bShow)
    {
        m_is_show = TRUE;
    }
    else
    {
        m_is_show = FALSE;
    }
}


void CMyMenu::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
    CWnd::OnActivateApp(bActive, dwThreadID);

    // TODO: 在此处添加消息处理程序代码
    if (m_is_show == TRUE)
    {

    }
}


int CMyMenu::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码
    CRect rc;
    this->GetClientRect(rc);

    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = 12; // request a 12-pixel-height font
    lf.lfWidth = 6; //字体宽度计算公式=[((lf.lfWidth/12)+1)*8]像素，即为8的倍数
    lf.lfWeight = FW_NORMAL;
    lstrcpy(lf.lfFaceName, _T("宋体"));
    lf.lfCharSet = GB2312_CHARSET;

    m_hFont = ::CreateFontIndirect(&lf);

    return 0;
}

afx_msg LRESULT CMyMenu::OnMyLbuttondown(WPARAM wParam, LPARAM lParam)
{
    CRect rcWindow, rcWindowOwner;
    this->GetWindowRect(rcWindow);
    m_Wnd_Owner->GetWindowRect(rcWindowOwner);

    CPoint pt;
    ::GetCursorPos(&pt);

    if (!rcWindow.PtInRect(pt) && !rcWindowOwner.PtInRect(pt))
    {

    }

    return 0;
}

BOOL CMyMenu::OnEraseBkgnd(CDC* pDC)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值

    return TRUE;
    //return CWnd::OnEraseBkgnd(pDC);
}


void CMyMenu::OnKillFocus(CWnd* pNewWnd)
{
    CWnd::OnKillFocus(pNewWnd);

    // TODO: 在此处添加消息处理程序代码
}


void CMyMenu::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);

    // TODO: 在此处添加消息处理程序代码
}


void CMyMenu::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    //int index=m_MouseMoveItem;
    int index = 0;

    switch (nChar)
    {
    case VK_LEFT:
    case VK_UP:
    case VK_RIGHT:
    case VK_DOWN:
    case VK_RETURN:
        {

            return;
        }
    }

    CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
