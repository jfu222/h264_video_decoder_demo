// FrameInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SDH264Player.h"
#include "FrameInfoDlg.h"
#include "afxdialogex.h"


// CFrameInfoDlg dialog

IMPLEMENT_DYNAMIC(CFrameInfoDlg, CDialogEx)

CFrameInfoDlg::CFrameInfoDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CFrameInfoDlg::IDD, pParent)
{

}

CFrameInfoDlg::CFrameInfoDlg(CString str, CWnd* pParent /*=NULL*/)
    : CDialogEx(CFrameInfoDlg::IDD, pParent)
{
    m_str_frame_info = str;
}

CFrameInfoDlg::~CFrameInfoDlg()
{
}

void CFrameInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_edit_frame_info);
}


BEGIN_MESSAGE_MAP(CFrameInfoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON1, &CFrameInfoDlg::OnClickedButton1)
END_MESSAGE_MAP()


// CFrameInfoDlg message handlers


BOOL CFrameInfoDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  Add extra initialization here

    m_edit_frame_info.SetSel(-1, 0, false);

    SetFrameInfo(m_str_frame_info);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


int CFrameInfoDlg::SetFrameInfo(CString str_frame_info)
{
    m_edit_frame_info.SetWindowText(str_frame_info);
    m_edit_frame_info.SetSel(-1, 0, false);

    return 0;
}


void CFrameInfoDlg::OnClickedButton1()
{
    // TODO: Add your control notification handler code here
    this->OnOK();
}
