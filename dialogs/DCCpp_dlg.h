//
// Created by mathieu on 22/06/2021.
//

#ifndef DDGI_DCCPP_DCCPP_DLG_H
#define DDGI_DCCPP_DCCPP_DLG_H

class DCCpp_dlg
{
  public:
    static bool runParamDlg();
    static bool CALLBACK ParamDlgProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam);
    static void showInvalidParamMsg();
    static void switchDlgParamMode(HWND wnd);
};

#endif // DDGI_DCCPP_DCCPP_DLG_H
