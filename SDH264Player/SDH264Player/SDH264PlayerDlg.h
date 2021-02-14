// SDH264PlayerDlg.h : header file
//

#pragma once
#include "MyStatic.h"


// CSDH264PlayerDlg dialog
class CSDH264PlayerDlg : public CDialogEx
{
    // Construction
public:
    CSDH264PlayerDlg(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    enum { IDD = IDD_SDH264PLAYER_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


    // Implementation
protected:
    HICON m_hIcon;

public:
    CMyStatic m_static_player;
    bool m_isPause; // «∑Ò‘›Õ£

    CString m_str_url;

public:
    int InitCtrl();
    int CalcWindowPos();
    int GetFrameInfo(CString &str);

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    afx_msg void OnBnClickedButton4();
    afx_msg void OnBnClickedButton5();
    afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
    afx_msg LRESULT OnDecoderThreadEnd(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLookPictureOpen(WPARAM wParam, LPARAM lParam);
};
