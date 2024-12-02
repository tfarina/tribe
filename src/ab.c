#include "ab.h"

#include <stdio.h>

#include "third_party/sqlite/sqlite3.h"

static sqlite3 *hdb = NULL;  /* SQLite db handle */

static
LPSTR
XStrDup(
	LPCSTR psz
	)
{
	LPSTR pszRet;

	if (!psz)
		return NULL;

	pszRet = LocalAlloc(LPTR, (lstrlen(psz) + 1) * sizeof(*pszRet));
	if (pszRet)
	{
		lstrcpy(pszRet, psz);
	}

	return pszRet;
}

static
HRESULT
_DbGetContactsRowCount(
	int *pnRowCount
	)
{
	int nRet;
	HRESULT hr = S_OK;
	char const szSql[] = "SELECT COUNT(*) FROM contacts";
	sqlite3_stmt *pStmt = NULL;

	if (!pnRowCount)
	{
		return E_INVALIDARG;
	}

	nRet = sqlite3_prepare_v2(hdb, szSql, -1, &pStmt, NULL);
	if (nRet != SQLITE_OK)
	{
		fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
				sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto out;
	}

	nRet = sqlite3_step(pStmt);
	if (nRet != SQLITE_ROW)
	{
		fprintf(stderr, "ERROR: sqlite3_step failed: %s\n",
				sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto out;
	}

	*pnRowCount = sqlite3_column_int(pStmt, 0);

out:
	sqlite3_finalize(pStmt);

	return hr;
}

static
HRESULT
_DbInsertContact(
	CONTACT *pContact
	)
{
	int rc = 0;
	HRESULT hr = S_OK;
	char const szInsertSql[] =
		"INSERT INTO contacts (fname, lname, email) VALUES (?, ?, ?)";
	sqlite3_stmt *pInsertStmt = NULL;

	rc = sqlite3_prepare_v2(hdb, szInsertSql, -1, &pInsertStmt, NULL);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_prepare_v2 failed: %s\n",
			sqlite3_errmsg(hdb));
		return E_FAIL;
	}

	rc = sqlite3_bind_text(pInsertStmt, 1, pContact->szFirstName, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_bind_text failed: %s\n",
			sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto out;
	}

	rc = sqlite3_bind_text(pInsertStmt, 2, pContact->szLastName, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_bind_text failed: %s\n",
			sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto out;
	}

	rc = sqlite3_bind_text(pInsertStmt, 3, pContact->szEmail, -1, SQLITE_STATIC);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_bind_text failed: %s\n",
			sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto out;
	}

	rc = sqlite3_step(pInsertStmt);
	if (rc != SQLITE_DONE)
	{
		fprintf(stderr, "sqlite3_step failed: %s\n",
			sqlite3_errmsg(hdb));
		hr = E_FAIL;
	}

out:
	rc = sqlite3_finalize(pInsertStmt);
	pInsertStmt = NULL;
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "sqlite3_finalize failed: %s\n",
			sqlite3_errmsg(hdb));
		hr = E_FAIL;
	}

	return hr;
}

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

void
ABDestroyContact(
	CONTACT *pContact
	)
{
	if (!pContact)
	{
		return;
	}

	LocalFree(pContact->szFirstName);
	pContact->szFirstName = NULL;
	LocalFree(pContact->szLastName);
	pContact->szLastName = NULL;
	LocalFree(pContact->szEmail);
	pContact->szEmail = NULL;

	LocalFree(pContact);
	pContact = NULL;
}

HRESULT
ABContactSetFirstName(
	CONTACT *pContact,
	TCHAR const *szFirstName
	)
{
	HRESULT hr = S_OK;
	SIZE_T nLen = 0;

	nLen = lstrlen(szFirstName) + 1;
	pContact->szFirstName = LocalAlloc(LMEM_ZEROINIT, nLen * sizeof(TCHAR));
	if (!pContact->szFirstName)
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}
	CopyMemory(pContact->szFirstName, szFirstName, nLen);

out:
	return hr;
}

HRESULT
ABContactSetLastName(
	CONTACT *pContact,
	TCHAR const *szLastName
	)
{
	HRESULT hr = S_OK;
	SIZE_T nLen = 0;

	nLen = lstrlen(szLastName) + 1;
	pContact->szLastName = LocalAlloc(LMEM_ZEROINIT, nLen * sizeof(TCHAR));
	if (!pContact->szLastName)
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}
	CopyMemory(pContact->szLastName, szLastName, nLen);

out:
	return hr;
}

HRESULT
ABContactSetEmail(
	CONTACT *pContact,
	TCHAR const *szEmail
	)
{
	HRESULT hr = S_OK;
	SIZE_T nLen = 0;

	nLen = lstrlen(szEmail) + 1;
	pContact->szEmail = LocalAlloc(LMEM_ZEROINIT, nLen * sizeof(TCHAR));
	if (!pContact->szEmail)
	{
		hr = E_OUTOFMEMORY;
		goto out;
	}
	CopyMemory(pContact->szEmail, szEmail, nLen);

out:
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
	ULONG *pulcContacts,
	CONTACT **ppContacts
	)
{
	HRESULT hr = S_OK;
	int nRet;
	int nRowCount = 0;
	ULONG cContacts;
	CONTACT aContacts[] =
	{
		{ -4, "John", "Doe", "john_doe@mail.com" },
		{ -3, "Jane", "Doe", "jane_doe@mail.com" },
		{ -2, "John", "Smith", "john_smith@mail.com" },
	};
	LPCONTACT lprgContacts;
	char const szSql[] = "SELECT * FROM contacts";
	sqlite3_stmt *pStmt = NULL;
	ULONG index;
	int nRows = 0;

	if (!pulcContacts || !ppContacts)
	{
		return E_INVALIDARG;
	}

	hr = _DbGetContactsRowCount(&nRowCount);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	cContacts = ARRAYSIZE(aContacts);

	lprgContacts = LocalAlloc(LMEM_ZEROINIT, (cContacts + nRowCount) * sizeof(CONTACT));
	if (!lprgContacts)
	{
		hr = E_OUTOFMEMORY;
		goto err;
	}

	for (index = 0; index < cContacts; index++)
	{
		lprgContacts[index].id = aContacts[index].id;

		lprgContacts[index].szFirstName = XStrDup(aContacts[index].szFirstName);
		if (!lprgContacts[index].szFirstName)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}

		lprgContacts[index].szLastName = XStrDup(aContacts[index].szLastName);
		if (!lprgContacts[index].szLastName)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}

		lprgContacts[index].szEmail = XStrDup(aContacts[index].szEmail);
		if (!lprgContacts[index].szEmail)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}
	}

	nRet = sqlite3_prepare_v2(hdb, szSql, -1, &pStmt, NULL);
	if (nRet != SQLITE_OK)
	{
		fprintf(stderr, "ERROR: sqlite3_prepare_v2 failed: %s\n",
				sqlite3_errmsg(hdb));
		hr = E_FAIL;
		goto err;
	}

	for (index = 0; index < nRowCount; index++)
	{
		char const *psz = NULL;
		nRet = sqlite3_step(pStmt);
		if (nRet != SQLITE_ROW)
			break;

		lprgContacts[index + cContacts].id = sqlite3_column_int(pStmt, 0);

		psz = (char const *)sqlite3_column_text(pStmt, 1);
		lprgContacts[index + cContacts].szFirstName = XStrDup(psz);
		if (!lprgContacts[index + cContacts].szFirstName)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}

		psz = (char const *)sqlite3_column_text(pStmt, 2);
		lprgContacts[index + cContacts].szLastName = XStrDup(psz);
		if (!lprgContacts[index + cContacts].szLastName)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}

		psz = (char const *)sqlite3_column_text(pStmt, 3);
		lprgContacts[index + cContacts].szEmail = XStrDup(psz);
		if (!lprgContacts[index + cContacts].szEmail)
		{
			hr = E_OUTOFMEMORY;
			goto err;
		}

		nRows++;
	}

	*pulcContacts = cContacts + nRows;
	*ppContacts = lprgContacts;

err:
	sqlite3_finalize(pStmt);
	if (FAILED(hr))
	{
		LocalFree(lprgContacts);
		lprgContacts = NULL;
	}

	return hr;
}

HRESULT
ABAddContactV2(
	CONTACT *pContact
	)
{
	return _DbInsertContact(pContact);
}