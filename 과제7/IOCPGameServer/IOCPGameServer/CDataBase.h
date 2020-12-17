#pragma once
class CClient;
class CDataBase
{
public:
	void Insert_DB(CClient& c);
	void Update_DB(CClient& c);
	void Import_DB();
	void HandleDiagnosticRecord(SQLSMALLINT hType, RETCODE RetCode);
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt = 0;
	SQLRETURN retcode;
	SQLWCHAR szName[NAME_LEN];
	SQLINTEGER ID, LEVEL;
	SQLLEN cbName = 0, cbID = 0, cbLEVEL = 0;

	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];
};

