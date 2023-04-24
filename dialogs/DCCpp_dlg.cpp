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
    std::stringstream temp;

    // Convert int numbers to string
    std::string detectors = std::to_string(DCCpp::detectorsModuleCount);
    std::string timer = std::to_string(DCCpp::cmdTimer);

    switch (message)
    {
    case WM_INITDIALOG:
        if (DCCpp::usbMode)
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_USB_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        }
        else
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_WIFI_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        }

        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), WM_SETTEXT, 0, (LPARAM)DCCpp::ipAddress);
        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_S88_EDITTEXT), WM_SETTEXT, 0, (LPARAM)detectors.c_str());
        SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_TIMER_EDITTEXT), WM_SETTEXT, 0, (LPARAM)timer.c_str());

        // Init combo box list
        for (it = portList.begin(); it != portList.end(); ++it)
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_ADDSTRING, 0, (LPARAM)it->c_str());

            if (strstr(DCCpp::comNumber, it->c_str()))
            {
                int index = std::distance(portList.begin(), it);
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_SETCURSEL, index, 0);
            }
            else
            {
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_SETCURSEL, 0, 0);
            }
        }

        if((std::strcmp(DCCpp::accessoryCmdType, "a") == 0))
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_A_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        }
        else if ((std::strcmp(DCCpp::accessoryCmdType, "X") == 0))
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_X_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        }
        else
        {
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_T_RADIO), BM_SETCHECK, BST_CHECKED, 1);
        }

        DCCpp_dlg::switchDlgParamMode(wnd);
        break;

    case WM_COMMAND:

        switch (LOWORD(wparam))
        {
        case DCCPP_PARAM_DLG_CANCEL_BUTTON:
            EndDialog(wnd, 0);
            break;
        case DCCPP_PARAM_DLG_CONNECTION_BUTTON:
            DCCpp::detectorsModuleCount = (int)GetDlgItemInt(wnd, DCCPP_PARAM_DLG_S88_EDITTEXT, nullptr, false);
            DCCpp::cmdTimer = (int)GetDlgItemInt(wnd, DCCPP_PARAM_DLG_TIMER_EDITTEXT, nullptr, false);

            // https://github.com/MathieuAndrade/New_DCCPP/blob/main/doc/Timer.md
            if(DCCpp::cmdTimer > 200) {
                DCCpp::cmdTimer = 200;
            }

            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), WM_GETTEXT, (WPARAM)20, (LPARAM)DCCpp::comNumber);
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), WM_GETTEXT, (WPARAM)20, (LPARAM)DCCpp::ipAddress);

            if (DCCpp::usbMode)
            {
                if (strlen(DCCpp::comNumber) != 0)
                {
                    success = true;
                    EndDialog(wnd, 1);
                }
                else
                {
                    DCCpp_dlg::showInvalidParamMsg();
                }
            }
            else
            {
                if (strlen(DCCpp::ipAddress) != 0)
                {
                    success = true;
                    EndDialog(wnd, 1);
                }
                else
                {
                    DCCpp_dlg::showInvalidParamMsg();
                }
            }

            break;
        case DCCPP_PARAM_DLG_SCAN_BUTTON:
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_RESETCONTENT, 0, 0);

            portList = DCCpp_utils::listPorts();
            for (it = portList.begin(); it != portList.end(); ++it)
            {
                SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_ADDSTRING, 0, (LPARAM)it->c_str());
            }
            SendMessage(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), CB_SETCURSEL, 0, 0);
            break;
        case DCCPP_PARAM_DLG_USB_RADIO:
            DCCpp::usbMode = true;
            DCCpp_dlg::switchDlgParamMode(wnd);
            break;
        case DCCPP_PARAM_DLG_WIFI_RADIO:
            DCCpp::usbMode = false;
            DCCpp_dlg::switchDlgParamMode(wnd);
            break;
        case DCCPP_PARAM_DLG_A_RADIO:
            // Simple accessory mode
            DCCpp::accessoryCmdType[0] = 'a';
            break;
        case DCCPP_PARAM_DLG_X_RADIO:
            // Extended accessory mode
            DCCpp::accessoryCmdType[0] = 'X';
            break;
        case DCCPP_PARAM_DLG_T_RADIO:
            // Turnout mode
            DCCpp::accessoryCmdType[0] = 'T';
            break;
        default:
            break;
        }
        break;
    case WM_NOTIFY:
        switch (((LPNMHDR)lparam)->code)
        {
        case (UINT)NM_CLICK:
        case (UINT)NM_RETURN:
            ShellExecute(nullptr,  "open", TIMER_HELP_LINK, nullptr, nullptr, SW_SHOW);
            break;
        default:
            break;
        }
        break ;

    case WM_CLOSE:
        EndDialog(wnd, 0);
        break;
    default:
        return false;
    }

    return success;
}

void DCCpp_dlg::showInvalidParamMsg()
{
    char msg[] = "Les parametres que vous avez indiques semblent errones\nVeuillez recommencer";
    MessageBox(DCCpp::wnd, msg, " Parametres invalides", MB_APPLMODAL | MB_OK | MB_ICONSTOP);
}

void DCCpp_dlg::switchDlgParamMode(HWND wnd)
{
    if (DCCpp::usbMode)
    {
        // Switch controls
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), true);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), false);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LABEL), true);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_LABEL), false);
        // Scan work only on USB mode
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_SCAN_BUTTON), true);
    }
    else
    {
        // Switch controls
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_EDITTEXT), true);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LIST), false);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_IP_LABEL), true);
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_COM_LABEL), false);
        // Scan work only on USB mode
        EnableWindow(GetDlgItem(wnd, DCCPP_PARAM_DLG_SCAN_BUTTON), false);
    }
}