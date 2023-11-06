#ifndef _DB_H_INCLUDED_
#define _DB_H_INCLUDED_ 1

#include <windows.h>
#include <tchar.h>

typedef struct _CONTACTROW
{
	int id;
	TCHAR *szFirstName;
	TCHAR *szLastName;
	TCHAR *szEmail;
} CONTACTROW, *LPCONTACTROW;

int db_init(void);

HRESULT
GetContactList(
	ULONG *pnCount,
	CONTACTROW **ppContacts
	);

#endif  /* !defined(_DB_H_INCLUDED_) */