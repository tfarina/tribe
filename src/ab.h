#ifndef _AB_H_INCLUDED_
#define _AB_H_INCLUDED_ 1

#include <windows.h>
#include <tchar.h>

typedef struct _CONTACT
{
	int id;
	TCHAR *szFirstName;
	TCHAR *szLastName;
	TCHAR *szEmail;
} CONTACT, *LPCONTACT;

int ab_init(void);

HRESULT
GetContactList(
	ULONG *pnCount,
	CONTACT **ppContacts
	);

#endif  /* !defined(_AB_H_INCLUDED_) */