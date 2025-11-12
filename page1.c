#include <windows.h>
#include <stdio.h>

#include "get_device.h"
#include "page1.h"

// 自定义
#define WINDOW_NAME "C-Paint"

HWND hComb; // 下拉框的实例
HWND hRefreshButton;
HWND hButton1;
HWND hButton2;

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

    SendMessage(hComb, CB_SETCURSEL, (WPARAM)-1, 0); // 设置默认项为空白

    hRefreshButton = CreateWindow(
        "BUTTON",
        "Refresh List",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        50, 80, 120, 50,
        hwnd,
        (HMENU)ID_REFRESH_BUTTON,
        hInstance,
        NULL);

    hButton1 = CreateWindow(
        "BUTTON",
        "Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        50, 180, 100, 50,
        hwnd,
        (HMENU)ID_BUTTON1,
        hInstance,
        NULL);

    hButton2 = CreateWindow(
        "BUTTON",
        "Quit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        200, 180, 100, 50,
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
                // //g_count = get_devices_count();
                // g_showText = 1;
                // //output = get_device_name(0);
                // InvalidateRect(hwnd, NULL, FALSE); // 触发重绘
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
}