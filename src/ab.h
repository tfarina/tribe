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

HRESULT
ABCreateContact(
	CONTACT **ppContact
	);

HRESULT
ABContactSetFirstName(
	CONTACT *pContact,
	TCHAR const *szFirstName
	);

HRESULT
ABContactSetLastName(
	CONTACT *pContact,
	TCHAR const *szLastName
	);

HRESULT
ABContactSetEmail(
	CONTACT *pContact,
	TCHAR const *szEmail
	);

int
ABInitialize(
	void
	);

HRESULT
ABEnumContacts(
	ULONG *pnCount,
	CONTACT **ppContacts
	);

HRESULT
ABAddContactV2(
	CONTACT *pContact
	);

#endif  /* !defined(_AB_H_INCLUDED_) */
