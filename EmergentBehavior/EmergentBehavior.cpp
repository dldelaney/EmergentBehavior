// EmergentBehavior.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "EmergentBehavior.h"
#include "WindowsX.h"
#include <math.h>
#include <string>
#include <cstdlib>
#include <iostream>

#define MAX_LOADSTRING 100
#define UPDATE_WINDOW1 0
#define WINDOW_UPDATE_TIME 200 //time in millis between window updates
#define NODE_COUNT 1000

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
            SetTimer(msg.hwnd, UPDATE_WINDOW1, WINDOW_UPDATE_TIME, (TIMERPROC)NULL); //start the timer 
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
    unsigned char externalColor[3] = { 255,0,0 }; //used for drawing the node (it won't accept byte for some reason)
    short point[2] = { 0,0 };  //x, y
    short range = 100;
    short radius = 5;
    bool isActive = false;
    bool showRange = false;
    char internalColor = -1; //used for color logic
    //colors: R, O, G, B, I, V //took out yellow bc it wouldn't look good, <-- number is respective to that list 

    //12-23-20 removed msg and state bc they were useless (not doing RSSI anymore)
};
int windowWidth = 0;
int windowHeight = 0;
bool windowInitialized = false;

const int initNodeCount = NODE_COUNT; //for some reason putting NODE_COUNT directly into mainNodeGroup doesn't work
Node mainNodeGroup[initNodeCount];
short nodeCount = 0;
short mainNodeGroupSize = sizeof(mainNodeGroup) / sizeof(Node);
//----------------------------------------
//FUNCION DEFINITIONS
//---------------------------------------
float distToNode(Node node1, Node node2);
int getMinIndexCharArr(char arr[]);
char* noColorsIndex(char arr[]);
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
   RECT initWindowSize;
   if (GetWindowRect(hWnd, &initWindowSize)) {
       windowWidth = initWindowSize.right - initWindowSize.left;
       windowHeight = initWindowSize.bottom - initWindowSize.top;
   }//TODO: update this when the window changes sizes
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
         //keeping this in in case I want to use it later for testing (places node at mouse cursor)
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

            //place the nodes in random positions the first frame
            if (!windowInitialized) { //if it's the first frame
                for (int i = 0; i < mainNodeGroupSize; i++) { //place each node randomly
                    //RAND_MAX is 32767, so using % makes it between 0 and windowWidth
                    mainNodeGroup[i].point[0] = rand() % windowWidth; 
                    mainNodeGroup[i].point[1] = rand() % windowHeight;
                    //TODO: add check to see if it's too close to another node (and re-roll if so)?
                    mainNodeGroup[i].isActive = true;
                }
                windowInitialized = true;
            }



            //TODO - apply emergent behavior rules here to trigger every cycle

            //get number of nodes within radius (as well as color (black color is considered "off")
            std::string testState = "rainbowChange"; //or gameOfLife or rainbow (no color changing) or flickering

            

            if (testState == "rainbowChange") {
                //get the number of nodes within range of each node
                char surroundingColors[6] = {'-', '-', '-', '-', '-', '-' }; //R, O, G, B, I, V respectively
                /*
                Get colors of surrounding nodes
                if a color is not represented, take it
                if all colors are represented, take the one with the least amount of nodes
                if the least amount is tied, pick one at random
                */
                for (int mainNodeNum = 0; mainNodeNum < mainNodeGroupSize; mainNodeNum++) {

                    for (int otherNode = 0; otherNode < mainNodeGroupSize; otherNode++) {

                        //check if the node is within range
                        if (distToNode(mainNodeGroup[mainNodeNum], mainNodeGroup[otherNode]) < mainNodeGroup[mainNodeNum].range) {
                            //check to see if it has a color
                            if (mainNodeGroup[otherNode].internalColor != -1) {
                                //if it does, add one to the index of the color of the node, effectively making a chart of the most-used colors
                                surroundingColors[mainNodeGroup[otherNode].internalColor]++;
                            }
                            //if it doesn't have a color, it doesn't matter, it actually helps to not add it to any list
                        }
                    }//end inner for loop, chart of surrounding colors is now filled
                    //back to the singular node now
                    
                    //get the min value index of the array
                    //there's probably a better and more elegant way to do this
                    int minIndex = getMinIndexCharArr(surroundingColors);
                    if (surroundingColors[minIndex] > -1) {
                        //if that value is not -1 (-1 is the "no color" number)
                        //take that color
                        mainNodeGroup[mainNodeNum].internalColor = surroundingColors[minIndex];
                    }
                    else {
                        //if it is -1
                        //count how many -1s are in the array and pick randomly between them
                        char* noColorsList = noColorsIndex(surroundingColors);
                        int noColorsListSize = sizeof(noColorsList) / sizeof(char);
                        //get the randomly selected color (noColorsList is actually a list of colors that do not exist in surroundingColors)
                        int chosenColor = noColorsList[rand() % noColorsListSize];
                        mainNodeGroup[mainNodeNum].internalColor = chosenColor;

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
            SelectObject(hdc, GetStockObject(DC_BRUSH));//paint the background
            SetDCBrushColor(hdc, RGB(0, 0, 0));
            Rectangle(hdc, 0,0,100000, 100000);

            for (int i = 0; i < mainNodeGroupSize; i++) {//loop through all the nodes
                if (mainNodeGroup[i].isActive) {
                    SelectObject(hdc, GetStockObject(DC_BRUSH));
                    SetDCBrushColor(hdc, RGB(mainNodeGroup[i].externalColor[0], mainNodeGroup[i].externalColor[1], mainNodeGroup[i].externalColor[2]));
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


float distToNode(Node node1, Node node2) {
    int x1 = node1.point[0];
    int y1 = node1.point[1];
    int x2 = node2.point[0];
    int y2 = node2.point[1];
    return sqrt((float)abs(((x2 - x1)* (x2 - x1)) + ((y2 - y1)* (y2 - y1))));
}

int getMinIndexCharArr(char arr[]) {
    int minIndex = 0;
    int arraySize = sizeof(arr) / sizeof(char);
    int i = 0;
    for (i; i < arraySize; i++) {
        if (arr[i] < arr[minIndex]) {
            minIndex = i;
        }
    }
    return i;
}

char* noColorsIndex(char arr[]) {
    int arrIndex = 0;
    int arraySize = sizeof(arr) / sizeof(char);
    //count the number of noColors
    char finalArray[6];
    for (int i = 0; i < arraySize; i++) {
        if (arr[i] == -1) {

            arrIndex++;

        }
    }
    
}