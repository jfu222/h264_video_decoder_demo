// SDH264PlayerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDH264Player.h"
#include "SDH264PlayerDlg.h"
#include "afxdialogex.h"
#include "FrameInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

    // Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSDH264PlayerDlg dialog



CSDH264PlayerDlg::CSDH264PlayerDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CSDH264PlayerDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSDH264PlayerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_PLAYER, m_static_player);
}

BEGIN_MESSAGE_MAP(CSDH264PlayerDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CSDH264PlayerDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CSDH264PlayerDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CSDH264PlayerDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &CSDH264PlayerDlg::OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON5, &CSDH264PlayerDlg::OnBnClickedButton5)
    ON_WM_SIZE()
    ON_MESSAGE(WM_DECODER_THREAD_END, &CSDH264PlayerDlg::OnDecoderThreadEnd)
    ON_MESSAGE(WM_LOOK_PICTURE_OPEN, &CSDH264PlayerDlg::OnLookPictureOpen)
END_MESSAGE_MAP()


// CSDH264PlayerDlg message handlers

BOOL CSDH264PlayerDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // Add "About..." menu item to system menu.

    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    // TODO: Add extra initialization here

    InitCtrl();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSDH264PlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSDH264PlayerDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogEx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSDH264PlayerDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CSDH264PlayerDlg::OnBnClickedButton1() //开始
{
    // TODO: Add your control notification handler code here
    int ret = m_static_player.stop_play(); //先关闭
    if (ret != 0){ return; }

    CString str;
    this->GetDlgItem(IDC_EDIT1)->GetWindowText(str);

    m_str_url = str;

    //-------------------------------------
    ret = m_static_player.start_play(CT2A(str));
    if (ret != 0){ return; }

    m_isPause = false;
    this->GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("暂停"));

    this->GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
    this->GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
    this->GetDlgItem(IDC_BUTTON3)->EnableWindow(TRUE);
}


void CSDH264PlayerDlg::OnBnClickedButton2() //暂停
{
    // TODO: Add your control notification handler code here
    int ret = 0;
    if (m_isPause == true)
    {
        ret = m_static_player.pause_play(false);
        m_isPause = false;
        this->GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("暂停"));
    }
    else
    {
        ret = m_static_player.pause_play(true);
        m_isPause = true;
        this->GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("播放"));
    }
}


void CSDH264PlayerDlg::OnBnClickedButton3() //关闭
{
    // TODO: Add your control notification handler code here
    int ret = m_static_player.stop_play();

    m_isPause = false;
    this->GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("暂停"));

    this->GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
    this->GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
    this->GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
}


void CSDH264PlayerDlg::OnBnClickedButton4() //下一帧
{
    // TODO: Add your control notification handler code here
}


void CSDH264PlayerDlg::OnBnClickedButton5() //帧信息
{
    // TODO: Add your control notification handler code here
    CString str = _T("");

    GetFrameInfo(str);

    CFrameInfoDlg dlg(str);

    dlg.DoModal();

}


void CSDH264PlayerDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogEx::OnSize(nType, cx, cy);
    
    CalcWindowPos();
    // TODO: Add your message handler code here
}


int CSDH264PlayerDlg::InitCtrl()
{
    //----------------------------------------------
    m_static_player.SetHwndParent(this->m_hWnd);

    m_str_url = _T("");
    m_isPause = false;

    CString str = _T("C:\\HeavyHand_1080p.B_frames_cabac_tff.h264");

    m_str_url = str;

    this->GetDlgItem(IDC_STATIC3)->SetWindowText(_T(""));
    this->GetDlgItem(IDC_EDIT1)->SetWindowText(str);

    this->MoveWindow(0, 0, 1200, 700);
    this->CenterWindow();

    this->GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
    this->GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
    this->GetDlgItem(IDC_BUTTON3)->EnableWindow(FALSE);
    this->GetDlgItem(IDC_BUTTON4)->EnableWindow(FALSE);

    ((CButton *)(this->GetDlgItem(IDC_CHECK1)))->SetCheck(FALSE); //默认不勾选

    ::DragAcceptFiles(m_static_player.m_hWnd, TRUE); //视频播放窗口， hWnd是接收拖放的窗口句柄

    CalcWindowPos(); //调整各个子窗口位置

    return 0;
}


int CSDH264PlayerDlg::CalcWindowPos()
{
    if (!::IsWindow(m_static_player.m_hWnd))
    {
        return -1;
    }

    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    CRect rcClient;
    this->GetClientRect(&rcClient);

    w = rcClient.Width();
    h = rcClient.Height() - 80;
    
    //------------------
    x = 0;
    y = 0;
    this->GetDlgItem(IDC_STATIC3)->MoveWindow(x, y, w, 20, 1);

    //------------------
    m_static_player.MoveWindow(x, y + 20, w, h - 20, 1);

    //-------------------------------------
    y = h + 5;
    h = 26;
    this->GetDlgItem(IDC_STATIC0)->MoveWindow(x + 10, y + 5, 70, h, 1);
    this->GetDlgItem(IDC_EDIT1)->MoveWindow(x + 80, y, w - 100, h, 1);

    x += 100;
    y += 35;
    this->GetDlgItem(IDC_BUTTON1)->MoveWindow(x, y, 60, h, 1);

    x += 120;
    this->GetDlgItem(IDC_BUTTON2)->MoveWindow(x, y, 60, h, 1);

    x += 120;
    this->GetDlgItem(IDC_BUTTON3)->MoveWindow(x, y, 60, h, 1);

    x += 120;
    this->GetDlgItem(IDC_BUTTON4)->MoveWindow(x, y, 60, h, 1);

    x += 120;
    this->GetDlgItem(IDC_BUTTON5)->MoveWindow(x, y, 60, h, 1);
    
    x += 120;
    this->GetDlgItem(IDC_CHECK1)->MoveWindow(x, y, 76, h, 1);
    
    x += 120;
    this->GetDlgItem(IDC_CHECK2)->MoveWindow(x, y, 76, h, 1);

    return 0;
}


afx_msg LRESULT CSDH264PlayerDlg::OnDecoderThreadEnd(WPARAM wParam, LPARAM lParam)
{
    int ret = (int)wParam;

    if (ret != 0)
    {
        this->MessageBox(_T("H264视频文件播放失败"), _T("错误"), MB_OK);
    }

    this->OnBnClickedButton3(); //关闭

    return 0;
}


afx_msg LRESULT CSDH264PlayerDlg::OnLookPictureOpen(WPARAM wParam, LPARAM lParam)
{
    char * file = (char *)wParam;
    HWND hWnd = (HWND)lParam;
    
    this->OnBnClickedButton3(); //先关闭

    this->GetDlgItem(IDC_EDIT1)->SetWindowTextA(file);
    
    this->OnBnClickedButton1(); //开始

    return 0;
}


int CSDH264PlayerDlg::GetFrameInfo(CString &str)
{
    CString str2;

    str = _T("url: ") + m_str_url + _T("\r\n\r\n");

    CH264PictureBase & pic = m_static_player.m_outPicture.m_picture_frame;
    CH264SliceHeader & slice_header = m_static_player.m_outPicture.m_picture_frame.m_h264_slice_header;
    CH264SPS & sps = m_static_player.m_outPicture.m_picture_frame.m_h264_slice_header.m_sps;
    CH264PPS & pps = m_static_player.m_outPicture.m_picture_frame.m_h264_slice_header.m_pps;
    
    str2.Format(_T("frame_num_cnt: %d\r\n"), m_static_player.m_outPicture.m_picture_frame.m_PicNumCnt); str += str2;
    str2.Format(_T("frame_type: %s\r\n"), H264_SLIECE_TYPE_TO_STR(slice_header.slice_type)); str += str2;
    
    //----------------------------------
    CString profile_str = _T("");
    int & profile_idc = sps.profile_idc;
    int & level_idc = sps.level_idc;
    
    switch (sps.profile_idc)
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
    str2.Format(_T("profile: %s\r\n"), profile_str); str += str2;
    
    str2.Format(_T("video resolution: %d x %d\r\n"), m_static_player.m_outPicture.m_picture_frame.PicWidthInSamplesL, 
        m_static_player.m_outPicture.m_picture_frame.PicHeightInSamplesL); str += str2;

    str2.Format(_T("\r\n---------SPS info------------\r\n")); str += str2;
    str2.Format(_T("profile_idc: %d\r\n"), sps.profile_idc); str += str2;
    str2.Format(_T("constraint_set0_flag: %d\r\n"), sps.constraint_set0_flag); str += str2;
    str2.Format(_T("constraint_set1_flag: %d\r\n"), sps.constraint_set1_flag); str += str2;
    str2.Format(_T("constraint_set2_flag: %d\r\n"), sps.constraint_set2_flag); str += str2;
    str2.Format(_T("constraint_set3_flag: %d\r\n"), sps.constraint_set3_flag); str += str2;
    str2.Format(_T("constraint_set4_flag: %d\r\n"), sps.constraint_set4_flag); str += str2;
    str2.Format(_T("constraint_set5_flag: %d\r\n"), sps.constraint_set5_flag); str += str2;
    str2.Format(_T("reserved_zero_2bits: %d\r\n"), sps.reserved_zero_2bits); str += str2;
    str2.Format(_T("level_idc: %d\r\n"), sps.level_idc); str += str2;
    str2.Format(_T("seq_parameter_set_id: %d\r\n"), sps.seq_parameter_set_id); str += str2;
    str2.Format(_T("chroma_format_idc: %d\r\n"), sps.chroma_format_idc); str += str2;
    str2.Format(_T("separate_colour_plane_flag: %d\r\n"), sps.separate_colour_plane_flag); str += str2;
    str2.Format(_T("bit_depth_luma_minus8: %d\r\n"), sps.bit_depth_luma_minus8); str += str2;
    str2.Format(_T("bit_depth_chroma_minus8: %d\r\n"), sps.bit_depth_chroma_minus8); str += str2;
    str2.Format(_T("qpprime_y_zero_transform_bypass_flag: %d\r\n"), sps.qpprime_y_zero_transform_bypass_flag); str += str2;
    str2.Format(_T("seq_scaling_matrix_present_flag: %d\r\n"), sps.seq_scaling_matrix_present_flag); str += str2;

    str2 = _T("seq_scaling_list_present_flag[0..11]: ");
    for (int i = 0; i < 12; ++i) { CString str3 = _T(""); str3.Format(_T("%d"), sps.seq_scaling_list_present_flag[i]); str2 += str3; } str += str2 + _T("\r\n");

    str2.Format(_T("log2_max_frame_num_minus4: %d\r\n"), sps.log2_max_frame_num_minus4); str += str2;
    str2.Format(_T("pic_order_cnt_type: %d\r\n"), sps.pic_order_cnt_type); str += str2;
    str2.Format(_T("log2_max_pic_order_cnt_lsb_minus4: %d\r\n"), sps.log2_max_pic_order_cnt_lsb_minus4); str += str2;
    str2.Format(_T("delta_pic_order_always_zero_flag: %d\r\n"), sps.delta_pic_order_always_zero_flag); str += str2;
    str2.Format(_T("offset_for_non_ref_pic: %d\r\n"), sps.offset_for_non_ref_pic); str += str2;
    str2.Format(_T("offset_for_top_to_bottom_field: %d\r\n"), sps.offset_for_top_to_bottom_field); str += str2;
    str2.Format(_T("num_ref_frames_in_pic_order_cnt_cycle: %d\r\n"), sps.num_ref_frames_in_pic_order_cnt_cycle); str += str2;

    str2.Format(_T("offset_for_ref_frame[0..%d]: "), sps.num_ref_frames_in_pic_order_cnt_cycle);
    for (int i = 0; i < sps.num_ref_frames_in_pic_order_cnt_cycle; ++i) { CString str3 = _T(""); str3.Format(_T("%d"), sps.offset_for_ref_frame[i]); str2 += str3; } str += str2 + _T("\r\n");

    str2.Format(_T("max_num_ref_frames: %d\r\n"), sps.max_num_ref_frames); str += str2;
    str2.Format(_T("gaps_in_frame_num_value_allowed_flag: %d\r\n"), sps.gaps_in_frame_num_value_allowed_flag); str += str2;
    str2.Format(_T("pic_width_in_mbs_minus1: %d\r\n"), sps.pic_width_in_mbs_minus1); str += str2;
    str2.Format(_T("pic_height_in_map_units_minus1: %d\r\n"), sps.pic_height_in_map_units_minus1); str += str2;
    str2.Format(_T("frame_mbs_only_flag: %d\r\n"), sps.frame_mbs_only_flag); str += str2;
    str2.Format(_T("mb_adaptive_frame_field_flag: %d\r\n"), sps.mb_adaptive_frame_field_flag); str += str2;
    str2.Format(_T("direct_8x8_inference_flag: %d\r\n"), sps.direct_8x8_inference_flag); str += str2;
    str2.Format(_T("frame_cropping_flag: %d\r\n"), sps.frame_cropping_flag); str += str2;
    str2.Format(_T("frame_crop_left_offset: %d\r\n"), sps.frame_crop_left_offset); str += str2;
    str2.Format(_T("frame_crop_right_offset: %d\r\n"), sps.frame_crop_right_offset); str += str2;
    str2.Format(_T("frame_crop_top_offset: %d\r\n"), sps.frame_crop_top_offset); str += str2;
    str2.Format(_T("frame_crop_bottom_offset: %d\r\n"), sps.frame_crop_bottom_offset); str += str2;
    str2.Format(_T("vui_parameters_present_flag: %d\r\n"), sps.vui_parameters_present_flag); str += str2;
    
    str2.Format(_T("\r\n---------SPS-vui-m_vui info------------\r\n")); str += str2;
    str2.Format(_T("aspect_ratio_info_present_flag: %d\r\n"), sps.m_vui.aspect_ratio_info_present_flag); str += str2;
    str2.Format(_T("aspect_ratio_idc: %d\r\n"), sps.m_vui.aspect_ratio_idc); str += str2;
    str2.Format(_T("sar_width: %d\r\n"), sps.m_vui.sar_width); str += str2;
    str2.Format(_T("sar_height: %d\r\n"), sps.m_vui.sar_height); str += str2;
    str2.Format(_T("overscan_info_present_flag: %d\r\n"), sps.m_vui.overscan_info_present_flag); str += str2;
    str2.Format(_T("overscan_appropriate_flag: %d\r\n"), sps.m_vui.overscan_appropriate_flag); str += str2;
    str2.Format(_T("video_signal_type_present_flag: %d\r\n"), sps.m_vui.video_signal_type_present_flag); str += str2;
    str2.Format(_T("video_format: %d\r\n"), sps.m_vui.video_format); str += str2;
    str2.Format(_T("video_full_range_flag: %d\r\n"), sps.m_vui.video_full_range_flag); str += str2;
    str2.Format(_T("colour_description_present_flag: %d\r\n"), sps.m_vui.colour_description_present_flag); str += str2;
    str2.Format(_T("colour_primaries: %d\r\n"), sps.m_vui.colour_primaries); str += str2;
    str2.Format(_T("transfer_characteristics: %d\r\n"), sps.m_vui.transfer_characteristics); str += str2;
    str2.Format(_T("matrix_coefficients: %d\r\n"), sps.m_vui.matrix_coefficients); str += str2;
    str2.Format(_T("chroma_loc_info_present_flag: %d\r\n"), sps.m_vui.chroma_loc_info_present_flag); str += str2;
    str2.Format(_T("chroma_sample_loc_type_top_field: %d\r\n"), sps.m_vui.chroma_sample_loc_type_top_field); str += str2;
    str2.Format(_T("chroma_sample_loc_type_bottom_field: %d\r\n"), sps.m_vui.chroma_sample_loc_type_bottom_field); str += str2;
    str2.Format(_T("timing_info_present_flag: %d\r\n"), sps.m_vui.timing_info_present_flag); str += str2;
    str2.Format(_T("num_units_in_tick: %d\r\n"), sps.m_vui.num_units_in_tick); str += str2;
    str2.Format(_T("time_scale: %d\r\n"), sps.m_vui.time_scale); str += str2;
    str2.Format(_T("fixed_frame_rate_flag: %d\r\n"), sps.m_vui.fixed_frame_rate_flag); str += str2;
    str2.Format(_T("nal_hrd_parameters_present_flag: %d\r\n"), sps.m_vui.nal_hrd_parameters_present_flag); str += str2;
    str2.Format(_T("vcl_hrd_parameters_present_flag: %d\r\n"), sps.m_vui.vcl_hrd_parameters_present_flag); str += str2;
    str2.Format(_T("low_delay_hrd_flag: %d\r\n"), sps.m_vui.low_delay_hrd_flag); str += str2;
    str2.Format(_T("pic_struct_present_flag: %d\r\n"), sps.m_vui.pic_struct_present_flag); str += str2;
    str2.Format(_T("bitstream_restriction_flag: %d\r\n"), sps.m_vui.bitstream_restriction_flag); str += str2;
    str2.Format(_T("motion_vectors_over_pic_boundaries_flag: %d\r\n"), sps.m_vui.motion_vectors_over_pic_boundaries_flag); str += str2;
    str2.Format(_T("max_bytes_per_pic_denom: %d\r\n"), sps.m_vui.max_bytes_per_pic_denom); str += str2;
    str2.Format(_T("max_bits_per_mb_denom: %d\r\n"), sps.m_vui.max_bits_per_mb_denom); str += str2;
    str2.Format(_T("log2_max_mv_length_horizontal: %d\r\n"), sps.m_vui.log2_max_mv_length_horizontal); str += str2;
    str2.Format(_T("log2_max_mv_length_vertical: %d\r\n"), sps.m_vui.log2_max_mv_length_vertical); str += str2;
    str2.Format(_T("max_num_reorder_frames: %d\r\n"), sps.m_vui.max_num_reorder_frames); str += str2;
    str2.Format(_T("max_dec_frame_buffering: %d\r\n"), sps.m_vui.max_dec_frame_buffering); str += str2;
    
    str2.Format(_T("\r\n---------------------\r\n")); str += str2;
    str2.Format(_T("PicWidthInMbs: %d\r\n"), sps.PicWidthInMbs); str += str2;
    str2.Format(_T("FrameHeightInMbs: %d\r\n"), sps.FrameHeightInMbs); str += str2;
    str2.Format(_T("PicHeightInMapUnits: %d\r\n"), sps.PicHeightInMapUnits); str += str2;
    str2.Format(_T("PicSizeInMapUnits: %d\r\n"), sps.PicSizeInMapUnits); str += str2;
    str2.Format(_T("ChromaArrayType: %d\r\n"), sps.ChromaArrayType); str += str2;
    str2.Format(_T("BitDepthY: %d\r\n"), sps.BitDepthY); str += str2;
    str2.Format(_T("QpBdOffsetY: %d\r\n"), sps.QpBdOffsetY); str += str2;
    str2.Format(_T("BitDepthC: %d\r\n"), sps.BitDepthC); str += str2;
    str2.Format(_T("QpBdOffsetC: %d\r\n"), sps.QpBdOffsetC); str += str2;
    str2.Format(_T("SubWidthC: %d\r\n"), sps.SubWidthC); str += str2;
    str2.Format(_T("SubHeightC: %d\r\n"), sps.SubHeightC); str += str2;
    str2.Format(_T("MbWidthC: %d\r\n"), sps.MbWidthC); str += str2;
    str2.Format(_T("MbHeightC: %d\r\n"), sps.MbHeightC); str += str2;
    str2.Format(_T("Chroma_Format: %d\r\n"), sps.Chroma_Format); str += str2;
    str2.Format(_T("PicWidthInSamplesL: %d\r\n"), sps.PicWidthInSamplesL); str += str2;
    str2.Format(_T("PicWidthInSamplesC: %d\r\n"), sps.PicWidthInSamplesC); str += str2;
    str2.Format(_T("fps: %.2f\r\n"), sps.fps); str += str2;

    str2.Format(_T("\r\n---------PPS info------------\r\n")); str += str2;
    str2.Format(_T("pic_parameter_set_id: %d\r\n"), pps.pic_parameter_set_id); str += str2;
    str2.Format(_T("seq_parameter_set_id: %d\r\n"), pps.seq_parameter_set_id); str += str2;
    str2.Format(_T("entropy_coding_mode_flag: %d (%s)\r\n"), pps.entropy_coding_mode_flag, pps.entropy_coding_mode_flag ? _T("cabac") : _T("cavlc")); str += str2;
    str2.Format(_T("bottom_field_pic_order_in_frame_present_flag: %d\r\n"), pps.bottom_field_pic_order_in_frame_present_flag); str += str2;
    str2.Format(_T("num_slice_groups_minus1: %d\r\n"), pps.num_slice_groups_minus1); str += str2;
    str2.Format(_T("slice_group_map_type: %d\r\n"), pps.slice_group_map_type); str += str2;

    str2 = _T("run_length_minus1[0..7]:");
    for (int i = 0; i < 8; ++i){ CString str3 = _T(""); str3.Format(_T(" %d"), pps.run_length_minus1[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("top_left[0..7]:");
    for (int i = 0; i < 8; ++i){ CString str3 = _T(""); str3.Format(_T(" %d"), pps.top_left[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("bottom_right[0..7]:");
    for (int i = 0; i < 8; ++i){ CString str3 = _T(""); str3.Format(_T(" %d"), pps.bottom_right[i]); str2 += str3; } str += str2 + _T("\r\n");

    str2.Format(_T("slice_group_change_direction_flag: %d\r\n"), pps.slice_group_change_direction_flag); str += str2;
    str2.Format(_T("slice_group_change_rate_minus1: %d\r\n"), pps.slice_group_change_rate_minus1); str += str2;
    str2.Format(_T("pic_size_in_map_units_minus1: %d\r\n"), pps.pic_size_in_map_units_minus1); str += str2;
    str2.Format(_T("slice_group_id: %d\r\n"), pps.slice_group_id); str += str2;
    str2.Format(_T("num_ref_idx_l0_default_active_minus1: %d\r\n"), pps.num_ref_idx_l0_default_active_minus1); str += str2;
    str2.Format(_T("num_ref_idx_l1_default_active_minus1: %d\r\n"), pps.num_ref_idx_l1_default_active_minus1); str += str2;
    str2.Format(_T("weighted_pred_flag: %d\r\n"), pps.weighted_pred_flag); str += str2;
    str2.Format(_T("weighted_bipred_idc: %d\r\n"), pps.weighted_bipred_idc); str += str2;
    str2.Format(_T("pic_init_qp_minus26: %d\r\n"), pps.pic_init_qp_minus26); str += str2;
    str2.Format(_T("pic_init_qs_minus26: %d\r\n"), pps.pic_init_qs_minus26); str += str2;
    str2.Format(_T("chroma_qp_index_offset: %d\r\n"), pps.chroma_qp_index_offset); str += str2;
    str2.Format(_T("deblocking_filter_control_present_flag: %d\r\n"), pps.deblocking_filter_control_present_flag); str += str2;
    str2.Format(_T("constrained_intra_pred_flag: %d\r\n"), pps.constrained_intra_pred_flag); str += str2;
    str2.Format(_T("redundant_pic_cnt_present_flag: %d\r\n"), pps.redundant_pic_cnt_present_flag); str += str2;
    str2.Format(_T("transform_8x8_mode_flag: %d\r\n"), pps.transform_8x8_mode_flag); str += str2;
    str2.Format(_T("pic_scaling_matrix_present_flag: %d\r\n"), pps.pic_scaling_matrix_present_flag); str += str2;

    str2 = _T("pic_scaling_list_present_flag[0..11]: ");
    for (int i = 0; i < 12; ++i) { CString str3 = _T(""); str3.Format(_T("%d"), pps.pic_scaling_list_present_flag[i]); str2 += str3; } str += str2 + _T("\r\n");

    str2.Format(_T("second_chroma_qp_index_offset: %d\r\n"), pps.second_chroma_qp_index_offset); str += str2;
    
    str2.Format(_T("\r\n---------Slice Header Info------------\r\n")); str += str2;
    str2.Format(_T("first_mb_in_slice: %d\r\n"), slice_header.first_mb_in_slice); str += str2;
    str2.Format(_T("slice_type: %d\r\n"), slice_header.slice_type); str += str2;
    str2.Format(_T("pic_parameter_set_id: %d\r\n"), slice_header.pic_parameter_set_id); str += str2;
    str2.Format(_T("colour_plane_id: %d\r\n"), slice_header.colour_plane_id); str += str2;
    str2.Format(_T("frame_num: %d\r\n"), slice_header.frame_num); str += str2;
    str2.Format(_T("field_pic_flag: %d\r\n"), slice_header.field_pic_flag); str += str2;
    str2.Format(_T("bottom_field_flag: %d\r\n"), slice_header.bottom_field_flag); str += str2;
    str2.Format(_T("idr_pic_id: %d\r\n"), slice_header.idr_pic_id); str += str2;
    str2.Format(_T("pic_order_cnt_lsb: %d\r\n"), slice_header.pic_order_cnt_lsb); str += str2;
    str2.Format(_T("delta_pic_order_cnt_bottom: %d\r\n"), slice_header.delta_pic_order_cnt_bottom); str += str2;
    str2.Format(_T("delta_pic_order_cnt[0..1]: {%d, %d}\r\n"), slice_header.delta_pic_order_cnt[0], slice_header.delta_pic_order_cnt[1]); str += str2;
    str2.Format(_T("redundant_pic_cnt: %d\r\n"), slice_header.redundant_pic_cnt); str += str2;
    str2.Format(_T("direct_spatial_mv_pred_flag: %d\r\n"), slice_header.direct_spatial_mv_pred_flag); str += str2;
    str2.Format(_T("num_ref_idx_active_override_flag: %d\r\n"), slice_header.num_ref_idx_active_override_flag); str += str2;
    str2.Format(_T("num_ref_idx_l0_active_minus1: %d\r\n"), slice_header.num_ref_idx_l0_active_minus1); str += str2;
    str2.Format(_T("num_ref_idx_l1_active_minus1: %d\r\n"), slice_header.num_ref_idx_l1_active_minus1); str += str2;
    str2.Format(_T("cabac_init_idc: %d\r\n"), slice_header.cabac_init_idc); str += str2;
    str2.Format(_T("slice_qp_delta: %d\r\n"), slice_header.slice_qp_delta); str += str2;
    str2.Format(_T("sp_for_switch_flag: %d\r\n"), slice_header.sp_for_switch_flag); str += str2;
    str2.Format(_T("slice_qs_delta: %d\r\n"), slice_header.slice_qs_delta); str += str2;
    str2.Format(_T("disable_deblocking_filter_idc: %d\r\n"), slice_header.disable_deblocking_filter_idc); str += str2;
    str2.Format(_T("slice_alpha_c0_offset_div2: %d\r\n"), slice_header.slice_alpha_c0_offset_div2); str += str2;
    str2.Format(_T("slice_beta_offset_div2: %d\r\n"), slice_header.slice_beta_offset_div2); str += str2;
    str2.Format(_T("slice_group_change_cycle: %d\r\n"), slice_header.slice_group_change_cycle); str += str2;
    str2.Format(_T("ref_pic_list_modification_flag_l0: %d\r\n"), slice_header.ref_pic_list_modification_flag_l0); str += str2;

    str2 = _T("modification_of_pic_nums_idc[0][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.modification_of_pic_nums_idc[0][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("modification_of_pic_nums_idc[1][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.modification_of_pic_nums_idc[1][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("abs_diff_pic_num_minus1[0][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.abs_diff_pic_num_minus1[0][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("abs_diff_pic_num_minus1[1][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.abs_diff_pic_num_minus1[1][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("long_term_pic_num[0][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.long_term_pic_num[0][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("long_term_pic_num[1][0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.long_term_pic_num[1][i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2.Format(_T("ref_pic_list_modification_flag_l1: %d\r\n"), slice_header.ref_pic_list_modification_flag_l1); str += str2;
    str2.Format(_T("ref_pic_list_modification_count_l0: %d\r\n"), slice_header.ref_pic_list_modification_count_l0); str += str2;
    str2.Format(_T("ref_pic_list_modification_count_l1: %d\r\n"), slice_header.ref_pic_list_modification_count_l1); str += str2;
    str2.Format(_T("luma_log2_weight_denom: %d\r\n"), slice_header.luma_log2_weight_denom); str += str2;
    str2.Format(_T("chroma_log2_weight_denom: %d\r\n"), slice_header.chroma_log2_weight_denom); str += str2;
    str2.Format(_T("luma_weight_l0_flag: %d\r\n"), slice_header.luma_weight_l0_flag); str += str2;

    str2 = _T("luma_weight_l0[0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.luma_weight_l0[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("luma_offset_l0[0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.luma_offset_l0[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2.Format(_T("chroma_weight_l0_flag: %d\r\n"), slice_header.chroma_weight_l0_flag); str += str2;

    str2 = _T("chroma_weight_l0[0..31][0..1]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d_%d"), slice_header.chroma_weight_l0[i][0], slice_header.chroma_weight_l0[i][1]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("chroma_offset_l0[0..31][0..1]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d_%d"), slice_header.chroma_offset_l0[i][0], slice_header.chroma_offset_l0[i][1]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2.Format(_T("luma_weight_l1_flag: %d\r\n"), slice_header.luma_weight_l1_flag); str += str2;
    
    str2 = _T("luma_weight_l1[0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.luma_weight_l1[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("luma_offset_l1[0..31]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d"), slice_header.luma_offset_l1[i]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2.Format(_T("chroma_weight_l1_flag: %d\r\n"), slice_header.chroma_weight_l1_flag); str += str2;
    
    str2 = _T("chroma_weight_l1[0..31][0..1]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d_%d"), slice_header.chroma_weight_l1[i][0], slice_header.chroma_weight_l1[i][1]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2 = _T("chroma_offset_l1[0..31][0..1]:");
    for (int i = 0; i < 31; ++i) { CString str3 = _T(""); str3.Format(_T(" %d_%d"), slice_header.chroma_offset_l1[i][0], slice_header.chroma_offset_l1[i][1]); str2 += str3; } str += str2 + _T("\r\n");
    
    str2.Format(_T("no_output_of_prior_pics_flag: %d\r\n"), slice_header.no_output_of_prior_pics_flag); str += str2;
    str2.Format(_T("long_term_reference_flag: %d\r\n"), slice_header.long_term_reference_flag); str += str2;
    str2.Format(_T("adaptive_ref_pic_marking_mode_flag: %d\r\n"), slice_header.adaptive_ref_pic_marking_mode_flag); str += str2;
    
    for (int i = 0; i < 31; ++i)
    {
        str2 = _T("");
        str2.Format(_T(" m_dec_ref_pic_marking[%d/31]: memory_management_control_operation(%d) | difference_of_pic_nums_minus1(%d) "
            "| long_term_pic_num_2(%d) | long_term_frame_idx(%d) | max_long_term_frame_idx_plus1(%d)"), 
            i, 
            slice_header.m_dec_ref_pic_marking[i].memory_management_control_operation, 
            slice_header.m_dec_ref_pic_marking[i].difference_of_pic_nums_minus1, 
            slice_header.m_dec_ref_pic_marking[i].long_term_pic_num_2, 
            slice_header.m_dec_ref_pic_marking[i].long_term_frame_idx, 
            slice_header.m_dec_ref_pic_marking[i].max_long_term_frame_idx_plus1
            );

        str += str2 + _T("\r\n");
    }
    
    str2.Format(_T("dec_ref_pic_marking_count: %d\r\n"), slice_header.dec_ref_pic_marking_count); str += str2;
    str2.Format(_T("slice_id: %d\r\n"), slice_header.slice_id); str += str2;
    str2.Format(_T("syntax_element_categories: %d\r\n"), slice_header.syntax_element_categories); str += str2;
    str2.Format(_T("slice_type_fixed: %d\r\n"), slice_header.slice_type_fixed); str += str2;
    str2.Format(_T("mb_cnt: %d\r\n"), slice_header.mb_cnt); str += str2;
    str2.Format(_T("QPY_prev: %d\r\n"), slice_header.QPY_prev); str += str2;
    str2.Format(_T("SliceQPY: %d\r\n"), slice_header.SliceQPY); str += str2;
    str2.Format(_T("MbaffFrameFlag: %d\r\n"), slice_header.MbaffFrameFlag); str += str2;
    str2.Format(_T("PicHeightInMbs: %d\r\n"), slice_header.PicHeightInMbs); str += str2;
    str2.Format(_T("PicHeightInSamplesL: %d\r\n"), slice_header.PicHeightInSamplesL); str += str2;
    str2.Format(_T("PicHeightInSamplesC: %d\r\n"), slice_header.PicHeightInSamplesC); str += str2;
    str2.Format(_T("PicSizeInMbs: %d\r\n"), slice_header.PicSizeInMbs); str += str2;
    str2.Format(_T("MaxPicNum: %d\r\n"), slice_header.MaxPicNum); str += str2;
    str2.Format(_T("CurrPicNum: %d\r\n"), slice_header.CurrPicNum); str += str2;
    str2.Format(_T("SliceGroupChangeRate: %d\r\n"), slice_header.SliceGroupChangeRate); str += str2;
    str2.Format(_T("MapUnitsInSliceGroup0: %d\r\n"), slice_header.MapUnitsInSliceGroup0); str += str2;
    str2.Format(_T("QSY: %d\r\n"), slice_header.QSY); str += str2;
    str2.Format(_T("picNumL0Pred: %d\r\n"), slice_header.picNumL0Pred); str += str2;
    str2.Format(_T("picNumL1Pred: %d\r\n"), slice_header.picNumL1Pred); str += str2;
    str2.Format(_T("refIdxL0: %d\r\n"), slice_header.refIdxL0); str += str2;
    str2.Format(_T("refIdxL1: %d\r\n"), slice_header.refIdxL1); str += str2;
    str2.Format(_T("PrevRefFrameNum: %d\r\n"), slice_header.PrevRefFrameNum); str += str2;
    str2.Format(_T("UnusedShortTermFrameNum: %d\r\n"), slice_header.UnusedShortTermFrameNum); str += str2;
    str2.Format(_T("FilterOffsetA: %d\r\n"), slice_header.FilterOffsetA); str += str2;
    str2.Format(_T("FilterOffsetB: %d\r\n"), slice_header.FilterOffsetB); str += str2;
    
    str2.Format(_T("\r\n---------Picture Info----------------\r\n")); str += str2;
    str2.Format(_T("TopFieldOrderCnt: %d\r\n"), pic.TopFieldOrderCnt); str += str2;
    str2.Format(_T("BottomFieldOrderCnt: %d\r\n"), pic.BottomFieldOrderCnt); str += str2;
    str2.Format(_T("PicOrderCntMsb: %d\r\n"), pic.PicOrderCntMsb); str += str2;
    str2.Format(_T("PicOrderCntLsb: %d\r\n"), pic.PicOrderCntLsb); str += str2;
    str2.Format(_T("FrameNumOffset: %d\r\n"), pic.FrameNumOffset); str += str2;
    str2.Format(_T("absFrameNum: %d\r\n"), pic.absFrameNum); str += str2;
    str2.Format(_T("picOrderCntCycleCnt: %d\r\n"), pic.picOrderCntCycleCnt); str += str2;
    str2.Format(_T("frameNumInPicOrderCntCycle: %d\r\n"), pic.frameNumInPicOrderCntCycle); str += str2;
    str2.Format(_T("expectedPicOrderCnt: %d\r\n"), pic.expectedPicOrderCnt); str += str2;
    str2.Format(_T("PicOrderCnt: %d\r\n"), pic.PicOrderCnt); str += str2;
    str2.Format(_T("FrameNum: %d\r\n"), pic.FrameNum); str += str2;
    str2.Format(_T("FrameNumWrap: %d\r\n"), pic.FrameNumWrap); str += str2;
    str2.Format(_T("LongTermFrameIdx: %d\r\n"), pic.LongTermFrameIdx); str += str2;
    str2.Format(_T("PicNum: %d\r\n"), pic.PicNum); str += str2;
    str2.Format(_T("LongTermPicNum: %d\r\n"), pic.LongTermPicNum); str += str2;
    str2.Format(_T("FieldNum: %d\r\n"), pic.FieldNum); str += str2;
    str2.Format(_T("MaxLongTermFrameIdx: %d\r\n"), pic.MaxLongTermFrameIdx); str += str2;
    str2.Format(_T("memory_management_control_operation_5_flag: %d\r\n"), pic.memory_management_control_operation_5_flag); str += str2;
    str2.Format(_T("memory_management_control_operation_6_flag: %d\r\n"), pic.memory_management_control_operation_6_flag); str += str2;
    str2.Format(_T("reference_marked_type: %d\r\n"), pic.reference_marked_type); str += str2;
    str2.Format(_T("m_RefPicList0Length: %d\r\n"), pic.m_RefPicList0Length); str += str2;
    str2.Format(_T("m_RefPicList1Length: %d\r\n"), pic.m_RefPicList1Length); str += str2;
    
    for (int i = 0; i < pic.m_RefPicList0Length; ++i)
    {
        str2.Format(_T("m_PicNumCnt=%d(%s); PicOrderCnt=%d; m_RefPicList0[%d]: %s; PicOrderCnt=%d; PicNum=%d; PicNumCnt=%d;\r\n"), 
            pic.m_PicNumCnt, H264_SLIECE_TYPE_TO_STR(slice_header.slice_type), pic.PicOrderCnt, i,
            (pic.m_RefPicList0[i]) ? H264_SLIECE_TYPE_TO_STR(pic.m_RefPicList0[i]->m_picture_frame.m_h264_slice_header.slice_type) : "UNKNOWN",
            (pic.m_RefPicList0[i]) ? pic.m_RefPicList0[i]->m_picture_frame.PicOrderCnt : -1,
            (pic.m_RefPicList0[i]) ? pic.m_RefPicList0[i]->m_picture_frame.PicNum : -1,
            (pic.m_RefPicList0[i]) ? pic.m_RefPicList0[i]->m_picture_frame.m_PicNumCnt : -1
            );
        str += str2;
    }

    for (int i = 0; i < pic.m_RefPicList1Length; ++i)
    {
        str2.Format(_T("m_PicNumCnt=%d(%s); PicOrderCnt=%d; m_RefPicList1[%d]: %s; PicOrderCnt=%d; PicNum=%d; PicNumCnt=%d;\r\n"), 
            pic.m_PicNumCnt, H264_SLIECE_TYPE_TO_STR(slice_header.slice_type), pic.PicOrderCnt, i,
            (pic.m_RefPicList1[i]) ? H264_SLIECE_TYPE_TO_STR(pic.m_RefPicList1[i]->m_picture_frame.m_h264_slice_header.slice_type) : "UNKNOWN",
            (pic.m_RefPicList1[i]) ? pic.m_RefPicList1[i]->m_picture_frame.PicOrderCnt : -1,
            (pic.m_RefPicList1[i]) ? pic.m_RefPicList1[i]->m_picture_frame.PicNum : -1,
            (pic.m_RefPicList1[i]) ? pic.m_RefPicList1[i]->m_picture_frame.m_PicNumCnt : -1
            );
        str += str2;
    }

    str2.Format(_T("m_PicNumCnt: %d\r\n"), pic.m_PicNumCnt); str += str2;
    str2.Format(_T("m_slice_cnt: %d\r\n"), pic.m_slice_cnt); str += str2;
    
    str2.Format(_T("\r\n---------H264 Macro Block Info----------------\r\n")); str += str2;
    
    int frame_mb_cnt[5] = {0}; //帧宏块数目统计
    int field_mb_I_cnt[5] = {0}; //场宏块数目统计

    for (int i = 0; i < pic.PicSizeInMbs; ++i)
    {
        if (pic.m_mbs[i].mb_field_decoding_flag == 0) //帧宏块数目统计
        {
            frame_mb_cnt[pic.m_mbs[i].m_slice_type_fixed % 5] += 1;
        }
        else //场宏块数目统计
        {
            field_mb_I_cnt[pic.m_mbs[i].m_slice_type_fixed % 5] += 1;
        }
    }
    
    str2.Format(_T("frame_mb_cnt: I(%d)  P(%d)  B(%d)  SI(%d)  SP(%d)\r\n"), 
        frame_mb_cnt[H264_SLIECE_TYPE_I], frame_mb_cnt[H264_SLIECE_TYPE_P], frame_mb_cnt[H264_SLIECE_TYPE_B], 
        frame_mb_cnt[H264_SLIECE_TYPE_SI], frame_mb_cnt[H264_SLIECE_TYPE_SP]); str += str2;
    
    str2.Format(_T("field_mb_I_cnt: I(%d)  P(%d)  B(%d)  SI(%d)  SP(%d)\r\n"), 
        field_mb_I_cnt[H264_SLIECE_TYPE_I], field_mb_I_cnt[H264_SLIECE_TYPE_P], field_mb_I_cnt[H264_SLIECE_TYPE_B], 
        field_mb_I_cnt[H264_SLIECE_TYPE_SI], field_mb_I_cnt[H264_SLIECE_TYPE_SP]); str += str2;

    return 0;
}
