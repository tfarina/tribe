#include "ab.h"

#include <stdio.h>

#include "sqlite3.h"

static sqlite3 *hdb = NULL;  /* SQLite db handle */

HRESULT
ABCreateContact(
	CONTACT **ppContact
	)
{
	CONTACT *pContact = NULL;
	HRESULT hr = S_OK;

	/* Allocate space for the CONTACT structure
	 */
	pContact = LocalAlloc(LMEM_ZEROINIT, sizeof(CONTACT));
	if (NULL == pContact)
	{
		hr = E_OUTOFMEMORY;
		goto err;
	}

	/* Zero init the object
	 */
	ZeroMemory(pContact, sizeof(CONTACT));

	pContact->szFirstName = NULL;
	pContact->szLastName = NULL;
	pContact->szEmail = NULL;

	*ppContact = pContact;

	return S_OK;

err:
	LocalFree(pContact);
	pContact = NULL;

	return hr;
}

int
ABInitialize(
	void
	)
{
	char const dbname[] = "abdb.sqlite3";
	char const create_sql[] =
		"CREATE TABLE IF NOT EXISTS contacts ("
		"  id INTEGER PRIMARY KEY,"
		"  fname TEXT,"
		"  lname TEXT,"
		"  email TEXT"
		");";
	int rc;
	char *err_msg = NULL;

	rc = sqlite3_open(dbname, &hdb);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Failed to open the SQLite database at %s: %s\n", dbname,
			sqlite3_errmsg(hdb));
		sqlite3_close(hdb);
		return -1;
	}

    rc = sqlite3_exec(hdb, create_sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_exec failed: %s\n", err_msg);
		sqlite3_free(err_msg);
		sqlite3_close(hdb);
		return -1;
	}

	return 0;
}

HRESULT
ABEnumContacts(
	ULONG *pnCount,
	CONTACT **ppContacts
	)
{
	ULONG cContacts;
	CONTACT aContacts[] =
	{
		{ 0, "John", "Doe", "john_doe@mail.com" },
		{ 1, "Jane", "Doe", "jane_doe@mail.com" },
		{ 2, "John", "Smith", "john_smith@mail.com" },
	};
	LPCONTACT lpContactList;
	ULONG index;

	if (NULL == pnCount || NULL == ppContacts)
	{
		return E_INVALIDARG;
	}

	cContacts = ARRAYSIZE(aContacts);

	lpContactList = LocalAlloc(LMEM_ZEROINIT, cContacts * sizeof(CONTACT));
	if (NULL == lpContactList)
	{
		return E_OUTOFMEMORY;
	}

	for (index = 0; index < cContacts; index++)
	{
		ULONG ulLen;

		lpContactList[index].id = aContacts[index].id;

		ulLen = lstrlen(aContacts[index].szFirstName) + 1;
		lpContactList[index].szFirstName = malloc(ulLen * sizeof(TCHAR));
		if (NULL != lpContactList[index].szFirstName)
		{
			lstrcpy(lpContactList[index].szFirstName, aContacts[index].szFirstName);
		}
		else
		{
			return E_OUTOFMEMORY;
		}

		ulLen = lstrlen(aContacts[index].szLastName) + 1;
		lpContactList[index].szLastName = malloc(ulLen * sizeof(TCHAR));
		if (NULL != lpContactList[index].szLastName)
		{
			lstrcpy(lpContactList[index].szLastName, aContacts[index].szLastName);
		}
		else
		{
			return E_OUTOFMEMORY;
		}

		ulLen = lstrlen(aContacts[index].szEmail) + 1;
		lpContactList[index].szEmail = malloc(ulLen * sizeof(TCHAR));
		if (NULL != lpContactList[index].szEmail)
		{
			lstrcpy(lpContactList[index].szEmail, aContacts[index].szEmail);
		}
		else
		{
			return E_OUTOFMEMORY;
		}
	}

	*pnCount = cContacts;
	*ppContacts = lpContactList;

	return S_OK;
}
