//
// Created by mathieu on 22/06/2021.
//

#ifndef DDGI_DCCPP_DCCPP_DLG_H
#define DDGI_DCCPP_DCCPP_DLG_H

#define _WIN32_IE 0x0600
#include <commctrl.h>

class DCCpp_dlg
{
  public:
    static HWND progressWnd;

    static bool runParamDlg();
    static bool CALLBACK ParamDlgProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
    static void showInvalidParamMsg();
    static void switchDlgParamMode(HWND wnd);

    static bool runProgressDlg();
    static bool CALLBACK ProgressDlgProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
    static void setTimerOfProgressDlg(int seconds);
    static void closeProgressDlg();
};

#endif // DDGI_DCCPP_DCCPP_DLG_H
