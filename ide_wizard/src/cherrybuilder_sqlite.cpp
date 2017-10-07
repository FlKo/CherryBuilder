/* ===============================================================================
 * CherryBuilder - The Productivity Extension for C++Builder®
 * ===============================================================================
 * MIT License
 *
 * Copyright (c) 2017 Florian Koch <flko@mail.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * ===============================================================================
 */

#include <vcl.h>
#pragma hdrstop

#include "cherrybuilder_sqlite.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace SQLite
{

//===========================================================================
// TDatabase
//===========================================================================
TDatabase::TDatabase(TJournalMode JournalMode)
    : 	FIsOpen(false),
		FJournalMode(JournalMode)
{
}
//---------------------------------------------------------------------------

TDatabase::TDatabase(const String& FileName, TJournalMode JournalMode)
    : 	FIsOpen(false),
		FJournalMode(JournalMode)
{	
    Open(FileName);
}
//---------------------------------------------------------------------------

TDatabase::~TDatabase()
{
    if (FIsOpen)
        Close();
}
//---------------------------------------------------------------------------

sqlite3* TDatabase::GetHandle()
{
    return FSQLiteDB;
}
//---------------------------------------------------------------------------

void TDatabase::Open(const String& FileName)
{
    if (!FIsOpen)
    {
        FFileName = FileName;

        // Open or create the database (if non-existent)
        int ResultCode = sqlite3_open16(FFileName.w_str(), &FSQLiteDB);
                
        if (ResultCode != SQLITE_OK)
            throw Exception(
                L"sqlite3 error: " + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FSQLiteDB)))
                );            

        FIsOpen = true;
		
		SetJournalMode(FJournalMode);		
    }
    else
    {
        throw Exception(L"sqlite3 error: database is already open");
    }
}
//---------------------------------------------------------------------------

void TDatabase::Close()
{
    sqlite3_close(FSQLiteDB);

    FIsOpen = false;
}
//---------------------------------------------------------------------------

void TDatabase::BeginTransaction()
{
    if (FIsOpen)
    {
        char *ErrMsg;

        sqlite3_exec(FSQLiteDB, "BEGIN", 0, 0, &ErrMsg);

        if (ErrMsg)
        {
            String ErrorText = ErrMsg;
            sqlite3_free(ErrMsg);
           
            throw Exception(L"sqlite3 error: " + ErrorText);
        }
    }
    else
    {
        throw Exception(L"sqlite3 error: no open database");
    }
}
//---------------------------------------------------------------------------

void TDatabase::RollbackTransaction()
{
    if (FIsOpen)
    {
        char *ErrMsg;

        sqlite3_exec(FSQLiteDB, "ROLLBACK", 0, 0, &ErrMsg);

        if (ErrMsg)
        {
            String ErrorText = ErrMsg;
            sqlite3_free(ErrMsg);
            
            throw Exception(L"sqlite3 error: " + ErrorText);
        }
    }
    else
    {  
        throw Exception(L"sqlite3 error: no open database");
    }
        
}
//---------------------------------------------------------------------------

void TDatabase::CommitTransaction()
{
    if (FIsOpen)
    {
        char *ErrMsg;

        sqlite3_exec(FSQLiteDB, "COMMIT", 0, 0, &ErrMsg);

        if (ErrMsg)
        {
            String ErrorText = ErrMsg;
            sqlite3_free(ErrMsg);
			
            throw Exception(L"sqlite3 error: " + ErrorText);
        }
    }
    else
    {  
        throw Exception(L"sqlite3 error: no open database");
    }
}
//---------------------------------------------------------------------------

bool TDatabase::IsOpen()
{
    return FIsOpen;
}
//---------------------------------------------------------------------------

__int64 TDatabase::GetLastInsertRowId()
{
    if (!FIsOpen)
        throw Exception(L"sqlite3 error: no open database");

    return sqlite3_last_insert_rowid(FSQLiteDB);
}
//---------------------------------------------------------------------------

void TDatabase::SetJournalMode(TJournalMode JournalMode)
{
	AnsiString JournalModeName = "";
	
	switch (JournalMode)
	{
		case jmDelete:
			JournalModeName = "DELETE";
		break;
		
		case jmTruncate:
			JournalModeName = "TRUNCATE";
		break;
		
		case jmPersist:
			JournalModeName = "PERSIST";
		break;
		
		case jmMemory:
			JournalModeName = "MEMORY";
		break;
		
		case jmWal:
			JournalModeName = "WAL";
		break;
		
		case jmOff:
			JournalModeName = "OFF";
		break;
	}
	
    if (FIsOpen)
    {
        char *ErrMsg;

        sqlite3_exec(FSQLiteDB, AnsiString("PRAGMA journal_mode=" + JournalModeName + ";").c_str(), 0, 0, &ErrMsg);

        if (ErrMsg)
        {
            String ErrorText = ErrMsg;
            sqlite3_free(ErrMsg);         
            throw Exception(L"sqlite3 error: " + ErrorText);
        }
    }
    else
    {
        throw Exception(L"sqlite3 error: no open database");
    }
}
//---------------------------------------------------------------------------

//===========================================================================
// TStatement
//===========================================================================

TStatement::TStatement(TDatabase& Database, const String& StatementText)
    :   FDatabase(Database),
        FStatementText(StatementText),
        FCompiledStatement(NULL)
{
    if (FDatabase.IsOpen())
    {
        Prepare();
    }
    else
    {
        throw Exception(L"slite3 error: no open database");
    }
}
//---------------------------------------------------------------------------

TStatement::~TStatement()
{
    if (FDatabase.IsOpen())
        sqlite3_finalize(FCompiledStatement);
}
//---------------------------------------------------------------------------

void TStatement::BindBlob(int ParamNo, const void* Val, int nBytes, bool IsStatic)
{
    int ResultCode = sqlite3_bind_blob(
                        FCompiledStatement,
                        ParamNo,
                        Val,
                        nBytes,
                        IsStatic ? SQLITE_STATIC : SQLITE_TRANSIENT
                        );

    if (ResultCode != SQLITE_OK)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

}
//---------------------------------------------------------------------------

void TStatement::BindDouble(int ParamNo, double Val)
{
    int ResultCode = sqlite3_bind_double(
                        FCompiledStatement,
                        ParamNo,
                        Val
                        );

    if (ResultCode != SQLITE_OK)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }
}
//---------------------------------------------------------------------------

void TStatement::BindInt(int ParamNo, int Val)
{
    int ResultCode = sqlite3_bind_int(
                        FCompiledStatement,
                        ParamNo,
                        Val
                        );

    if (ResultCode != SQLITE_OK)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );            
    }
}
//---------------------------------------------------------------------------

void TStatement::BindInt64(int ParamNo, __int64 Val)
{
    int ResultCode = sqlite3_bind_int64(
                        FCompiledStatement,
                        ParamNo,
                        Val
                        );

    if (ResultCode != SQLITE_OK)
    {  
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }
}
//---------------------------------------------------------------------------

void TStatement::BindString(int ParamNo, const String& Val, bool IsStatic)
{
    int ResultCode = sqlite3_bind_text16(
                        FCompiledStatement,
                        ParamNo,
                        Val.w_str(),
                        -1,
                        IsStatic ? SQLITE_STATIC : SQLITE_TRANSIENT
                        );

    if (ResultCode != SQLITE_OK)
    { 
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }
}
//---------------------------------------------------------------------------

void TStatement::BindNull(int ParamNo)
{
    int ResultCode = sqlite3_bind_null(
                        FCompiledStatement,
                        ParamNo
                        );

    if (ResultCode != SQLITE_OK)
    {  
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }
}
//---------------------------------------------------------------------------

int TStatement::GetParamCount()
{
    return sqlite3_bind_parameter_count(FCompiledStatement);
}
//---------------------------------------------------------------------------

int TStatement::GetColumnCount()
{
    return sqlite3_column_count(FCompiledStatement);
}
//---------------------------------------------------------------------------

int TStatement::ExecuteStep()
{
    int ResultCode = sqlite3_step(FCompiledStatement);

    switch (ResultCode)
    {
        case SQLITE_BUSY:
        case SQLITE_DONE:
        case SQLITE_ROW:
            return ResultCode;

        default:
            throw Exception(
                L"sqlite3 error: "
                + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
                );
    }
}
//---------------------------------------------------------------------------

int TStatement::Reset()
{
    int ResultCode = sqlite3_reset(FCompiledStatement);

    switch (ResultCode)
    {
        case SQLITE_OK:
        case SQLITE_DONE:
        case SQLITE_ROW:
            return ResultCode;
            
        default:
            throw Exception(
                L"sqlite3 error: "
                + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
                );
    }
}
//---------------------------------------------------------------------------

int TStatement::GetColumnType(int ColNo)
{
    int RetVal = sqlite3_column_type(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    // Possible values:
    // SQLITE_INTEGER
    // SQLITE_FLOAT
    // SQLITE_TEXT
    // SQLITE_BLOB
    // SQLITE_NULL

    return RetVal;
}
//---------------------------------------------------------------------------

const void* TStatement::GetColumnAsBlob(int ColNo, int& nBytes)
{
    const void* RetVal = sqlite3_column_blob(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    {  
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );          
    }

    nBytes = sqlite3_column_bytes(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    return RetVal;
}
//---------------------------------------------------------------------------

const void* TStatement::GetColumnAsBlob(const String& ColName, int& nBytes)
{
    return GetColumnAsBlob(GetColumnNoByName(ColName), nBytes);
}
//---------------------------------------------------------------------------

double TStatement::GetColumnAsDouble(int ColNo)
{
    double RetVal = sqlite3_column_double(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    { 
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    return RetVal;
}
//---------------------------------------------------------------------------

double TStatement::GetColumnAsDouble(const String& ColName)
{
    return GetColumnAsDouble(GetColumnNoByName(ColName));
}
//---------------------------------------------------------------------------

int TStatement::GetColumnAsInt(int ColNo)
{
    int RetVal = sqlite3_column_int(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    {
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    return RetVal;
}
//---------------------------------------------------------------------------

int TStatement::GetColumnAsInt(const String& ColName)
{
    return GetColumnAsInt(GetColumnNoByName(ColName));
}
//---------------------------------------------------------------------------

__int64 TStatement::GetColumnAsInt64(int ColNo)
{
    __int64 RetVal = sqlite3_column_int64(FCompiledStatement, ColNo);

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    {  
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    return RetVal;
}
//---------------------------------------------------------------------------

__int64 TStatement::GetColumnAsInt64(const String& ColName)
{
    return GetColumnAsInt64(GetColumnNoByName(ColName));
}
//---------------------------------------------------------------------------

const String TStatement::GetColumnAsString(int ColNo)
{
    String RetVal = static_cast<const wchar_t*>(sqlite3_column_text16(FCompiledStatement, ColNo));

    if (sqlite3_errcode(FDatabase.GetHandle()) == SQLITE_NOMEM)
    { 
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }

    return RetVal;
}
//---------------------------------------------------------------------------

const String TStatement::GetColumnAsString(const String& ColName)
{
    return GetColumnAsString(GetColumnNoByName(ColName));
}
//---------------------------------------------------------------------------

void TStatement::Prepare()
{

    int ResultCode = sqlite3_prepare16_v2(
                        FDatabase.GetHandle(),
                        FStatementText.w_str(),
                        -1,
                        &FCompiledStatement,
                        NULL
                        );

    if (ResultCode != SQLITE_OK)
    {   
        throw Exception(
            L"sqlite3 error: "
            + String(static_cast<const wchar_t*>(sqlite3_errmsg16(FDatabase.GetHandle())))
            );
    }
}
//---------------------------------------------------------------------------

int TStatement::GetColumnNoByName(const String& ColName)
{
    int CurColNo;
    int nCol = GetColumnCount();

    for (CurColNo = 0; CurColNo < nCol ; ++CurColNo)
    {
        String CurColName = static_cast<const wchar_t*>(
                                sqlite3_column_name16(FCompiledStatement, CurColNo)
                                );

        if (ColName == CurColName)
            break;
    }

    if (CurColNo == nCol)
        throw Exception(L"sqlite3 error: no column with name '" + ColName + L"'");

    return CurColNo;
}
//---------------------------------------------------------------------------

} // namespace SQLite
