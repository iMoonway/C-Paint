#include <windows.h>
#include <stdio.h>

#include "get_device.h"
#include "page1.h"
#include "page2.h"
#include "main.h"

// 自定义
#define WINDOW_NAME "C-Paint"

HWND hComb; // 下拉框的实例
HWND hBaudComb; // 波特率下拉框实例
HWND hRefreshButton;
HWND hButton1;
HWND hButton2;

const char* baudDefaultList[] = {
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200"
};

void Page1Init(HWND hwnd, HINSTANCE hInstance)
{
    // 下拉框

    hComb = CreateWindow(
        "COMBOBOX",
        "BlueTooth",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        50, 20, 400, 100,
        hwnd,
        (HMENU)ID_COMBOBOX,
        hInstance,
        NULL);

    hBaudComb = CreateWindow(
        "COMBOBOX",
        "BaudRateSetting",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
        50, 150, 200, 100,
        hwnd,
        (HMENU)ID_BAUD_COMBOBOX,
        hInstance,
        NULL);

    init_device_list(); // 初始化获取设备所需的变量

    int count = get_devices_count(); // 获取可以获取到的所有端口数目
    int i = 0;

    // 对下拉框的内容进行填充
    for (i = 0; i < count; i++)
    {
        char *friend_name = get_device_name(i);

        if (friend_name[0] != '\0')
        {
            SendMessage(hComb, CB_ADDSTRING, 0, (LPARAM)friend_name);
        }
    }

    for (int i = 0;i < 8;i++){
        SendMessage(hBaudComb, CB_ADDSTRING, 0, (LPARAM)baudDefaultList[i]);
    }

    SendMessage(hBaudComb,CB_SETCURSEL,(WPARAM)-1, 0);
    SendMessage(hComb, CB_SETCURSEL, (WPARAM)-1, 0);

    hRefreshButton = CreateWindow(
        "BUTTON",
        "Refresh List",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        590, 20, 120, 50,
        hwnd,
        (HMENU)ID_REFRESH_BUTTON,
        hInstance,
        NULL);

    hButton1 = CreateWindow(
        "BUTTON",
        "Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        150, 475, 100, 50,
        hwnd,
        (HMENU)ID_BUTTON1,
        hInstance,
        NULL);

    hButton2 = CreateWindow(
        "BUTTON",
        "Quit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        550, 475, 100, 50,
        hwnd,
        (HMENU)ID_BUTTON2,
        hInstance,
        NULL);
}

static int g_count = 0;    // 测试变量
static int g_showText = 0; // 测试变量

void Page1Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return;

    case WM_COMMAND:
        switch
            LOWORD(wParam)
            {
            case ID_BUTTON2:
                PostQuitMessage(0);
                return;
            case ID_REFRESH_BUTTON:
                refresh_device_list();
                SendMessage(hComb, CB_RESETCONTENT, 0, 0);

                int count = get_devices_count();
                int i = 0;

                for (i = 0; i < count; i++)
                {
                    char *friend_name = get_device_name(i);

                    if (friend_name[0] != '\0')
                    {
                        SendMessage(hComb, CB_ADDSTRING, 0, (LPARAM)friend_name);
                    }
                }
                SendMessage(hComb, CB_SETCURSEL, (WPARAM)-1, 0);
                return;

            // 按钮处理
            case ID_BUTTON1:
                int comboSelcet = SendMessage(hComb, CB_GETCURSEL, 0, 0);
                int deviceCount = get_devices_count();

                if(comboSelcet < 0|| comboSelcet >= deviceCount){
                    MessageBoxW(NULL,
                                L"未选择或者选择有误,请点击Refresh_List后再重新选择",
                                L"错误",
                                MB_OK | MB_ICONERROR);
                    return;
                }

                int comNumber = get_device_com_number(comboSelcet);
                
                char baudStr[52] = {0};
                GetWindowText(hBaudComb, baudStr, sizeof(baudStr)/sizeof(baudStr[0]));
                int baudNumber = -1;
                sscanf(baudStr,"%6d",&baudNumber);

                init_com(comNumber,baudNumber);
                set_page_index(1);

                return;
            }

    case WM_PAINT:
        // PAINTSTRUCT ps;
        // HDC hdc = BeginPaint(hwnd, &ps);

        // char buffer[52];
        // strcpy(buffer,output);
        // if(g_showText){
        //     //sprintf(buffer, "Value: %d", g_count);
        //     TextOutA(hdc, 50, 200, buffer, strlen(buffer));
        // }
        // //TextOut(hdc, 10, 10, "Hello, World! (WinAPI)", 25);

        // EndPaint(hwnd, &ps);
        return;
    case WM_SIZE:
        int parentX = LOWORD(lParam);
        int parentY = HIWORD(lParam);

        int leftUPX = parentX * 5/8;
        int leftUPY = parentY * 2/3;

        MoveWindow(hComb,
            leftUPX / 10, 20,
            leftUPX * 4 / 5, 100,
            TRUE);
        MoveWindow(hBaudComb,
            leftUPX / 10, 150,
            leftUPX * 4 / 5, 100,
            TRUE);
        MoveWindow(hRefreshButton,
            leftUPX + (parentX - leftUPX) *3/10, 20,
            (parentX - leftUPX) * 2 / 5, leftUPY / 8,
            TRUE);
        MoveWindow(hButton1, 
            (parentX - 200) / 4, (parentY - leftUPY - 50) / 2 + leftUPY, 
            100, 50, 
            TRUE);
        MoveWindow(hButton2, 
            parentX / 2 + (parentX - 200) / 4, (parentY - leftUPY - 50) / 2 + leftUPY, 
            100, 50, 
            TRUE);
    }
}

void Page1Show(int showAble){
    int command = -1;

    if (showAble == 1){
        command = SW_SHOW;
    }
    else command = SW_HIDE;

    ShowWindow(hComb,command);
    ShowWindow(hRefreshButton,command);
    ShowWindow(hButton1,command);
    ShowWindow(hButton2,command);
    ShowWindow(hBaudComb,command);
}