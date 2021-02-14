
// SDH264Player.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h" // main symbols


// CSDH264PlayerApp:
// See SDH264Player.cpp for the implementation of this class
//

class CSDH264PlayerApp : public CWinApp
{
public:
    CSDH264PlayerApp();

    // Overrides
public:
    virtual BOOL InitInstance();

    // Implementation

    DECLARE_MESSAGE_MAP()
};

extern CSDH264PlayerApp theApp;