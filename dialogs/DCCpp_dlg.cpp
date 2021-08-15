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

            if (DCCpp::usbMode)
            {
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), WM_GETTEXT, (WPARAM)20, (LPARAM)DCCpp::comIp);
            }
            else
            {
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), WM_GETTEXT, (WPARAM)20, (LPARAM)DCCpp::comIp);
            }

            if (strlen(DCCpp::comIp) != 0)
            {
                success = true;
                EndDialog(wnd, 1);
            } // TODO: Show an alert message

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
        case DCCPP_PARAM_DLG_USB_RADIO:
            // Switch controls
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), true);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), false);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LABEL), true);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_LABEL), false);
            // Scan work only on USB mode
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_SCAN_BUTTON), true);

            DCCpp::usbMode = true;
            break;
        case DCCPP_PARAM_DLG_WIFI_RADIO:
            // Switch controls
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), true);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), false);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_LABEL), true);
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LABEL), false);
            // Scan work only on USB mode
            EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_SCAN_BUTTON), false);

            DCCpp::usbMode = false;
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