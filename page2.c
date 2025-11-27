#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <minwindef.h>
#include <math.h>

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
HANDLE hSerial = NULL;
int serialInit = 0;

int canvasLen = 500;
int magniNum;

int comNumber = -1;
int baudRate = -1;

struct ParentWindow{
    int windowSizeX;
    int windowSizeY;
};

struct ParentWindow pw;

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
        500, 500,
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
                sprintf(point16, "%02x%03x%03x", 
                    1, 
                    (pt.x * 2000 + canvasLen / 2) / canvasLen, 
                    (pt.y * 2000 + canvasLen / 2) / canvasLen
                );

                sprintf(pointInfo, "%02x%03x%03x -> (%0.1f, %0.1f)\n",
                    1,
                    (pt.x * 2000 + canvasLen / 2) / canvasLen,
                    (pt.y * 2000 + canvasLen / 2) / canvasLen,
                    (float)(pt.x * 200) / canvasLen,
                    (float)(pt.y * 200) / canvasLen
                );

                WriteFile(hFile,pointInfo,strlen(pointInfo),NULL,NULL);

                if (serialInit)
                {
                    char sendData[4];
                    DWORD dataSendNumber = 0;
                    OVERLAPPED overlapped = {0};

                    for (int i = 0; i <= 8; i += 2)
                    {
                        char byte[3] = {pointInfo[i], pointInfo[i + 1], '\0'};
                        sendData[i / 2] = (char)strtol(byte, NULL, 16);
                    }

                    overlapped.Offset = 0;
                    overlapped.OffsetHigh = 0;

                    int error = WriteFile(hSerial, sendData, 4, &dataSendNumber, &overlapped);

                    if (!error && GetLastError() != ERROR_IO_PENDING)
                    {
                        wchar_t buffer[52] = {0};
                        swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"发送错误，代码为%8x", GetLastError());

                        MessageBoxW(NULL,
                                    buffer,
                                    L"错误",
                                    MB_OK | MB_ICONERROR);
                    
                                    serialInit = 0;
                    }
                }
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
        ScreenToClient(hCanvas, &pt);

        RECT rc;
        GetClientRect(hCanvas, &rc);

        if (PtInRect(&rc,pt)){
            g_points[g_pointCount] = pt;
            g_pointCount++;

            InvalidateRect(hwnd, NULL, FALSE);

            SetFilePointer(hFile, 0, NULL, FILE_END);

            char pointInfo[100] = {0};

            char point16[30] = {0};
            sprintf(point16, "%02x%03x%03x",
                2,
                (pt.x * 2000 + canvasLen / 2) / canvasLen,
                (pt.y * 2000 + canvasLen / 2) / canvasLen
            );

            sprintf(pointInfo, "%02x%03x%03x -> (%0.1f, %0.1f)\n",
                2,
                (pt.x * 2000 + canvasLen / 2) / canvasLen,
                (pt.y * 2000 + canvasLen / 2) / canvasLen,
                (float)(pt.x * 200) / canvasLen,
                (float)(pt.y * 200) / canvasLen
            );

            WriteFile(hFile, pointInfo, strlen(pointInfo), NULL, NULL);
            
            if (serialInit)
            {
                char sendData[4];
                DWORD dataSendNumber = 0;
                OVERLAPPED overlapped = {0};

                for (int i = 0; i <= 8; i += 2)
                {
                    char byte[3] = {pointInfo[i], pointInfo[i + 1], '\0'};
                    sendData[i / 2] = (char)strtol(byte, NULL, 16);
                }

                overlapped.Offset = 0;
                overlapped.OffsetHigh = 0;

                int error = WriteFile(hSerial, sendData, 4, &dataSendNumber, &overlapped);

                if (!error && GetLastError() != ERROR_IO_PENDING)
                {
                    wchar_t buffer[52] = {0};
                    swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"发送错误，代码为%8x", GetLastError());

                    MessageBoxW(NULL,
                                buffer,
                                L"错误",
                                MB_OK | MB_ICONERROR);

                    serialInit = 0;
                }
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (g_startDraw)
        {
            g_startDraw = 0;
            ReleaseCapture(); 
            KillTimer(hwnd,g_timer);

            POINT pt;

            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);

            char pointInfo[100] = {0};

            char point16[30] = {0};

            sprintf(point16, "%02x%03x%03x",
                3,
                (pt.x * 2000 + canvasLen / 2) / canvasLen,
                (pt.y * 2000 + canvasLen / 2) / canvasLen
            );

            sprintf(pointInfo, "%02x%03x%03x -> (%0.1f, %0.1f)\n",
                3,
                (pt.x * 2000 + canvasLen / 2) / canvasLen,
                (pt.y * 2000 + canvasLen / 2) / canvasLen,
                (float)(pt.x * 200) / canvasLen,
                (float)(pt.y * 200) / canvasLen
            );

            SetFilePointer(hFile, 0, NULL, FILE_END);
            WriteFile(hFile, pointInfo, strlen(pointInfo), NULL, NULL);

            if (serialInit){
                char sendData[4];
                DWORD dataSendNumber = 0;
                OVERLAPPED overlapped = {0};

                for (int i = 0; i < 8; i += 2)
                {
                    char byte[3] = {pointInfo[i], pointInfo[i + 1], '\0'};
                    sendData[i / 2] = (char)strtol(byte, NULL, 16);
                }

                overlapped.Offset = 0;
                overlapped.OffsetHigh = 0;

                int error = WriteFile(hSerial, sendData, 4, &dataSendNumber, &overlapped);

                if (!error && GetLastError() != ERROR_IO_PENDING)
                {
                    wchar_t buffer[52] = {0};
                    swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"发送错误，代码为%8x", GetLastError());

                    MessageBoxW(NULL,
                                buffer,
                                L"错误",
                                MB_OK | MB_ICONERROR);

                    serialInit = 0;
                }
            }
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
        CloseHandle(hSerial);
        PostQuitMessage(0);
        return;
    case WM_SIZE:
        pw.windowSizeX = LOWORD(lParam);
        pw.windowSizeY = HIWORD(lParam);

        canvasLen = min(pw.windowSizeX * 3/4,pw.windowSizeY) * 5/6;

        int canvasX = (pw.windowSizeX * 3 / 4 - canvasLen)/2;
        int canvasY = (pw.windowSizeY - canvasLen) / 2;

        MoveWindow(hCanvas,canvasX,canvasY,canvasLen,canvasLen,TRUE);
    case WM_PAINT:
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        char testBuffer[52] = {0};
        sprintf(testBuffer, "COM is %d,baud rate is %d", comNumber, baudRate);

        TextOut(hdc, 0, 0, testBuffer, strlen(testBuffer));

        EndPaint(hwnd, &ps);
        break;
    }
}


int init_com(int com, int baud){
    comNumber = com;
    baudRate = baud;

    char portName[52] = {0};
    sprintf(portName, "\\\\.\\COM%d", comNumber);

    hSerial = CreateFile(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        MessageBoxW(NULL,
                    L"串口打开错误",
                    L"错误",
                    MB_OK | MB_ICONERROR);
                    serialInit = 0;
        return 0;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);

    GetCommState(hSerial,&dcb);

    dcb.BaudRate = baudRate;
    dcb.ByteSize = 8;
    dcb.Parity = 0;
    dcb.StopBits = 0;

    SetCommState(hSerial,&dcb);

    COMMTIMEOUTS timeouts = {0};

    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 1000;

    SetCommTimeouts(hSerial,&timeouts);

    serialInit = 1;

    return 1;
}