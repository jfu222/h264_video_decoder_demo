#pragma once

#include <afxwin.h>
#include <vector>
#include "H264VideoDecoder.h" //H264ÊÓÆµ½âÂëÆ÷

#define WM_MY_LBUTTONDOWN (WM_USER+101)

using namespace std;

// CMyMenu

class CMyMenu : public CWnd
{
    DECLARE_DYNAMIC(CMyMenu)

public:
    CMyMenu();
    virtual ~CMyMenu();

protected:
    DECLARE_MESSAGE_MAP()

public:
    CWnd* m_Wnd_Owner;

    CRect m_rcWindow;
    BOOL m_is_show;

    HFONT m_hFont;

    CH264MacroBlock m_mb;
    BITMAP m_bitmap;

public:
    void MyTrackPopupMenu(CWnd *pWnd, CPoint pt, CH264MacroBlock &mb, BITMAP &bitmap);
    int hdc_paint(HDC hdc, int dst_x, int dst_y, int dst_width, int dst_height, BITMAP &bitmap);

public:
    afx_msg void OnPaint();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnMyLbuttondown(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};


