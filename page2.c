#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

#include "page2.h"

POINT g_points[MAX_POINTS];
int g_pointCount = 0;

HWND hCanvas;

// 当前是否处于“绘图页面”
int g_isDrawingPage = 0;
int g_startDraw = 0;
UINT_PTR g_timer;

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void Page2Init(HWND hwnd, HINSTANCE hInstance)
{
    // 在 WinMain 开头添加：
    WNDCLASS wcCanvas = {0};
    wcCanvas.lpfnWndProc = CanvasProc; // 自定义窗口过程
    wcCanvas.hInstance = hInstance;
    wcCanvas.lpszClassName = "DrawingCanvas";
    wcCanvas.hbrBackground = CreateSolidBrush(RGB(112,128,144));
    wcCanvas.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&wcCanvas);

    hCanvas = CreateWindow(
        "DrawingCanvas",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        50, 50,          
        400, 400,         
        hwnd,             
        (HMENU)ID_CANVAS, 
        hInstance,
        NULL 
    );
}

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
        if (g_isDrawingPage && g_pointCount < MAX_POINTS && hwnd == hCanvas)
        {
            
            // 获取客户区坐标
            POINT pt;
            RECT rc;

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            GetClientRect(hwnd,&rc);

            if(PtInRect(&rc,pt)){

                SetCapture(hwnd);

                g_startDraw = 1;
                g_points[g_pointCount] = pt;
                g_pointCount++;

                // 触发重绘
                InvalidateRect(hwnd, NULL, FALSE);

                g_timer = SetTimer(hwnd,0,25,NULL);
            }
        }
        break;

    case WM_TIMER:
    {
        if (!g_startDraw || g_pointCount >= MAX_POINTS)
            return 0;

        // 获取当前鼠标全局坐标
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(hwnd, &pt);

        g_points[g_pointCount] = pt;
        g_pointCount++;

        InvalidateRect(hwnd, NULL, FALSE);

        break;
    }
    case WM_LBUTTONUP:
    {
        if (g_startDraw)
        {
            g_startDraw = 0;
            ReleaseCapture(); 
            KillTimer(hwnd,g_timer);
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 如果在绘图页面，绘制所有红点
        if (g_isDrawingPage && hwnd == hCanvas)
        {
            HBRUSH hRedBrush = CreateSolidBrush(RGB(255, 0, 0));
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hRedBrush);

            for (int i = 0; i < g_pointCount; ++i)
            {
                // 画一个直径 10 的实心圆
                Ellipse(hdc,
                        g_points[i].x - 5, g_points[i].y - 5,
                        g_points[i].x + 5, g_points[i].y + 5);
            }

            SelectObject(hdc, hOldBrush);
            DeleteObject(hRedBrush);
        }

        EndPaint(hwnd, &ps);
        break;
    }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Page2Show(int showAble){
    int command = -1;

    if (showAble == 1)
    {
        command = SW_SHOW;
    }
    else
        command = SW_HIDE;

    ShowWindow(hCanvas, command);

    g_isDrawingPage = showAble;
}

void Page2Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return;
    }
}