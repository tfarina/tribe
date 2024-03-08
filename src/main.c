/* Windows Header Files */
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>

/* C RunTime Header Files */
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "db.h"
#include "resource.h"
#include "alpm_list.h"
#include "arraysize.h"

#define MAX_LOADSTRING 100
#define MAX_STRING_RES_LENGTH 255
#define MAX_EDIT_LENGTH 100

/* Initial window size. */
#define INIT_WINDOW_WIDTH  500
#define INIT_WINDOW_HEIGHT 350

/* The width and height of toolbar button images */
#define TB_BITMAP_CX 24
#define TB_BITMAP_CY 24

/* Defines the number of columns in the contact list view */
#define CONTACT_LIST_COLUMN_COUNT 3

typedef struct _CONTACT
{
	TCHAR szFirstName[MAX_LOADSTRING];
	TCHAR szLastName[MAX_LOADSTRING];
	TCHAR szEmail[MAX_LOADSTRING];
} CONTACT, *LPCONTACT;

HINSTANCE g_hInst;     /* Instance handle to this app */
HWND g_hwndMain;       /* Window handle to this app */
HWND g_hwndStatusBar;  /* Window handle to status window */
HWND g_hwndListView;   /* Window handle to list view */
HWND g_hwndToolbar;    /* Window handle to toolbar */

LPTSTR g_szClassName = TEXT("TribeAB");  /* Window class name */

LONG g_hStatus;

static alpm_list_t *contactList = NULL;

BOOL
InitWindowClass(
	HINSTANCE
	);
BOOL
CreateMainWindow(
	HINSTANCE,
	int
	);
void
CreateChildrenControls(
	HWND
	);
void
CreateListView(
	HWND
	);
void
CreateToolbar(
	HWND
	);
void
CreateStatusBar(
	HWND
	);
void
PopulateListView(
	void
	);
void
AdjustChildrenControls(
	HWND
	);
void
SelectAllItems(
	void
	);
static
void
UpdateUI(
	void
	);
LRESULT CALLBACK
MainWndProc(
	HWND,
	UINT,
	WPARAM,
	LPARAM
	);
LRESULT CALLBACK
AboutDlgProc(
	HWND,
	UINT,
	WPARAM,
	LPARAM
	);

int WINAPI
WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{
	MSG msg;
	HACCEL hAccelTable;  /* Handle to accelerator table */
	INITCOMMONCONTROLSEX iccex;
	int rc;

	g_hInst = hInstance;

	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_ANIMATE_CLASS |
		ICC_BAR_CLASSES |
		ICC_COOL_CLASSES |
		ICC_DATE_CLASSES |
		ICC_LISTVIEW_CLASSES |
		ICC_NATIVEFNTCTL_CLASS |
		ICC_PROGRESS_CLASS |
		ICC_TAB_CLASSES |
		ICC_WIN95_CLASSES;

	InitCommonControlsEx(&iccex);

	/* Initialize the window */
	if (!InitWindowClass(hInstance))
	{
		return FALSE;
	}

	/* Create the main window */
	if (!CreateMainWindow(hInstance, nCmdShow))
	{
		return FALSE;
	}

	rc = db_init();
	if (rc < 0)
	{
		/* We should show a Message Box with an error message here. */
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_MAINWND_ACCEL));

	/* Main Message Loop */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(g_hwndMain, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

/*
 *  FUNCTION: InitWindowClass()
 *
 *  PURPOSE: Registers the window class.
 *
 *  COMMENTS:
 *
 *    This function and its usage is only necessary if you want this code
 *    to be compatible with Win32 systems prior to the 'RegisterClassEx'
 *    function that was added to Windows 95. It is important to call this function
 *    so that the application will get 'well formed' small icons associated
 *    with it.
 */
BOOL
InitWindowClass(
	HINSTANCE hInstance
	)
{
	WNDCLASSEX wcex;

	wcex.cbSize         = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)MainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_BTNFACE+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= g_szClassName;
	wcex.hIconSm		= LoadImage(hInstance,
								    MAKEINTRESOURCE(IDI_APP_ICON),
									IMAGE_ICON, 16, 16,
									LR_DEFAULTCOLOR);

	if (!RegisterClassEx(&wcex))
	{
		return FALSE;
	}

	return TRUE;
}

/*
 *   FUNCTION: CreateMainWindow(HANDLE, int)
 *
 *   PURPOSE: Creates main window
 *
 *   COMMENTS:
 *
 *        In this function, we create and display the main program window.
 */
BOOL
CreateMainWindow(
	HINSTANCE hInstance,
	int nCmdShow
	)
{
	TCHAR szTitle[MAX_STRING_RES_LENGTH];  /* Title bar text */
	HMENU hMenu;

	LoadString(hInstance, IDS_APP_NAME, szTitle, ARRAYSIZE(szTitle));

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENUBAR));

	/* Create the main window for this application instance. */
	g_hwndMain = CreateWindowEx(
		0,
		g_szClassName,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		INIT_WINDOW_WIDTH,
		INIT_WINDOW_HEIGHT,
		NULL,
		hMenu,
		hInstance,
		NULL);

	if (!g_hwndMain)
	{
		return FALSE;
	}

	ShowWindow(g_hwndMain, nCmdShow);
	UpdateWindow(g_hwndMain);

	return TRUE;
}

int
load_contacts(
	void
	)
{
	HRESULT hr;
	ULONG cContacts;
	LPCONTACTROW lpContacts = NULL;
	ULONG index;
	LPCONTACT lpContact;

	hr = GetContactList(&cContacts, &lpContacts);
	if (FAILED(hr))
		goto exit;

	for (index = 0; index < cContacts; index++)
	{
		lpContact = LocalAlloc(LMEM_ZEROINIT, sizeof(CONTACT));
		if (!lpContact)
		{
			/* Out of memory. */
			goto exit;
		}
		lstrcpy(lpContact->szFirstName, lpContacts[index].szFirstName);
		lstrcpy(lpContact->szLastName, lpContacts[index].szLastName);
		lstrcpy(lpContact->szEmail, lpContacts[index].szEmail);

		contactList = alpm_list_add(contactList, lpContact);
	}

	return 0;

exit:
	return -1;
}

void
PopulateListView(
	void
	)
{
	alpm_list_t *item;
	LPCONTACT contact;
	LVITEM lvI = {0};
	int index = 0;

	lvI.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	lvI.iItem = 0;

	for (item = contactList; item; item = alpm_list_next(item))
	{
		contact = (LPCONTACT) item->data;
		lvI.iSubItem = 0; /* COL_FIRST_NAME */
		lvI.lParam = (LPARAM) contact;
		lvI.pszText = contact->szFirstName;

		index = ListView_InsertItem(g_hwndListView, &lvI);
		if (index != -1)
		{
			ListView_SetItemText(g_hwndListView, index, 1 /* COL_LAST_NAME*/, contact->szLastName);
			ListView_SetItemText(g_hwndListView, index, 2 /* COL_EMAIL */, contact->szEmail);
		}
		lvI.iItem++;
	}

	ListView_SetItemState(
		g_hwndListView,
		0,
		LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);

	ListView_EnsureVisible(g_hwndListView, 0, FALSE);
}

void
CreateListView(
	HWND hWndParent
	)
{
	DWORD dwStyle;
	LVCOLUMN lvc;
	int iCol;
	TCHAR *szColumnLabels[CONTACT_LIST_COLUMN_COUNT] =
	{
		TEXT("First Name"),
		TEXT("Last Name"),
		TEXT("Email")
	};

	g_hwndListView = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		WC_LISTVIEW,
		(LPTSTR) NULL,
		WS_TABSTOP | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE |
		WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS | WS_EX_CLIENTEDGE,
		0,
		0,
		0,
		0,
		hWndParent,
		(HMENU) IDC_LISTVIEW,
		g_hInst,
		NULL);

	dwStyle = GetWindowLong(g_hwndListView, GWL_STYLE);
	if (dwStyle & LVS_EDITLABELS)
	{
		dwStyle &= ~LVS_EDITLABELS;
	}
	SetWindowLong(g_hwndListView, GWL_STYLE, dwStyle);

	/* Set styles */
	ListView_SetExtendedListViewStyle(g_hwndListView, LVS_EX_FULLROWSELECT);

	/* Insert columns */
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;

	for (iCol = 0; iCol < CONTACT_LIST_COLUMN_COUNT; iCol++)
	{
		lvc.iSubItem = iCol;
		lvc.pszText = szColumnLabels[iCol];
		ListView_InsertColumn(g_hwndListView, iCol, &lvc);
	}
}

void
CreateToolbar(
	HWND hWndParent
	)
{
	HIMAGELIST hImageList = NULL;
	int const imageListID = 0;

	TBBUTTON tbButtons[] =
	{
		{ MAKELONG(STD_FILENEW, imageListID), IDC_TB_NEW, TBSTATE_ENABLED, BTNS_AUTOSIZE, {0}, 0, 0 },
		{ MAKELONG(STD_PROPERTIES, imageListID), IDC_TB_PROPERTIES, 0, BTNS_AUTOSIZE, {0}, 0, 1 },
		{ MAKELONG(STD_DELETE, imageListID), IDC_TB_DELETE, 0, BTNS_AUTOSIZE, {0}, 0, 2 },
	};

	g_hwndToolbar = CreateWindowEx(
		0,                /* ex style */
		TOOLBARCLASSNAME, /* class name - defined in commctrl.h */
		(LPTSTR)NULL,     /* dummy text */
		WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE,  /* style */
		0,                /* x position */
		0,                /* y position */
		0,                /* width */
		0,                /* height */
		hWndParent,
		NULL,
		g_hInst,
		NULL);

	if (!g_hwndToolbar)
		return;

	hImageList = ImageList_Create(
		TB_BITMAP_CX,
		TB_BITMAP_CY,
		ILC_COLOR32 | ILC_MASK,
		0,
		0);

	/* Set the image list */
	SendMessage(g_hwndToolbar, TB_SETIMAGELIST, (WPARAM) imageListID, (LPARAM) hImageList);

	/* Load the button images */
	SendMessage(g_hwndToolbar, TB_LOADIMAGES, (WPARAM) IDB_STD_LARGE_COLOR, (LPARAM) HINST_COMMCTRL);

	/* Add buttons */
	SendMessage(g_hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
	SendMessage(g_hwndToolbar, TB_ADDBUTTONS, (WPARAM) ARRAYSIZE(tbButtons), (LPARAM) &tbButtons);

	/* Add strings */
	SendMessage(g_hwndToolbar, TB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT("New\0Properties\0Delete\0\0"));

	/* Resize the toolbar, and then show it */
	SendMessage(g_hwndToolbar, TB_AUTOSIZE, 0, 0);
	ShowWindow(g_hwndToolbar, TRUE);
}

void
CreateStatusBar(
	HWND hWndParent
	)
{
    RECT statusBarRect;

	g_hwndStatusBar = CreateWindowEx(
		0,                /* ex style */
		STATUSCLASSNAME,  /* class name - defined in commctrl.h */
		(LPTSTR) NULL,    /* dummy text */
		WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,  /* style */
		0,                /* x position */
		0,                /* y position */
		0,                /* width */
		0,                /* height */
		hWndParent,       /* parent */
		(HMENU) IDC_STATUSBAR, /* ID */
		g_hInst,          /* instance */
		NULL);            /* no extra data */

	GetClientRect(g_hwndStatusBar, &statusBarRect);

	/* Calculate the height of statusbar. */
	g_hStatus = statusBarRect.bottom - statusBarRect.top;
}

void
CreateChildrenControls(
	HWND hWndParent
	)
{
	CreateToolbar(hWndParent);

	CreateStatusBar(hWndParent);

	CreateListView(hWndParent);

	load_contacts();

	PopulateListView();

	SetFocus(g_hwndListView);

	UpdateUI();
}

void
AdjustChildrenControls(
	HWND hWndParent
	)
{
	RECT rc;
	RECT toolbarRect;
	int tbX, tbY, tbW, tbH;
	int statusX, statusY, statusW, statusH;
	int lvX, lvY, lvW, lvH;

    GetClientRect(hWndParent, &rc);
	GetClientRect(g_hwndToolbar, &toolbarRect);

	tbX = 0;
	tbY = rc.top;
	tbW = rc.right - rc.left;
	tbH = toolbarRect.bottom - toolbarRect.top;

	statusX = 0;
	statusY = rc.bottom - g_hStatus;
	statusW = rc.right - rc.left;
	statusH = g_hStatus;

	lvX = rc.left;
    lvY = rc.top;
	lvW = rc.right - rc.left;
	lvH = rc.bottom - rc.top;

	/*
	 * If statusbar is visible, reduce the height of listview,
	 * otherwise it will take the whole are.
	 */
	if (IsWindowVisible(g_hwndStatusBar))
	{
		lvH = lvH - statusH;
	}

	if (IsWindowVisible(g_hwndToolbar))
	{
		lvY = lvY + tbH;
		lvH = lvH - tbH - 5;
	}

	MoveWindow(g_hwndToolbar,
		       tbX,
		       tbY,
		       tbW,
		       tbH,
		       TRUE);

	MoveWindow(g_hwndStatusBar,
		       statusX,
		       statusY,
		       statusW,
		       statusH,
		       TRUE);

	MoveWindow(g_hwndListView,
		       lvX,
			   lvY,
			   lvW,
			   lvH,
			   TRUE);
}

void
SelectAllItems(
	void
	)
{
	int i;
	int iCount = ListView_GetItemCount(g_hwndListView);

	for (i = 0; i < iCount; i++)
	{
		ListView_SetItemState(g_hwndListView, i, LVIS_SELECTED, LVIS_SELECTED);
	}
}

INT_PTR CALLBACK
fnNamePageProc(
	HWND hWndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	TCHAR szBuf[] = TEXT("Not implemented yet!");
	TCHAR szCaption[MAX_STRING_RES_LENGTH];

	LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));

	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY: /* OK */
			{
				TCHAR szFirstName[MAX_EDIT_LENGTH];
				szFirstName[0] = '\0';

				GetDlgItemText(hWndDlg, IDC_PROPPAGE_NAME_EDIT_FIRSTNAME, szFirstName, ARRAYSIZE(szFirstName));
				if (!lstrlen(szFirstName))
				{
					TCHAR szMsg[MAX_STRING_RES_LENGTH];

					LoadString(g_hInst, IDS_NEEDS_FIRSTNAME, szMsg, ARRAYSIZE(szMsg));
					MessageBox(hWndDlg, szMsg, szCaption, MB_ICONEXCLAMATION | MB_OK);
					SetFocus(GetDlgItem(hWndDlg, IDC_PROPPAGE_NAME_EDIT_FIRSTNAME));
					SetWindowLongPtr(hWndDlg, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				MessageBox(hWndDlg, szBuf, szCaption, MB_ICONWARNING | MB_OK);
			}
			break;
		case PSN_RESET: /* Cancel */
			break;
		}
		break; /* WM_NOTIFY */

	default:
		break;
	} /* switch */

	return FALSE;
}

static void
ShowContactPropertiesDialog(
	HWND hWnd
	)
{
	PROPSHEETPAGE propSheetPage[1];
	PROPSHEETHEADER propSheetHeader;

	propSheetPage[0].dwSize = sizeof(PROPSHEETPAGE);
	propSheetPage[0].dwFlags = PSP_USETITLE;
	propSheetPage[0].hInstance = g_hInst;
	propSheetPage[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_NAME);
	propSheetPage[0].pszIcon = NULL;
	propSheetPage[0].pszTitle = TEXT("Name");
	propSheetPage[0].pfnDlgProc = fnNamePageProc;
	propSheetPage[0].lParam = 0;

	propSheetHeader.dwSize = sizeof(PROPSHEETHEADER);
	propSheetHeader.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	propSheetHeader.hwndParent = hWnd;
	propSheetHeader.hInstance = g_hInst;
	propSheetHeader.pszIcon = NULL;
	propSheetHeader.pszCaption = TEXT("Properties");
	propSheetHeader.nPages = sizeof(propSheetPage) / sizeof(PROPSHEETPAGE);
	propSheetHeader.nStartPage = 0;
	propSheetHeader.ppsp = (LPCPROPSHEETPAGE) &propSheetPage;
	propSheetHeader.pfnCallback = NULL;

	PropertySheet(&propSheetHeader);
}

static void
UpdateUI(
	void
	)
{
	int numSelected = ListView_GetSelectedCount(g_hwndListView);
	HMENU hMenu = GetSubMenu(GetMenu(g_hwndMain), 0);
	BOOL canDelete = numSelected > 0 ? TRUE : FALSE;
	UINT uFlag = canDelete ? MF_ENABLED : MF_GRAYED;
	BOOL canEdit = numSelected == 1 ? TRUE : FALSE;
	UINT uFlagEdit = canEdit ? MF_ENABLED : MF_GRAYED;

	SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, (WPARAM) IDC_TB_DELETE, (LPARAM) MAKELONG(canDelete, 0));
	SendMessage(g_hwndToolbar, TB_ENABLEBUTTON, (WPARAM) IDC_TB_PROPERTIES, (LPARAM) MAKELONG(canEdit, 0));

	EnableMenuItem(hMenu, IDM_DELETE, MF_BYCOMMAND | uFlag);
	EnableMenuItem(hMenu, IDM_PROPERTIES, MF_BYCOMMAND | uFlagEdit);
}

static void
HandleListViewNotifications(
	HWND hWnd,
	LPARAM lParam
	)
{
	NMLISTVIEW *nmListView = (NMLISTVIEW *) lParam;

	switch (nmListView->hdr.code)
	{
	case LVN_KEYDOWN:
		UpdateUI();
		switch (((LV_KEYDOWN *) nmListView)->wVKey)
		{
		case VK_DELETE:
			{
				TCHAR szBuf[] = TEXT("Not implemented yet!");
				TCHAR szCaption[MAX_STRING_RES_LENGTH];

				LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));
				MessageBox(hWnd, szBuf, szCaption, MB_ICONWARNING | MB_OK);
			}
			break;

		case VK_RETURN:
			{
				TCHAR szBuf[] = TEXT("Not implemented yet!");
				TCHAR szCaption[MAX_STRING_RES_LENGTH];

				LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));
				MessageBox(hWnd, szBuf, szCaption, MB_ICONWARNING | MB_OK);
			}
			break;
		}
		break;

	case NM_CLICK:
		UpdateUI();
		break;
	}
}

/*
 *  FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
 *
 *  PURPOSE:  Processes messages for the main window.
 *
 *  WM_COMMAND	- process the application menu
 *  WM_PAINT	- Paint the main window
 *  WM_DESTROY	- post a quit message and return
 */
LRESULT CALLBACK
MainWndProc(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
		case WM_CREATE:
			CreateChildrenControls(hWnd);
			break;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDC_TB_NEW:
				case IDM_NEW_CONTACT:
					ShowContactPropertiesDialog(hWnd);
					break;

				case IDC_TB_PROPERTIES:
				case IDM_PROPERTIES:
					{
						TCHAR szBuf[] = TEXT("Not implemented yet!");
						TCHAR szCaption[MAX_STRING_RES_LENGTH];

						LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));
						MessageBox(hWnd, szBuf, szCaption, MB_ICONWARNING | MB_OK);
					}
					break;

				case IDC_TB_DELETE:
				case IDM_DELETE:
					{
						int numSelected;
						TCHAR szBuf[MAX_STRING_RES_LENGTH];
						TCHAR szCaption[MAX_STRING_RES_LENGTH];

						numSelected = ListView_GetSelectedCount(g_hwndListView);
						if (numSelected < 1)
						{
							break;
						}

						LoadString(g_hInst, IDS_CONFIRM_DELETE_ITEM_MSG, szBuf, ARRAYSIZE(szBuf));
						LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));

						if (MessageBox(hWnd, szBuf, szCaption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) == IDYES)
						{
							int iSelIndex;

							iSelIndex = ListView_GetNextItem(g_hwndListView, -1, LVNI_SELECTED);
							while (iSelIndex != -1)
							{
								ListView_DeleteItem(g_hwndListView, iSelIndex);

								iSelIndex = ListView_GetNextItem(g_hwndListView, -1, LVNI_SELECTED);
							}
						}
						SetFocus(g_hwndListView);
						UpdateUI();
					}
					break;

				case IDM_EXIT:
					SendMessage(hWnd, WM_CLOSE, 0, 0L);
					break;

				case IDM_SELECTALL:
					SelectAllItems();
					break;

				case IDM_STATUSBAR:
					if (IsWindowVisible(g_hwndStatusBar))
					{
						CheckMenuItem(GetMenu(hWnd), IDM_STATUSBAR, MF_BYCOMMAND | MF_UNCHECKED);
						ShowWindow(g_hwndStatusBar, SW_HIDE);
					}
					else
					{
						CheckMenuItem(GetMenu(hWnd), IDM_STATUSBAR, MF_BYCOMMAND | MF_CHECKED);
						ShowWindow(g_hwndStatusBar, SW_NORMAL);
					}
					AdjustChildrenControls(hWnd);
					break;

				case IDM_HELP_CHM:
					{
						TCHAR szExeFullPath[MAX_PATH] = _T("");
						LPTSTR pszProcessName;

						/* Construct the process' working directory */
						GetModuleFileName(NULL, szExeFullPath, MAX_PATH);
						pszProcessName = _tcsrchr(szExeFullPath, _T('\\')) + 1; /* The +1 removes the backslash */
						*pszProcessName = _T('\0'); /* Erase the filename.exe from szExeFullPath */
						_tcscat(szExeFullPath, _T("tribe.chm"));

						HtmlHelp(GetDesktopWindow(), szExeFullPath, HH_DISPLAY_TOPIC, 0);
					}
					break;

				case IDM_ABOUT:
				   DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutDlgProc);
				   break;
			}
			break;


		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc;

				/* TODO: Why do we need this code to make the listview repaint? */
				hdc = BeginPaint(hWnd, &ps);
				EndPaint(hWnd, &ps);
			}
			break;


		case WM_SIZE:
			AdjustChildrenControls(hWnd);
			break;


		case WM_CLOSE:
			DestroyWindow(hWnd);
			break;


		case WM_DESTROY:
			PostQuitMessage(0);
			break;


		case WM_NOTIFY:
			switch (wParam)
			{
			case IDC_LISTVIEW:
				HandleListViewNotifications(hWnd, lParam);
				break;
			}
			break;


		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
   }

   return 0;
}

/*
 * Mesage handler for about box.
 */
LRESULT CALLBACK
AboutDlgProc(
	HWND hDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				char szFile[MAX_PATH];
				DWORD dwBogus;
				DWORD dwVerInfoSize;
				LPVOID lpInfo;
				UINT uLen = 0;
				LPSTR pszVersion = NULL;
				LPSTR pszCopyright = NULL;

				GetModuleFileName(NULL, szFile, sizeof(szFile));

				dwVerInfoSize = GetFileVersionInfoSize(szFile, &dwBogus);
				if (dwVerInfoSize)
				{
					lpInfo = LocalAlloc(LMEM_ZEROINIT, dwVerInfoSize);
					if (lpInfo)
					{
						if (GetFileVersionInfoA(szFile, dwBogus, dwVerInfoSize, lpInfo))
						{
							if (VerQueryValueA(lpInfo, "\\StringFileInfo\\040904b0\\FileVersion", (LPVOID *)&pszVersion, &uLen) && uLen)
							{
								SetDlgItemText(hDlg, IDC_VERSION_NUMBER_LABEL, pszVersion);
							}
							if (VerQueryValueA(lpInfo, "\\StringFileInfo\\040904b0\\LegalCopyright", (LPVOID *)&pszCopyright, &uLen) && uLen)
							{
								SetDlgItemText(hDlg, IDC_COPYRIGHT_LABEL, pszCopyright);
							}
						}
					}
				}
			}
			return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}

    return FALSE;
}
