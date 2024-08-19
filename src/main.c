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

#include "ab.h"
#include "resource.h"
#include "alpm_list.h"
#include "arraysize.h"

#define MAX_COLUMN_TEXT_LENGTH 64
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

enum
{
	COL_FIRST_NAME = 0,
	COL_LAST_NAME,
	COL_EMAIL,
	N_COLUMNS
};

typedef struct _CONTACT_ITEM
{
	int id;
	TCHAR szFirstName[MAX_COLUMN_TEXT_LENGTH];
	TCHAR szLastName[MAX_COLUMN_TEXT_LENGTH];
	TCHAR szEmail[MAX_COLUMN_TEXT_LENGTH];
} CONTACT_ITEM, *LPCONTACT_ITEM;

HINSTANCE g_hInst;     /* Handle to this application instance */
HWND g_hwndMain;       /* Handle to this application window */
HWND g_hwndStatusBar;  /* Handle to status bar window */
HWND g_hwndListView;   /* Handle to list view window */
HWND g_hwndToolbar;    /* Handle to toolbar window*/

LPTSTR g_szClassName = TEXT("TribeAB");  /* Window class name */

LONG g_hStatus;

static alpm_list_t *contactList = NULL;

typedef struct _PROPDLGINFO
{
	int nID;
	CONTACT *pContact;
} PROPDLGINFO;

BOOL
RegisterMainWindowClass(
	HINSTANCE
	);
HWND
CreateMainWindow(
	HINSTANCE,
	int
	);
void
CreateChildControls(
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

	if (!RegisterMainWindowClass(hInstance))
	{
		return FALSE;
	}

	g_hwndMain = CreateMainWindow(hInstance, nCmdShow);
	if (NULL == g_hwndMain)
	{
		return FALSE;
	}

	rc = ABInitialize();
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
 *  FUNCTION: RegisterMainWindowClass()
 *
 *  PURPOSE: Registers the main window class with the system.
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
RegisterMainWindowClass(
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
HWND
CreateMainWindow(
	HINSTANCE hInstance,
	int nCmdShow
	)
{
	HWND hwndMain;
	TCHAR szTitle[MAX_STRING_RES_LENGTH];  /* Title bar text */
	HMENU hMenu;

	LoadString(hInstance, IDS_APP_NAME, szTitle, ARRAYSIZE(szTitle));

	hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENUBAR));

	/* Create the main window for this application instance. */
	hwndMain = CreateWindowEx(
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

	if (!hwndMain)
	{
		return NULL;
	}

	ShowWindow(hwndMain, nCmdShow);
	UpdateWindow(hwndMain);

	return hwndMain;
}

HRESULT
LoadContacts(
	void
	)
{
	HRESULT hr = S_OK;
	ULONG cContacts = 0;
	LPCONTACT lpContacts = NULL;
	ULONG index = 0;
	LPCONTACT_ITEM lpContact = NULL;

	hr = ABEnumContacts(&cContacts, &lpContacts);
	if (FAILED(hr))
		goto exit;

	for (index = 0; index < cContacts; index++)
	{
		lpContact = LocalAlloc(LMEM_ZEROINIT, sizeof(CONTACT_ITEM));
		if (!lpContact)
		{
			hr = E_OUTOFMEMORY;
			goto exit;
		}
		lpContact->id = lpContacts[index].id;
		lstrcpy(lpContact->szFirstName, lpContacts[index].szFirstName);
		lstrcpy(lpContact->szLastName, lpContacts[index].szLastName);
		lstrcpy(lpContact->szEmail, lpContacts[index].szEmail);

		contactList = alpm_list_add(contactList, lpContact);
	}

exit:
	if (lpContacts)
	{
		LocalFree(lpContacts);
		lpContacts = NULL;
	}

	return hr;
}

static
void
SelectListViewItem(
	int index
	)
{
	ListView_SetItemState(
		g_hwndListView,
		index,
		LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);

	ListView_EnsureVisible(g_hwndListView, index, FALSE);
}

void
PopulateListView(
	void
	)
{
	alpm_list_t *item;
	LPCONTACT_ITEM contact;
	LVITEM lvI = {0};
	int index = 0;

	lvI.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
	lvI.iItem = 0;

	for (item = contactList; item; item = alpm_list_next(item))
	{
		contact = (LPCONTACT_ITEM) item->data;
		lvI.iSubItem = COL_FIRST_NAME;
		lvI.lParam = (LPARAM) contact;
		lvI.pszText = contact->szFirstName;

		index = ListView_InsertItem(g_hwndListView, &lvI);
		if (index != -1)
		{
			ListView_SetItemText(g_hwndListView, index, COL_LAST_NAME, contact->szLastName);
			ListView_SetItemText(g_hwndListView, index, COL_EMAIL, contact->szEmail);
		}
		lvI.iItem++;
	}

	SelectListViewItem(0);
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
		0,                /* extended window style */
		TOOLBARCLASSNAME, /* window class name */
		(LPTSTR)NULL,     /* window name */
		WS_CHILD | WS_VISIBLE | TBSTYLE_WRAPABLE,  /* window style */
		0,                /* initial x position */
		0,                /* initial y position */
		0,                /* width */
		0,                /* height */
		hWndParent,       /* handle to parent window */
		NULL,             /* handle to menu */
		g_hInst,          /* handle to program instance */
		NULL);            /* creation parameters */

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
		0,                /* extended window style */
		STATUSCLASSNAME,  /* window class name */
		(LPTSTR) NULL,    /* window name */
		WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,  /* window style */
		0,                /* initial x position */
		0,                /* initial y position */
		0,                /* width */
		0,                /* height */
		hWndParent,       /* handle to parent window */
		(HMENU) IDC_STATUSBAR, /* handle to menu */
		g_hInst,          /* handle to program instance */
		NULL);            /* creation parameters */

	GetClientRect(g_hwndStatusBar, &statusBarRect);

	/* Calculate the height of statusbar. */
	g_hStatus = statusBarRect.bottom - statusBarRect.top;
}

void
CreateChildControls(
	HWND hWndParent
	)
{
	CreateToolbar(hWndParent);

	CreateStatusBar(hWndParent);

	CreateListView(hWndParent);
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
	PROPDLGINFO *ppdi = (PROPDLGINFO *)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));

	switch (uMsg)
	{
	case WM_INITDIALOG:
		ppdi = (PROPDLGINFO *)((LPPROPSHEETPAGE)lParam)->lParam;
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)ppdi);
		break;

	case WM_DESTROY:
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, (LONG_PTR)NULL);
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_KILLACTIVE:
			{
				TCHAR szTemp[MAX_EDIT_LENGTH];
				szTemp[0] = '\0';

				GetDlgItemText(hWndDlg, IDC_PROPPAGE_NAME_EDIT_FIRSTNAME, szTemp, ARRAYSIZE(szTemp));
				if (lstrlen(szTemp))
				{
					ABContactSetFirstName(ppdi->pContact, szTemp);
				}
				GetDlgItemText(hWndDlg, IDC_PROPPAGE_NAME_EDIT_LASTNAME, szTemp, ARRAYSIZE(szTemp));
				if (lstrlen(szTemp))
				{
					ABContactSetLastName(ppdi->pContact, szTemp);
				}
				GetDlgItemText(hWndDlg, IDC_PROPPAGE_NAME_EDIT_EMAIL, szTemp, ARRAYSIZE(szTemp));
				if (lstrlen(szTemp))
				{
					ABContactSetEmail(ppdi->pContact, szTemp);
				}
			}
			break;

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

				ABAddContactV2(ppdi->pContact);
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

static
BOOL
CreateContactPropertiesDialog(
	HWND hWnd,
	PROPDLGINFO *ppdi
	)
{
	PROPSHEETPAGE propSheetPage[1];
	PROPSHEETHEADER propSheetHeader;
	INT_PTR nRetVal = 0;

	propSheetPage[0].dwSize = sizeof(PROPSHEETPAGE);
	propSheetPage[0].dwFlags = PSP_USETITLE;
	propSheetPage[0].hInstance = g_hInst;
	propSheetPage[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_NAME);
	propSheetPage[0].pszIcon = NULL;
	propSheetPage[0].pszTitle = TEXT("Name");
	propSheetPage[0].pfnDlgProc = fnNamePageProc;
	propSheetPage[0].lParam = (LPARAM)ppdi;

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

	nRetVal = PropertySheet(&propSheetHeader);

	return nRetVal >= 0;
}

static void
ShowContactPropertiesDialog(
	HWND hWnd
	)
{
	int numSelected;
	int iSelIndex;
	TCHAR szBuf[] = TEXT("Not implemented yet!");
	TCHAR szCaption[MAX_STRING_RES_LENGTH];

	numSelected = ListView_GetSelectedCount(g_hwndListView);
	if (numSelected == 1)
	{
		iSelIndex = ListView_GetNextItem(g_hwndListView, -1, LVNI_SELECTED);
		if (iSelIndex != -1)
		{
			LPCONTACT_ITEM pItem = NULL;
			LV_ITEM lvi;

			lvi.mask = LVIF_PARAM;
			lvi.iItem = iSelIndex;
			lvi.iSubItem = 0;
			lvi.lParam = 0;

			if (ListView_GetItem(g_hwndListView, &lvi))
				pItem = (LPCONTACT_ITEM) lvi.lParam;

			if (pItem)
			{
				/*
				 * TODO: Actually show the contact properties dialog for this
				 * contact item.
				 */
			}
		}
	}

	LoadString(g_hInst, IDS_APP_NAME, szCaption, ARRAYSIZE(szCaption));
	MessageBox(hWnd, szBuf, szCaption, MB_ICONWARNING | MB_OK);
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
				SendMessage(hWnd, WM_COMMAND, IDM_DELETE, 0);
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

static
void
MainWindow_OnPaint(
	HWND hWnd
	)
{
	PAINTSTRUCT ps;
	HDC hdc;

	/* TODO: Why do we need this code to make the listview repaint? */
	hdc = BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
}

static
void
MainWindow_OnDestroy(
	HWND hwnd
	)
{
	PostQuitMessage(0);
}

static
void
ShowPropertiesDialog(
	HWND hwnd
	)
{
	HRESULT hr = S_OK;
	PROPDLGINFO pdi = {0};

	pdi.nID = -1;

	hr = ABCreateContact(&pdi.pContact);
	if (FAILED(hr))
	{
		goto err;
	}

	if (!CreateContactPropertiesDialog(hwnd, &pdi))
	{
		goto err;
	}

err:
	ABDestroyContact(pdi.pContact);
	return;
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
			{
				CreateChildControls(hWnd);
				LoadContacts();
				PopulateListView();
				SetFocus(g_hwndListView);
				UpdateUI();
			}
			break;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDC_TB_NEW:
				case IDM_NEW_CONTACT:
					ShowPropertiesDialog(hWnd);
					break;

				case IDC_TB_PROPERTIES:
				case IDM_PROPERTIES:
					ShowContactPropertiesDialog(hWnd);
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

		HANDLE_MSG(hWnd, WM_PAINT, MainWindow_OnPaint);

		case WM_SIZE:
			AdjustChildrenControls(hWnd);
			break;

		case WM_CLOSE:
			ListView_DeleteAllItems(g_hwndListView);
			DestroyWindow(hWnd);
			break;

		HANDLE_MSG(hWnd, WM_DESTROY, MainWindow_OnDestroy);

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
