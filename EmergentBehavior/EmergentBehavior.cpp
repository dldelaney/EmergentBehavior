// EmergentBehavior.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "EmergentBehavior.h"
#include "WindowsX.h"
#include <math.h>
#include <string>

#define MAX_LOADSTRING 100
#define UPDATE_WINDOW1 0

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)

    
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EMERGENTBEHAVIOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EMERGENTBEHAVIOR));

    MSG msg;

    // Main message loop:
    bool started = false;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!started) {
            SetTimer(msg.hwnd, UPDATE_WINDOW1, 100, (TIMERPROC)NULL);
            started = true;
        }
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            //TODO - update only every 1/x seconds

        }
        
    }

    return (int) msg.wParam;
}



//
//  datastruct for a node
// color - the color of the circle (can be off/black)
//  point - the x-y point in the window
//  range - the range (radius) of the "signal" to notice other nodes (used in emergent behavior function)
//  diameter - the diameter of the node circle
//  isActive - if the node is being drawn
//

struct Node {
    unsigned char color[3] = { 255,0,0 }; //it won't accept byte for some reason
    short point[2] = { -1000,-1000 };
    short range = 100;
    short radius = 5;
    bool isActive = false;
    bool showRange = true;
    char message[30] = "";
};

Node mainNodeGroup[100];
short nodeCount = 0;
short mainNodeGroupSize = sizeof(mainNodeGroup) / sizeof(Node);
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EMERGENTBEHAVIOR));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EMERGENTBEHAVIOR);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
float getDist(int x1, int y1, int x2, int y2);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_LBUTTONDOWN:
        mainNodeGroup[nodeCount].point[0] = (short)GET_X_LPARAM(lParam);
        mainNodeGroup[nodeCount].point[1] = (short)GET_Y_LPARAM(lParam);
        mainNodeGroup[nodeCount].isActive = true;
        if (nodeCount < mainNodeGroupSize) {
            nodeCount++;
        }
        else {
            nodeCount = 0;
        }
        break;
    case WM_TIMER:
        switch (wParam) {
        case UPDATE_WINDOW1:
            RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            SetTimer(hWnd, UPDATE_WINDOW1, 70, (TIMERPROC)NULL);
            //TODO - apply rules here


             //check for messages in nodes that are within the range
            if (true) {
                for (int i = 0; i < mainNodeGroupSize; i++) {
                    for (int j = 0; j < mainNodeGroupSize; j++) {
                        if (mainNodeGroup[j].isActive) {
                            float distance = getDist(mainNodeGroup[i].point[0], mainNodeGroup[i].point[1],
                                mainNodeGroup[j].point[0], mainNodeGroup[j].point[1]);
                            if (distance < 0.1) {
                                //ignore it, it's the same point
                            }
                            else if (distance < mainNodeGroup[i].range) {
                                //read the message

                                //TODO - make and parse messages



                            }
                        }
                    }
                }
            }
            
            break;
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            //draw the background rectangle
            SelectObject(hdc, GetStockObject(DC_BRUSH));
            SetDCBrushColor(hdc, RGB(255, 255, 255));
            Rectangle(hdc, 0,0,100000, 100000);

            for (int i = 0; i < mainNodeGroupSize; i++) {//loop through all the nodes
                if (mainNodeGroup[i].isActive) {
                    SelectObject(hdc, GetStockObject(DC_BRUSH));
                    SetDCBrushColor(hdc, RGB(mainNodeGroup[i].color[0], mainNodeGroup[i].color[1], mainNodeGroup[i].color[2]));
                    //upper left hand coords(x,y), lower right hand coords(x,y)
                    Ellipse(hdc, mainNodeGroup[i].point[0] - mainNodeGroup[i].radius,
                        mainNodeGroup[i].point[1] - mainNodeGroup[i].radius,  //get starting x-y coordinates from object
                        mainNodeGroup[i].point[0] + mainNodeGroup[i].radius,
                        mainNodeGroup[i].point[1] + mainNodeGroup[i].radius);//get ending x-y coordinates from starting position and radius

                    //show the range of the node
                    if (mainNodeGroup[i].showRange) {
                        SelectObject(hdc, GetStockObject(NULL_BRUSH));
                        SetDCPenColor(hdc, RGB(0,0,0));
                        Ellipse(hdc, mainNodeGroup[i].point[0] - mainNodeGroup[i].range,
                            mainNodeGroup[i].point[1] - mainNodeGroup[i].range,
                            mainNodeGroup[i].point[0] + mainNodeGroup[i].range,
                            mainNodeGroup[i].point[1] + mainNodeGroup[i].range);
                    }
                }

            }

          
            

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


float getDist(int x1, int y1, int x2, int y2) {
    return sqrt((float)abs(((x2 - x1)* (x2 - x1)) + ((y2 - y1)* (y2 - y1))));
}