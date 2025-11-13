#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>

#include "page2.h"

POINT g_points[MAX_POINTS];
int g_pointCount = 0;

HWND hCanvas = NULL;

int g_isDrawingPage = 0;// 当前是否处于“绘图页面”
int g_startDraw = 0;
UINT_PTR g_timer;

char filePath[MAX_PATH];
HANDLE hFile = NULL;
char fileFullName[256] = {0}; 

int canvasLen = 400;
int magniNum;

LRESULT CALLBACK CanvasProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


void GetCurrentTimeSTR(char* input)
{
    SYSTEMTIME st;

    GetLocalTime(&st);

    char output[64];

    sprintf(output, "%2d-%2d-%2d-%2d-%2d",
            (int)st.wYear % 100, st.wMonth, st.wDay, st.wHour, st.wMinute);

    strcpy(input,output);

    return;
}


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
        canvasLen, canvasLen,
        hwnd,
        (HMENU)ID_CANVAS,
        hInstance,
        NULL);
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

                char pointInfo[100] = {0};

                char point16[30] = {0};
                sprintf(point16, "%02x%02x%02x", 1, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen);

                sprintf(pointInfo, "%02x%02x%02x -> (%d, %d)\n",
                        1, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen);

                WriteFile(hFile,pointInfo,strlen(pointInfo),NULL,NULL);
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

        SetFilePointer(hFile, 0, NULL, FILE_END);

        char pointInfo[100] = {0};

        char point16[30] = {0};
        sprintf(point16, "%02x%02x%02x", 2, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen);

        sprintf(pointInfo, "%02x%02x%02x -> (%d, %d)\n",2, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen, pt.x * 200 / canvasLen, pt.y * 200 / canvasLen);

        WriteFile(hFile, pointInfo, strlen(pointInfo), NULL, NULL);

        break;
    }
    case WM_LBUTTONUP:
    {
        if (g_startDraw)
        {
            g_startDraw = 0;
            ReleaseCapture(); 
            KillTimer(hwnd,g_timer);

            char pointInfo[100] = {0};

            char point16[30] = {0};
            sprintf(point16, "%02x%02x%02x", 3,0,0);

            sprintf(pointInfo, "%02x%02x%02x -> (%d, %d)\n",3,0,0,0,0);

            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, pointInfo, strlen(pointInfo), NULL, NULL);
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

    if (showAble){
        if (GetModuleFileNameA(NULL, filePath, MAX_PATH) == 0){
            MessageBox(NULL,"未能找到路径，请检查该路径是否合法或者能够读取","Error",MB_OK|MB_ICONERROR);

            return;
        }

        PathRemoveFileSpec(filePath);

        char fileName[100];
        GetCurrentTimeSTR(fileName);

        strcpy(fileFullName,"Data\\");

        strcat(fileFullName,fileName);
        strcat(fileFullName,".txt");

        strcat(filePath,"\\Data");

        CreateDirectory(filePath,NULL);

        hFile = CreateFile(
            fileFullName,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        //MessageBox(NULL, filePath, "Test", MB_OK | MB_ICONINFORMATION);
    }
}


void Page2Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        CloseHandle(hFile);
        PostQuitMessage(0);
        return;
    }
}