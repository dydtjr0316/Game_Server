#include "stdafx.h"
#include "CDataBase.h"
#include "CClient.h"

void CDataBase::HandleDiagnosticRecord(SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType, hstmt, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated.. 
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
	//while (true);
}

void CDataBase::Insert_DB(CClient& c)
{
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2020_fall", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					SQLWCHAR query[1024];
					char name[30];

					// 여기
					string temp = c.m_name;
					const char* ttemp = temp.c_str();
					strcpy_s(name, c.m_name);

					cout << name << endl;

					wsprintf(query, L"EXEC store_data %d, %s, %d, %d, %d, %d, %d",
						c.m_id, ttemp, (int)c.m_slevel, (int)c.x, (int)c.y, (int)c.m_shp, c.m_iexp);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query, SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						printf("DB 저장 성공, ID : %d\n", c.m_id);
					}
					else {
						HandleDiagnosticRecord(SQL_HANDLE_STMT, retcode);
					}
					{
						//retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC select_highlevel 50", SQL_NTS);
						//if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

						//	// Bind columns 1, 2, and 3  
						//	retcode = SQLBindCol(hstmt, 1, SQL_C_LONG, &ID, 100, &cbID);
						//	retcode = SQLBindCol(hstmt, 2, SQL_C_WCHAR, szName, NAME_LEN, &cbName);
						//	retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &LEVEL, 100, &cbLEVEL);

						//	// Fetch and print each row of data. On an error, display a message and exit.  
						//	/*for (int i = 0; ; i++) {
						//		retcode = SQLFetch(hstmt);
						//		if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
						//			show_error();
						//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
						//		{
						//			wcout << i + 1 << " : " << ID << ", " << szName << ", " << LEVEL << endl;
						//		}
						//		else
						//			break;
						//	}*/
						//}
					}
					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}

					SQLDisconnect(hdbc);
				}

				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

void CDataBase::Update_DB(CClient& c)
{
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2020_fall", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

					SQLWCHAR query[1024];
					char name[30];

					// 여기
					string temp = c.m_name;
					const char* ttemp = temp.c_str();
					strcpy_s(name, c.m_name);

					cout << name << endl;

					wsprintf(query, L"EXEC UPDATE_DB %d, %s, %d, %d, %d, %d, %d",
						c.m_id, ttemp, (int)c.m_slevel, (int)c.x, (int)c.y, (int)c.m_shp, c.m_iexp);
					retcode = SQLExecDirect(hstmt, (SQLWCHAR*)query, SQL_NTS);
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						printf("DB 저장 성공, ID : %d\n", c.m_id);
					}
					else {
						HandleDiagnosticRecord(SQL_HANDLE_STMT, retcode);
					}
					// Process data  
					if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
						SQLCancel(hstmt);
						SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
					}
					SQLDisconnect(hdbc);
				}
				SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
			}
		}
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
	}
}

void CDataBase::Import_DB()
{
}
