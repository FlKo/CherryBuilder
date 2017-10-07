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

#ifndef cherrybuilder_sqliteH
#define cherrybuilder_sqliteH
//---------------------------------------------------------------------------

#include "sqlite3.h"
//---------------------------------------------------------------------------

namespace SQLite
{

// Note: SQLite3 uses 'serialized' (thread-safe) mode by default

enum TJournalMode : int
{
	jmDelete 	= 0,
	jmTruncate 	= 1,
	jmPersist	= 2,
	jmMemory	= 3,
	jmWal		= 4,
	jmOff		= 5
};

class TDatabase
{
public:
    TDatabase(TJournalMode JournalMode=jmDelete);
    TDatabase(const String& FileName, TJournalMode JournalMode=jmDelete);
    virtual ~TDatabase();

    sqlite3* GetHandle();

    void Open(const String& FileName);
    void Close();

    void BeginTransaction();
    void RollbackTransaction();
    void CommitTransaction();

    bool IsOpen();

    __int64 		GetLastInsertRowId();
	TJournalMode 	GetJournalMode() { return FJournalMode; }

private:
    TDatabase(const TDatabase&);            // Prevent copy-construction
    TDatabase& operator=(const TDatabase&); // Prevent assignment
	
	void SetJournalMode(TJournalMode JournalMode);

    String FFileName;
    sqlite3 *FSQLiteDB;

    bool FIsOpen;
	TJournalMode FJournalMode;
};
//---------------------------------------------------------------------------

class TStatement
{
public:
    TStatement(TDatabase& Database, const String& StatementText);
    virtual ~TStatement();

    void BindBlob(int ParamNo, const void* Val, int nBytes, bool IsStatic = false);
    void BindDouble(int ParamNo, double Val);
    void BindInt(int ParamNo, int Val);
    void BindInt64(int ParamNo, __int64 Val);
    void BindString(int ParamNo, const String& Val, bool IsStatic = false);
    void BindNull(int ParamNo);

    int GetParamCount();
    int GetColumnCount();

    int ExecuteStep();
    int Reset();

    const void*     GetColumnAsBlob(int ColNo, int& nBytes);
    const void*     GetColumnAsBlob(const String& ColName, int& nBytes);
    int             GetColumnType(int ColNo);
    double          GetColumnAsDouble(int ColNo);
    double          GetColumnAsDouble(const String& ColName);
    int             GetColumnAsInt(int ColNo);
    int             GetColumnAsInt(const String& ColName);
    __int64         GetColumnAsInt64(int ColNo);
    __int64         GetColumnAsInt64(const String& ColName);
    const String    GetColumnAsString(int ColNo);
    const String    GetColumnAsString(const String& ColName);

private:
    TStatement(const TStatement&);              // Prevent copy-construction
    TStatement& operator=(const TStatement&);   // Prevent assignment

    void Prepare();

    int GetColumnNoByName(const String& ColName);

    TDatabase   &FDatabase;
    String      FStatementText;

    sqlite3_stmt *FCompiledStatement;
};
//---------------------------------------------------------------------------

}
//---------------------------------------------------------------------------
#endif
