#pragma once
#include "afxwin.h"


// CFrameInfoDlg dialog

class CFrameInfoDlg : public CDialogEx
{
    DECLARE_DYNAMIC(CFrameInfoDlg)

public:
    CFrameInfoDlg(CWnd* pParent = NULL);   // standard constructor
    CFrameInfoDlg(CString str, CWnd* pParent = NULL);
    virtual ~CFrameInfoDlg();

    // Dialog Data
    enum { IDD = IDD_DIALOG_FRAME_INFO };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CEdit m_edit_frame_info;
    CString m_str_frame_info;
    
public:
    int SetFrameInfo(CString str_frame_info);

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButton1();
};
