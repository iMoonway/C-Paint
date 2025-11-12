#include <windows.h>
#include <stdio.h>
#include <WinUser.h>

//自定义应用
#include "page1.h"
#include "page2.h"

//自定义
#define WINDOW_NAME "C-Paint"

int page_index = 1;


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); //自定义中断函数，仅对主窗口响应


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const char CLASS_NAME[] = "CPaintClass";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);// 注册窗口

    //父窗口
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        WINDOW_NAME, // 窗口标题
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "CreateWindow failed!", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    Page1Init(hwnd,hInstance);
    Page2Init(hwnd,hInstance);

    switch(page_index){
        case 0:
            Page1Show(1);
            Page2Show(0);
            break;
        case 1:
            Page1Show(0);
            Page2Show(1);
            break;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (page_index)
    {
    case 0:
        Page1Proc(hwnd,uMsg,wParam,lParam);
        break;
    case 1:
        Page2Proc(hwnd,uMsg,wParam,lParam);
        break;
    default:
        break;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}