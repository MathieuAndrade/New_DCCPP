//
// Created by mathieu on 22/06/2021.
//
#include "DCCpp.h"
#include "DCCpp_utils.h"
#include "DCCpp_dlg.h"
#include "DCCpp_dlg_definitions.h"

bool DCCpp_dlg::runParamDlg()
{
    return DialogBoxParam(DCCpp::instance, MAKEINTRESOURCE(DCCPP_PARAM_DLG), DCCpp::wnd, (DLGPROC)DCCpp_dlg::ParamDlgProc, (LPARAM) nullptr) > 0;
}

bool CALLBACK DCCpp_dlg::ParamDlgProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    bool success = false;
    std::list<std::string> portList = DCCpp_utils::listPorts();
    std::list<std::string>::iterator it;

    // Add auto mode on top level
    //  portList.push_front("AUTO");

    switch (message)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_USB_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_S88_EDITTEXT), WM_SETTEXT, 0, (LPARAM) "20");

        // Init combo box list
        for (it = portList.begin(); it != portList.end(); ++it)
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_ADDSTRING, 0, (LPARAM)it->c_str());
        }
        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_SETCURSEL, 0, 0);

        break;

    case WM_COMMAND:

        switch (LOWORD(wparam))
        {
        case DCCPP_PARAM_DLG_CANCEL_BUTTON:
            EndDialog(wnd, 0);
            break;
        case DCCPP_PARAM_DLG_CONNECTION_BUTTON:
            DCCpp::detectorsModuleCount = (int)GetDlgItemInt(wnd, DCCPP_PARAM_DLG_S88_EDITTEXT, nullptr, false);
            GetDlgItemText(wnd, DCCPP_PARAM_DLG_COM_LIST, &DCCpp::comIp, 9);
            success = true;
            EndDialog(wnd, 1);
            break;
        case DCCPP_PARAM_DLG_SCAN_BUTTON:
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_RESETCONTENT, 0, 0);
            portList = DCCpp_utils::listPorts();
            // Add auto mode on top level
            //  portList.push_front("AUTO");
            for (it = portList.begin(); it != portList.end(); ++it)
            {
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_ADDSTRING, 0, (LPARAM)it->c_str());
            }
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_SETCURSEL, 0, 0);
            break;
        default:
            break;
        }
        break;

    case WM_CLOSE:
        EndDialog(wnd, 0);
        break;
    default:
        return false;
    }

    return success;
}