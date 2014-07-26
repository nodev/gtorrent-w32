// Needs some error checking and refactoring
// GUI is pretty hacky atm but thats Win32 for you
//

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include "gtorrent.h"
#include "util.h"

extern "C"
{
#include "gtorrent-core.h"
}

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hTorrentList;								// Torrent Listview
HWND hStatusTree;								// Sidebar tree
HWND hDetailsTab;								// Details tab pane
HWND hToolBar;
HWND hMainFrame;
HWND hMainWindow;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
HWND				CreateTorrentListView(HWND hwndParent);
HWND				CreateStatusTreeView(HWND hwndParent);
HWND				CreateDetailTabView(HWND hwndParent);
HWND				CreateToolbar(HWND hWndParent);
HWND				CreateMainFrame(HWND hwndParent);

void GetWindowPos(HWND hWnd, int *x, int *y)
{
	HWND hWndParent = GetParent(hWnd);
	POINT p = { 0 };

	MapWindowPoints(hWnd, hWndParent, &p, 1);

	(*x) = p.x;
	(*y) = p.y;
}

bool IsMouseOver(HWND hWnd, int mousex, int mousey)
{
	int x, y;
	RECT rect;

	GetClientRect(hWnd, &rect);
	GetWindowPos(hWnd, &x, &y);

	if (mousex >= x && mousex <= (rect.right + x)
		&& mousey >= y && mousey <= (rect.bottom + y))
	{
		return TRUE;
	}
	return FALSE;
}

bool InitGTCore(void)
{
	gt_core *g = core_create();

	return (g != NULL);
}

#define CUSTOM_WC   _T("CustomControl")

void CustomPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;

	GetClientRect(hwnd, &rect);

	hdc = BeginPaint(hwnd, &ps);
	EndPaint(hwnd, &ps);
}


LRESULT CALLBACK CustomProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_PAINT:
		CustomPaint(hwnd);
		return 0;

	case WM_COMMAND:
		SendMessage(hMainWindow, uMsg, wParam, lParam);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CustomRegister(void)
{
	WNDCLASS wc = { 0 };

	wc.style = CS_GLOBALCLASS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = CustomProc;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = CUSTOM_WC;
	RegisterClass(&wc);
}

void CustomUnregister(void)
{
	UnregisterClass(CUSTOM_WC, NULL);
}


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	if (!InitGTCore())
	{
		MessageBox(NULL, TEXT("Error initializing gTorrent-Core!"), TEXT("ERROR"), MB_OK);
	}

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES | ICC_TAB_CLASSES;
	InitCommonControlsEx(&icex);

	CustomRegister();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_GTORRENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GTORRENT));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	CustomUnregister();

	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GTORRENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_GTORRENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   hMainWindow = hWnd;

   hStatusTree = CreateStatusTreeView(hWnd);
   hMainFrame = CreateMainFrame(hWnd);
		hToolBar = CreateToolbar(hMainFrame);
		hTorrentList = CreateTorrentListView(hMainFrame);
   hDetailsTab = CreateDetailTabView(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void ResizeWindow(DWORD dwSplitterPos)
{
	int x, y, w, h;
	RECT r;

	GetClientRect(hMainWindow, &r);
	x = y = 0;
	w = dwSplitterPos - 1;
	h = r.bottom;
	MoveWindow(hStatusTree, x, y, w, h, TRUE);
	UpdateWindow(hMainWindow);

	x = dwSplitterPos + 2;
	y = 0;
	w = r.right - dwSplitterPos - 2;
	h = r.bottom / 2;
	MoveWindow(hMainFrame, x, y, w, h, TRUE);

	GetClientRect(hToolBar, &r);
	MoveWindow(hToolBar, 0, 0, w, r.bottom, TRUE);

	MoveWindow(hTorrentList, 0, r.bottom, w, h - r.bottom, TRUE);
	UpdateWindow(hMainWindow);

	GetClientRect(hMainWindow, &r);
	x = dwSplitterPos + 2;
	y = (r.bottom - r.top) / 2;
	w = r.right - dwSplitterPos - 2;
	h = r.bottom / 2;
	MoveWindow(hDetailsTab, x, y, w, h, TRUE);
	UpdateWindow(hMainWindow);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	RECT rect;
	static HCURSOR 	hCursorWE;
	static HCURSOR 	hCursorNS;
	static BOOL	bSplitterMoving;
	static DWORD	dwSplitterV, dwSplitterH;
	static bool bHSplitter;

	switch (message)
	{

	case WM_CREATE:
		hCursorWE = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE));
		hCursorNS = LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS));
		bSplitterMoving = FALSE;
		bHSplitter = FALSE;

		GetClientRect(hWnd, &rect);

		// Rough approximation for treeview width
		dwSplitterV = (rect.right - rect.left) / 7;
		dwSplitterH = (rect.bottom - rect.top) / 2;
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case IDM_ADD_TORRENT:
			MessageBox(hWnd, TEXT("Add torrent!"), TEXT("Add"), MB_OK);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_SIZE:
		ResizeWindow(dwSplitterV);
		break;

	case WM_MOUSEMOVE:

		if (IsMouseOver(hDetailsTab, LOWORD(lParam), HIWORD(lParam)) && !bSplitterMoving)
			return 0;

		if (LOWORD(lParam) > 10)
		{
			SetCursor(hCursorWE);
			if ((wParam == MK_LBUTTON) && bSplitterMoving)
			{
				GetClientRect(hWnd, &rect);
				if (LOWORD(lParam) > rect.right)
					return 0;

				dwSplitterV = LOWORD(lParam);
				SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
			}
		}
		//else if (HIWORD(lParam) > 10)
		//{
		//	SetCursor(hCursorNS);
		//	if ((wParam == MK_LBUTTON) && bSplitterMoving)
		//	{
		//		GetClientRect(hWnd, &rect);
		//		if (HIWORD(lParam) > rect.bottom)
		//			return 0;

		//		dwSplitterH = HIWORD(lParam);
		//		SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(rect.right, rect.bottom));
		//	}
		//}
		break;


	case WM_LBUTTONDOWN:
		if (!IsMouseOver(hDetailsTab, LOWORD(lParam), HIWORD(lParam)))
		{
			SetCursor(hCursorWE);
			bSplitterMoving = TRUE;
			SetCapture(hWnd);
		}
		break;


	case WM_LBUTTONUP:
		ReleaseCapture();
		bSplitterMoving = FALSE;
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
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

TCHAR *pszColumnLabels[] =
{
	TEXT("Number"),
	TEXT("Name"),
	TEXT("Size"),
	TEXT("Status"),
	TEXT("Down Speed"),
	TEXT("Up Speed"),
	TEXT("ETA"),
	TEXT("Seeds/Peers"),
	TEXT("Added"),
	TEXT("Completed On")
};

HWND CreateMainFrame(HWND hwndParent)
{
	RECT rcClient;                       // The parent window's client area.

	GetClientRect(hwndParent, &rcClient);

	HWND hGroupBox = CreateWindow(CUSTOM_WC, L"",
		WS_CHILD | WS_VISIBLE,
		(rcClient.right - rcClient.left) / 2,
		0,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		hwndParent, NULL, hInst, NULL);

	return hGroupBox;
}

HWND CreateTorrentListView(HWND hwndParent)
{
	RECT rcClient;

	// Toolbar must be created before listview
	GetClientRect(hToolBar, &rcClient);
	int iHeight = rcClient.bottom;

	GetClientRect(hwndParent, &rcClient);

	// Create the list-view window in report view with label editing enabled.
	HWND hWndListView = CreateWindow(WC_LISTVIEW,
		TEXT(""),
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE,
		0,
		iHeight,
		rcClient.right,
		rcClient.bottom - iHeight,
		hwndParent,
		NULL,
		hInst,
		NULL);


	ListView_DeleteAllItems(hWndListView);

	LV_COLUMN   lvColumn;
	ZeroMemory(&lvColumn, sizeof(lvColumn));

	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT /*| LVCF_SUBITEM*/;
	lvColumn.fmt = LVCFMT_LEFT;

	for (int i = 0; i < (sizeof(pszColumnLabels) / sizeof(pszColumnLabels[0])); i++)
	{
		lvColumn.pszText = pszColumnLabels[i];
		ListView_InsertColumn(hWndListView, i, &lvColumn);
		ListView_SetColumnWidth(hWndListView, i, LVSCW_AUTOSIZE_USEHEADER);
	}

	// This is a hack to avoid first column taking all the width
	ListView_SetColumnWidth(hWndListView, 0, LVSCW_AUTOSIZE_USEHEADER);

	ListView_SetExtendedListViewStyle(hWndListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	return (hWndListView);
}

TCHAR *pszStatusTreeLables[] =
{
	TEXT("Downloading"),
	TEXT("Seeding"),
	TEXT("Completed"),
	TEXT("Active"),
	TEXT("Inactive")
};

HWND CreateStatusTreeView(HWND hwndParent)
{
	RECT rcClient;
	HWND hwndTV;

	GetClientRect(hwndParent, &rcClient);
	hwndTV = CreateWindow(WC_TREEVIEW,
		TEXT("Tree View"),
		WS_VISIBLE | WS_CHILD | WS_BORDER | TVS_HASLINES,
		0,
		0,
		rcClient.right/2,
		rcClient.bottom,
		hwndParent,
		NULL,
		hInst,
		NULL);

	TreeView_DeleteAllItems(hwndTV);

	TVINSERTSTRUCT tvins;
	HTREEITEM parent;

	tvins.hParent = TVI_ROOT;
	tvins.hInsertAfter = TVI_FIRST;
	tvins.item.mask = TVIF_TEXT;
	tvins.item.pszText = TEXT("Torrents");

	parent = TreeView_InsertItem(hwndTV, &tvins);

	tvins.hParent = parent;

	for (int i = 0; i < (sizeof(pszStatusTreeLables) / sizeof(pszStatusTreeLables[0])); i++)
	{
		tvins.hInsertAfter = TVI_LAST;
		tvins.item.pszText = pszStatusTreeLables[i];
		TreeView_InsertItem(hwndTV, &tvins);
	}

	TreeView_Expand(hwndTV, parent, TVE_EXPAND);

	return hwndTV;
}

TCHAR *pszTabLabels[] =
{
	TEXT("Files"),
	TEXT("Info"),
	TEXT("Peers"),
	TEXT("Trackers"),
	TEXT("Pieces"),
	TEXT("Speed"),
	TEXT("Logger")
};

HWND CreateDetailTabView(HWND hwndParent)
{
	RECT rcClient;
	HWND hwndTab;
	TCITEM tie;

	GetClientRect(hwndParent, &rcClient);

	hwndTab = CreateWindow(WC_TABCONTROL, TEXT(""),
		WS_CHILD | WS_VISIBLE,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		(rcClient.right - rcClient.left) / 2,
		(rcClient.bottom - rcClient.top) / 2,
		hwndParent, NULL, hInst, NULL);

	tie.mask = TCIF_TEXT;

	for (int i = 0; i < (sizeof(pszTabLabels) / sizeof(pszTabLabels[0])); i++)
	{
		tie.pszText = pszTabLabels[i];
		if (TabCtrl_InsertItem(hwndTab, i, &tie) == -1)
		{
			DestroyWindow(hwndTab);
			return NULL;
		}
	}

	return hwndTab;
}

HIMAGELIST hImageList = NULL;

HWND CreateToolbar(HWND hWndParent)
{
	const int ImageListID = 0;
	const int numButtons = 5;
	const int bitmapSize = 32;

	const DWORD buttonStyles = BTNS_AUTOSIZE;

	TBBUTTON tbButtons[numButtons];
	HICON ico;

	HWND hWndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE | TBSTYLE_LIST,
		0, 0, 0, 0,
		hWndParent, NULL, hInst, NULL);

	if (hWndToolbar == NULL)
		return NULL;

	hImageList = ImageList_Create(bitmapSize, bitmapSize, ILC_COLOR32 | ILC_MASK, numButtons, 0);

	SendMessage(hWndToolbar, TB_SETIMAGELIST,
		(WPARAM)ImageListID,
		(LPARAM)hImageList);

	SendMessage(hWndToolbar, TB_LOADIMAGES,	(WPARAM)IDB_STD_LARGE_COLOR, (LPARAM)HINST_COMMCTRL);

	for (int i = 0; i < numButtons; i++)
	{
		ico = LoadIcon(hInst, MAKEINTRESOURCE(IDI_NEWFILE + i));
		int idx = ImageList_AddIcon(hImageList, ico);
		tbButtons[i].iBitmap = idx;
		tbButtons[i].idCommand = IDM_ADD_TORRENT + i;
		tbButtons[i].fsState = TBSTATE_ENABLED;
		tbButtons[i].fsStyle = buttonStyles;
		tbButtons[i].dwData = 0L;
		tbButtons[i].iString = 0;
	}

	SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
	SendMessage(hWndToolbar, TB_ADDBUTTONS, (WPARAM)numButtons, (LPARAM)&tbButtons);

	SendMessage(hWndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(hWndToolbar, TRUE);

	return hWndToolbar;
}

