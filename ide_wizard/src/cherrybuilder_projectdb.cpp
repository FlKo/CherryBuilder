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

#include "cherrybuilder_projectdb.h"

#include <System.StrUtils.hpp>

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

TChBldProjectDB::TChBldProjectDB()
    :   FProjectPath(L""),
        FUpdateMutex(new TMutex(false))
{
    CS_SEND(L"ProjectDB::Constructor");
}
//---------------------------------------------------------------------------

TChBldProjectDB::~TChBldProjectDB()
{
    CS_SEND(L"ProjectDB::Destructor");

    // Delete the scanning mutex
    if (FUpdateMutex)
    {
        delete FUpdateMutex;
        FUpdateMutex = NULL;
    }
}
//---------------------------------------------------------------------------

void TChBldProjectDB::Refresh(
    const Ctags::VTag& Tags,
    bool DeepRefresh,
    bool DeleteFilesContent,
    const std::map<String, String>* ChangedContentFiles
    )
{
    CS_SEND(
        L"ProjectDB::Refresh (DeepRefresh: "
            + String((int)DeepRefresh)
            + L", Delete: "
            + String((int)DeleteFilesContent)
            + L")"
            );

    // Begin of interlock
    {
        TChBldLockGuard LG(FUpdateMutex);

        SQLite::TDatabase DB(SQLite::jmWal);

        // Create the sub-directory '__chbld' if it does not exist
        CreateWorkingDirIfRequired();

        // Create/open current project database
        DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

        __try
        {
            if (DeleteFilesContent)
            {
                String FileNames = L"";
                std::pair<String, String> ChangedContentFile;

                // Generate a comma-seperated string with the file names to update
                foreach_ (ChangedContentFile, *ChangedContentFiles)
                    FileNames += L"'" + ChangedContentFile.first + L"',";

                // Delete the last comma of the string
                FileNames.SetLength(FileNames.Length() - 1);

                SQLite::TStatement CmdDeleteChangedTags(
                    DB,
                    L"DELETE FROM Tags "
                    L"WHERE FileID IN ("
                        L"SELECT ID FROM Files "
                        L"WHERE Name IN ("
                        + FileNames
                        + L") );"
                    );

                SQLite::TStatement CmdDeleteChangedFiles(
                    DB,
                    L"DELETE FROM Files "
                    L"WHERE Name IN ("
                    + FileNames
                    + L");"
                    );

                try
                {
                    // Begin transaction
                    DB.BeginTransaction();

                    // Run command 'DeleteChangedTags'
                    CmdDeleteChangedTags.ExecuteStep();

                    // Run command 'DeleteChangedFiles'
                    CmdDeleteChangedFiles.ExecuteStep();

                    // Commit transaction
                    DB.CommitTransaction();
                }
                catch (Exception& E)
                {
                    // On exception, rollback transaction
                    DB.RollbackTransaction();

                    // Throw E again
                    throw Exception(E.Message);
                }
                catch (...)
                {
                    // On exception, rollback transaction
                    DB.RollbackTransaction();

                    // Throw Exception again
                    throw Exception(L"Unknown exception");
                }
            }

            if (DeepRefresh)
            {
                // Delete the complete tags table content
                SQLite::TStatement CmdClearTableTags(
                    DB,
                    L"DELETE FROM Tags;"
                    );

                // Delete the complete file table content
                SQLite::TStatement CmdClearTableFiles(
                    DB,
                    L"DELETE FROM Files;"
                    );

                try
                {
                    // Begin transaction
                    DB.BeginTransaction();

                    // Run command 'ClearTableTags'
                    CmdClearTableTags.ExecuteStep();

                    // Run command 'ClearTableFiles'
                    CmdClearTableFiles.ExecuteStep();

                    // Commit transaction
                    DB.CommitTransaction();
                }
                catch (Exception& E)
                {
                    // On exception, rollback transaction
                    DB.RollbackTransaction();

                    // Throw E again
                    throw Exception(E.Message);
                }
                catch (...)
                {
                    // On exception, rollback transaction
                    DB.RollbackTransaction();

                    // Throw Exception again
                    throw Exception(L"Unknown exception");
                }
            }

            // Create a map for the file name/ID relations
            std::map<String, String> FileMap;

            // Get the current files list from the files table
            SQLite::TStatement QryGetFileList(
                DB,
                L"SELECT * FROM Files;"
                );

            while (QryGetFileList.ExecuteStep() != SQLITE_DONE)
                FileMap[QryGetFileList.GetColumnAsString(1)] = QryGetFileList.GetColumnAsString(0);


            // Create a vector of SQLite statement pointers
            // (Cannot use statements directly as we don't provide an overloaded assignment operator)
            std::vector<SQLite::TStatement*> SQLiteStatements;

            // Iterate over each tag record
            for (std::size_t i = 0; i < Tags.size(); ++i)
            {
                // Check, whether the file is not already existent in the map...
                if (FileMap.find(Tags[i].File) == FileMap.end())
                {
                    // ...so we have to create an entry with a new GUID
                    FileMap[Tags[i].File] = Environment::CreateGuidString(true);
                }

                // Add the tag record data to database
                SQLiteStatements.push_back(
                    new SQLite::TStatement(
                        DB,
                        L"INSERT OR IGNORE INTO Tags("
                            L"Name,"
                            L"QualifiedName,"
                            L"FileID,"
                            L"Address,"
                            L"Kind,"
                            L"LineNo,"
                            L"Namespace,"
                            L"Class,"
                            L"Struct,"
                            L"Access,"
                            L"Implementation,"
                            L"Signature,"
                            L"Typeref_A,"
                            L"Typeref_B,"
                            L"Inherits"
                            L") "
                        L"VALUES("
                            L"?1,"
                            L"?2,"
                            L"?3,"
                            L"?4,"
                            L"?5,"
                            L"?6,"
                            L"?7,"
                            L"?8,"
                            L"?9,"
                            L"?10,"
                            L"?11,"
                            L"?12,"
                            L"?13,"
                            L"?14,"
                            L"?15"
                            L");"
                        )
                    );

                SQLiteStatements.back()->BindString(  1, Tags[i].Name);
                SQLiteStatements.back()->BindString(  2, Tags[i].QualifiedName);
                SQLiteStatements.back()->BindString(  3, FileMap[Tags[i].File]);
                SQLiteStatements.back()->BindString(  4, Tags[i].Address);
                SQLiteStatements.back()->BindString(  5, Tags[i].Kind);
                SQLiteStatements.back()->BindInt(     6, Tags[i].LineNo);
                SQLiteStatements.back()->BindString(  7, Tags[i].Namespace);
                SQLiteStatements.back()->BindString(  8, Tags[i].Class);
                SQLiteStatements.back()->BindString(  9, Tags[i].Struct);
                SQLiteStatements.back()->BindString( 10, Tags[i].Access);
                SQLiteStatements.back()->BindString( 11, Tags[i].Implementation);
                SQLiteStatements.back()->BindString( 12, Tags[i].Signature);
                SQLiteStatements.back()->BindString( 13, Tags[i].Typeref_A);
                SQLiteStatements.back()->BindString( 14, Tags[i].Typeref_B);
                SQLiteStatements.back()->BindString( 15, Tags[i].Inherits);
            }

            std::pair<String, String> File;

            // Iterate over each file name
            foreach_ (File, FileMap)
            {

                SQLiteStatements.push_back(
                    new SQLite::TStatement(
                        DB,
                        L"INSERT OR IGNORE INTO Files(ID, Name) "
                        L"VALUES(?1, ?2);"
                        )
                    );

                SQLiteStatements.back()->BindString(1, File.second);
                SQLiteStatements.back()->BindString(2, File.first);
            }

            try
            {
                // Begin transaction
                DB.BeginTransaction();

                // Run each statement from SQLitestatements vector
                for (std::size_t i = 0; i < SQLiteStatements.size(); ++i)
                    SQLiteStatements[i]->ExecuteStep();

                // Commit transaction
                DB.CommitTransaction();
            }
            catch (Exception& E)
            {
                // On exception, rollback transaction
                DB.RollbackTransaction();

                // Throw E again
                //throw Exception(E.Message);   // Don't throw here: If it doesn't work
                                                // maybe there's an active code completion
                                                // running
            }
            catch (...)
            {
                // On exception, rollback transaction
                DB.RollbackTransaction();

                // Throw Exception again
                //throw Exception(L"Unknown exception");    // Don't throw here: If it doesn't work
                                                            // maybe there's an active code completion
                                                            // running
            }

            /*

            // In either case it is necessary to fully qualify the 'Inherits' fields which
            // we're doing in the next steps

            // Get all records with an non-empty inheritance field
            SQLite::TStatement QryGetTagsWithInheritance(
                DB,
                L"SELECT * FROM Common WHERE Inherits <> '';"
                );

            // Iterate over each of it
            while (QryGetTagsWithInheritance.ExecuteStep() != SQLITE_DONE)
            {
                __int64 ID          = QryGetTagsWithInheritance.GetColumnAsInt64(0);
                String  Inherits    = QryGetTagsWithInheritance.GetColumnAsString(15);

                // There may be multiple ancestors which we're seperating here
                VString Ancestors = Environment::SplitCtagsObjectAncestors(Inherits);

                // We must perform the 'qualification' seperately for each ancestor
                for (std::size_t i = 0; i < Ancestors.size(); ++i)
                {
                    String &Ancestor = Ancestors[i];

                    // Build the query to get the ancestor tag
                    SQLite::TStatement QryGetAncestorTag(
                        DB,
                        L"SELECT * FROM Tags WHERE QualifiedName LIKE '%" + Ancestor + L"' "
                            L"AND ((Kind = 'class') OR (Kind = 'struct'));"
                        );

                    // Perform the query - If we have a result...
                    if (!QryGetAncestorTag.ExecuteStep() != SQLITE_DONE)
                    {
                        // ...we get it's qualified name
                        String QualifiedName = QryGetAncestorTag.GetColumnAsString(2);

                        // Now that we've got a qualified name we have to verify it


                        Ancestor = QualifiedName;
                    }
                }

                // At this point 'Ancestors' contains the list of all qualified ancestor names
                // for the current tag

                String AncestorList = L"";

                foreach_ (String Ancestor, Ancestors)
                    AncestorList = Ancestor + ",";

                if (!AncestorList.IsEmpty())
                    AncestorList = LeftStr(AncestorList, AncestorList.Length() - 1);

                if (AncestorList != QryGetTagsWithInheritance.GetColumnAsString(15))
                {
                    SQLite::TStatement CmdUpdateInheritsField(
                        DB,
                        L"REPLACE INTO Tags("
                            L"ID,"
                            L"Name,"
                            L"QualifiedName,"
                            L"FileID,"
                            L"Address,"
                            L"Kind,"
                            L"LineNo,"
                            L"Namespace,"
                            L"Class,"
                            L"Struct,"
                            L"Access,"
                            L"Implementation,"
                            L"Signature,"
                            L"Typeref_A,"
                            L"Typeref_B,"
                            L"Inherits"
                            L") "
                        L"VALUES("
                            L"?1,"
                            L"?2,"
                            L"?3,"
                            L"?4,"
                            L"?5,"
                            L"?6,"
                            L"?7,"
                            L"?8,"
                            L"?9,"
                            L"?10,"
                            L"?11,"
                            L"?12,"
                            L"?13,"
                            L"?14,"
                            L"?15,"
                            L"?16"
                            L");"
                        );

                    CmdUpdateInheritsField.BindInt64(   1, ID);
                    CmdUpdateInheritsField.BindString(  2, QryGetTagsWithInheritance.GetColumnAsString(1));
                    CmdUpdateInheritsField.BindString(  3, QryGetTagsWithInheritance.GetColumnAsString(2));
                    CmdUpdateInheritsField.BindString(  4, QryGetTagsWithInheritance.GetColumnAsString(3));
                    CmdUpdateInheritsField.BindString(  5, QryGetTagsWithInheritance.GetColumnAsString(4));
                    CmdUpdateInheritsField.BindString(  6, QryGetTagsWithInheritance.GetColumnAsString(5));
                    CmdUpdateInheritsField.BindString(  7, QryGetTagsWithInheritance.GetColumnAsString(6));
                    CmdUpdateInheritsField.BindString(  8, QryGetTagsWithInheritance.GetColumnAsString(7));
                    CmdUpdateInheritsField.BindString(  9, QryGetTagsWithInheritance.GetColumnAsString(8));
                    CmdUpdateInheritsField.BindString( 10, QryGetTagsWithInheritance.GetColumnAsString(9));
                    CmdUpdateInheritsField.BindString( 11, QryGetTagsWithInheritance.GetColumnAsString(10));
                    CmdUpdateInheritsField.BindString( 12, QryGetTagsWithInheritance.GetColumnAsString(11));
                    CmdUpdateInheritsField.BindString( 13, QryGetTagsWithInheritance.GetColumnAsString(12));
                    CmdUpdateInheritsField.BindString( 14, QryGetTagsWithInheritance.GetColumnAsString(13));
                    CmdUpdateInheritsField.BindString( 15, QryGetTagsWithInheritance.GetColumnAsString(14));
                    CmdUpdateInheritsField.BindString( 16, AncestorList);

                    try
                    {
                        // Begin transaction
                        DB.BeginTransaction();

                        CmdUpdateInheritsField.ExecuteStep();

                        // Commit transaction
                        DB.CommitTransaction();
                    }
                    catch (Exception& E)
                    {
                        // On exception, rollback transaction
                        DB.RollbackTransaction();

                        // Throw E again
                        //throw Exception(E.Message);   // Don't throw here: If it doesn't work
                                                        // maybe there's an active code completion
                                                        // running
                    }
                    catch (...)
                    {
                        // On exception, rollback transaction
                        DB.RollbackTransaction();

                        // Throw Exception again
                        //throw Exception(L"Unknown exception");    // Don't throw here: If it doesn't work
                                                                    // maybe there's an active code completion
                                                                    // running
                    }
                }
            }
            */
        }
        __finally
        {
            DB.Close();
        }
    }
    // End of interlock
}
//---------------------------------------------------------------------------

void  TChBldProjectDB::GetMatchingIdentifierList(const String& Query, Ctags::VTag& List, bool ClearList)
{
    CS_SEND(L"ProjectDB::GetMatchingIdentifierList(" + Query + L")");

    SQLite::TDatabase DB(SQLite::jmWal);

    // Clear the list if requested
    if (ClearList)
        List.clear();

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetMatchingIdentifiers(
            DB,
            Query
            );

        while (QryGetMatchingIdentifiers.ExecuteStep() != SQLITE_DONE)
        {
            Ctags::TTag Symbol;

            Symbol.Name             = QryGetMatchingIdentifiers.GetColumnAsString(1);
            Symbol.QualifiedName    = QryGetMatchingIdentifiers.GetColumnAsString(2);
            Symbol.File             = QryGetMatchingIdentifiers.GetColumnAsString(3);
            Symbol.Address          = QryGetMatchingIdentifiers.GetColumnAsString(4);
            Symbol.Kind             = QryGetMatchingIdentifiers.GetColumnAsString(5);
            Symbol.LineNo           = QryGetMatchingIdentifiers.GetColumnAsInt(6);
            Symbol.Namespace        = QryGetMatchingIdentifiers.GetColumnAsString(7);
            Symbol.Class            = QryGetMatchingIdentifiers.GetColumnAsString(8);
            Symbol.Struct           = QryGetMatchingIdentifiers.GetColumnAsString(9);
            Symbol.Access           = QryGetMatchingIdentifiers.GetColumnAsString(10);
            Symbol.Implementation   = QryGetMatchingIdentifiers.GetColumnAsString(11);
            Symbol.Signature        = QryGetMatchingIdentifiers.GetColumnAsString(12);
            Symbol.Typeref_A        = QryGetMatchingIdentifiers.GetColumnAsString(13);
            Symbol.Typeref_B        = QryGetMatchingIdentifiers.GetColumnAsString(14);
            Symbol.Inherits         = QryGetMatchingIdentifiers.GetColumnAsString(15);

            bool CanInsert = true;

            if (Symbol.Kind == L"property")
            {
                if (Symbol.Typeref_B.IsEmpty())
                    CanInsert = false;
            }

            if (CanInsert)
            {
                bool Exists = false;

                for (std::size_t i = 0; i < List.size(); ++i)
                {
                    if (    (List[i].Name == Symbol.Name) &&
                            (List[i].Kind == Symbol.Kind) &&
                            (List[i].Typeref_B == Symbol.Typeref_B)
                            )
                    {
                        Exists = true;
                        break;
                    }

                }

                if (!Exists)
                    List.push_back(Symbol);
            }
        }

    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }
}
//---------------------------------------------------------------------------

void TChBldProjectDB::GetPosNamespaces(
    const String& FileName,
    const int Line,
    const int Column,
    VString& Namespaces
    )
{
    CS_SEND(L"ProjectDB::GetPosNamespaces");

    SQLite::TDatabase DB(SQLite::jmWal);

    // Clear the namespaces
    Namespaces.clear();

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetNamespaces(
            DB,
            L"SELECT * FROM Common "
            L"WHERE FileName = '"
            + FileName + L"' "
            L"AND Kind = 'namespace' "
            L"AND LineNo <= " + Line + L" "
            L"ORDER BY LineNo ASC;"
            );

        while (QryGetNamespaces.ExecuteStep() != SQLITE_DONE)
            Namespaces.push_back(QryGetNamespaces.GetColumnAsString(1));
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }
}
//---------------------------------------------------------------------------

void TChBldProjectDB::GetPosClassesAndStructs(
    const String& FileName,
    const int Line,
    const int Column,
    VString& Classes
    )
{
    CS_SEND(L"ProjectDB::GetPosClassesAndStructs");

    SQLite::TDatabase DB(SQLite::jmWal);

    // Clear the classes
    Classes.clear();

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {

    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }
}
//---------------------------------------------------------------------------

Ctags::TTag TChBldProjectDB::GetPosSymbol(const String& SymbolText)
{
    CS_SEND(L"ProjectDB::GetPosSymbol");

    Ctags::TTag Symbol;

    SQLite::TDatabase DB(SQLite::jmWal);

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetSymbol(
            DB,
            L"SELECT * FROM Common "
            L"WHERE TagName = '" + SymbolText + L"' "
            L"AND ((Kind = 'variable') OR (Kind = 'local') OR (Kind = 'function') OR (Kind = 'class') "
            L"OR (Kind = 'struct') OR (Kind = 'namespace') OR (Kind = 'typedef') OR "
            L"(Kind = 'enumerator') OR (Kind = 'enum') OR (Kind = 'constructor') OR (Kind = 'destructor'))"
            );

        if (QryGetSymbol.ExecuteStep() != SQLITE_DONE)
        {
            Symbol.Name             = QryGetSymbol.GetColumnAsString(1);
            Symbol.QualifiedName    = QryGetSymbol.GetColumnAsString(2);
            Symbol.File             = QryGetSymbol.GetColumnAsString(3);
            Symbol.Address          = QryGetSymbol.GetColumnAsString(4);
            Symbol.Kind             = QryGetSymbol.GetColumnAsString(5);
            Symbol.LineNo           = QryGetSymbol.GetColumnAsInt(6);
            Symbol.Namespace        = QryGetSymbol.GetColumnAsString(7);
            Symbol.Class            = QryGetSymbol.GetColumnAsString(8);
            Symbol.Struct           = QryGetSymbol.GetColumnAsString(9);
            Symbol.Access           = QryGetSymbol.GetColumnAsString(10);
            Symbol.Implementation   = QryGetSymbol.GetColumnAsString(11);
            Symbol.Signature        = QryGetSymbol.GetColumnAsString(12);
            Symbol.Typeref_A        = QryGetSymbol.GetColumnAsString(13);
            Symbol.Typeref_B        = QryGetSymbol.GetColumnAsString(14);
            Symbol.Inherits         = QryGetSymbol.GetColumnAsString(15);
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }

    return Symbol;
}
//---------------------------------------------------------------------------

Ctags::TTag TChBldProjectDB::GetPosImplementation(const String& FileName, const int Line, const int Column)
{
    CS_SEND(L"ProjectDB::GetPosImplementation");

    SQLite::TDatabase DB(SQLite::jmWal);

    Ctags::TTag Symbol;

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetEnclosedFunction(
            DB,
            L"SELECT * FROM Common "
            L"WHERE FileName = '"
            + FileName + L"' "
            L"AND Kind = 'implementation' "
            L"AND LineNo <= " + Line + L" "
            L"ORDER BY LineNo DESC;"
            );

        if (QryGetEnclosedFunction.ExecuteStep() != SQLITE_DONE)
        {
            Symbol.Name             = QryGetEnclosedFunction.GetColumnAsString(1);
            Symbol.QualifiedName    = QryGetEnclosedFunction.GetColumnAsString(2);
            Symbol.File             = QryGetEnclosedFunction.GetColumnAsString(3);
            Symbol.Address          = QryGetEnclosedFunction.GetColumnAsString(4);
            Symbol.Kind             = QryGetEnclosedFunction.GetColumnAsString(5);
            Symbol.LineNo           = QryGetEnclosedFunction.GetColumnAsInt(6);
            Symbol.Namespace        = QryGetEnclosedFunction.GetColumnAsString(7);
            Symbol.Class            = QryGetEnclosedFunction.GetColumnAsString(8);
            Symbol.Struct           = QryGetEnclosedFunction.GetColumnAsString(9);
            Symbol.Access           = QryGetEnclosedFunction.GetColumnAsString(10);
            Symbol.Implementation   = QryGetEnclosedFunction.GetColumnAsString(11);
            Symbol.Signature        = QryGetEnclosedFunction.GetColumnAsString(12);
            Symbol.Typeref_A        = QryGetEnclosedFunction.GetColumnAsString(13);
            Symbol.Typeref_B        = QryGetEnclosedFunction.GetColumnAsString(14);
            Symbol.Inherits         = QryGetEnclosedFunction.GetColumnAsString(15);
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }

    return Symbol;
}
//---------------------------------------------------------------------------

Ctags::TTag TChBldProjectDB::GetPosHeader(const String& FileName, const int Line, const int Column)
{
    CS_SEND(L"ProjectDB::GetPosHeader");

    SQLite::TDatabase DB(SQLite::jmWal);

    Ctags::TTag Symbol;

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetHeader(
            DB,
            L"SELECT * FROM Common "
            L"WHERE ((Kind = 'function') OR (Kind = 'constructor') OR (Kind = 'destructor')) "
            L"AND FileName = '" + FileName + L"' "
            L"AND LineNo = " + String(Line) + L" "
            L"ORDER BY LineNo DESC;"
            );

        if (QryGetHeader.ExecuteStep() != SQLITE_DONE)
        {
            Symbol.Name             = QryGetHeader.GetColumnAsString(1);
            Symbol.QualifiedName    = QryGetHeader.GetColumnAsString(2);
            Symbol.File             = QryGetHeader.GetColumnAsString(3);
            Symbol.Address          = QryGetHeader.GetColumnAsString(4);
            Symbol.Kind             = QryGetHeader.GetColumnAsString(5);
            Symbol.LineNo           = QryGetHeader.GetColumnAsInt(6);
            Symbol.Namespace        = QryGetHeader.GetColumnAsString(7);
            Symbol.Class            = QryGetHeader.GetColumnAsString(8);
            Symbol.Struct           = QryGetHeader.GetColumnAsString(9);
            Symbol.Access           = QryGetHeader.GetColumnAsString(10);
            Symbol.Implementation   = QryGetHeader.GetColumnAsString(11);
            Symbol.Signature        = QryGetHeader.GetColumnAsString(12);
            Symbol.Typeref_A        = QryGetHeader.GetColumnAsString(13);
            Symbol.Typeref_B        = QryGetHeader.GetColumnAsString(14);
            Symbol.Inherits         = QryGetHeader.GetColumnAsString(15);
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }

    return Symbol;
}
//---------------------------------------------------------------------------

Ctags::TTag TChBldProjectDB::GetPosHeaderTarget(Ctags::TTag& Symbol)
{
    CS_SEND(L"ProjectDB::GetPosHeaderTarget");

    SQLite::TDatabase DB(SQLite::jmWal);

    Ctags::TTag HeaderSymbol;

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetMatchingHeader(
            DB,
            L"SELECT * FROM Common "
            L"WHERE ((Kind = 'function') OR (Kind = 'constructor') OR (Kind = 'destructor')) "
            L"AND TagName = '" + Symbol.Name + L"' "
            L"AND Signature = '" + Symbol.Signature + L"';"
            );

        if (QryGetMatchingHeader.ExecuteStep() != SQLITE_DONE)
        {
            HeaderSymbol.Name             = QryGetMatchingHeader.GetColumnAsString(1);
            HeaderSymbol.QualifiedName    = QryGetMatchingHeader.GetColumnAsString(2);
            HeaderSymbol.File             = QryGetMatchingHeader.GetColumnAsString(3);
            HeaderSymbol.Address          = QryGetMatchingHeader.GetColumnAsString(4);
            HeaderSymbol.Kind             = QryGetMatchingHeader.GetColumnAsString(5);
            HeaderSymbol.LineNo           = QryGetMatchingHeader.GetColumnAsInt(6);
            HeaderSymbol.Namespace        = QryGetMatchingHeader.GetColumnAsString(7);
            HeaderSymbol.Class            = QryGetMatchingHeader.GetColumnAsString(8);
            HeaderSymbol.Struct           = QryGetMatchingHeader.GetColumnAsString(9);
            HeaderSymbol.Access           = QryGetMatchingHeader.GetColumnAsString(10);
            HeaderSymbol.Implementation   = QryGetMatchingHeader.GetColumnAsString(11);
            HeaderSymbol.Signature        = QryGetMatchingHeader.GetColumnAsString(12);
            HeaderSymbol.Typeref_A        = QryGetMatchingHeader.GetColumnAsString(13);
            HeaderSymbol.Typeref_B        = QryGetMatchingHeader.GetColumnAsString(14);
            HeaderSymbol.Inherits         = QryGetMatchingHeader.GetColumnAsString(15);
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }

    return HeaderSymbol;
}
//---------------------------------------------------------------------------

Ctags::TTag TChBldProjectDB::GetPosImplementationTarget(Ctags::TTag& Symbol)
{
    CS_SEND(L"ProjectDB::GetPosImplementationTarget");

    SQLite::TDatabase DB(SQLite::jmWal);

    Ctags::TTag ImplementationSymbol;

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        SQLite::TStatement QryGetMatchingImplementation(
            DB,
            L"SELECT * FROM Common "
            L"WHERE Kind = 'implementation' "
            L"AND TagName = '" + Symbol.Name + L"' "
            L"AND Signature = '" + Symbol.Signature + L"';"
            );

        if (QryGetMatchingImplementation.ExecuteStep() != SQLITE_DONE)
        {
            ImplementationSymbol.Name           = QryGetMatchingImplementation.GetColumnAsString(1);
            ImplementationSymbol.QualifiedName  = QryGetMatchingImplementation.GetColumnAsString(2);
            ImplementationSymbol.File           = QryGetMatchingImplementation.GetColumnAsString(3);
            ImplementationSymbol.Address        = QryGetMatchingImplementation.GetColumnAsString(4);
            ImplementationSymbol.Kind           = QryGetMatchingImplementation.GetColumnAsString(5);
            ImplementationSymbol.LineNo         = QryGetMatchingImplementation.GetColumnAsInt(6);
            ImplementationSymbol.Namespace      = QryGetMatchingImplementation.GetColumnAsString(7);
            ImplementationSymbol.Class          = QryGetMatchingImplementation.GetColumnAsString(8);
            ImplementationSymbol.Struct         = QryGetMatchingImplementation.GetColumnAsString(9);
            ImplementationSymbol.Access         = QryGetMatchingImplementation.GetColumnAsString(10);
            ImplementationSymbol.Implementation = QryGetMatchingImplementation.GetColumnAsString(11);
            ImplementationSymbol.Signature      = QryGetMatchingImplementation.GetColumnAsString(12);
            ImplementationSymbol.Typeref_A      = QryGetMatchingImplementation.GetColumnAsString(13);
            ImplementationSymbol.Typeref_B      = QryGetMatchingImplementation.GetColumnAsString(14);
            ImplementationSymbol.Inherits       = QryGetMatchingImplementation.GetColumnAsString(15);
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }

    return ImplementationSymbol;
}
//---------------------------------------------------------------------------

void TChBldProjectDB::CreateWorkingDirIfRequired()
{
    //CS_SEND(L"ProjectDB::CreateWorkingDirIfRequired");

    // If the working dir does does not exist...
    if (!DirectoryExists(FProjectPath + L"__chbld"))
    {
        // ...create it...
        ForceDirectories(FProjectPath + L"__chbld");

        // ...and the hide it
        FileSetAttr(FProjectPath + L"__chbld", faHidden, false);
    }
}
//---------------------------------------------------------------------------

void TChBldProjectDB::SetProjectPath(const String& AProjectPath)
{
    CS_SEND(L"ProjectDB::SetProjectPath(" + AProjectPath + L")");

    // If the project path has REALLY changed
    if (AProjectPath != FProjectPath)
    {
        if (!AProjectPath.IsEmpty())
        {
            // Assign the project path
            FProjectPath = AProjectPath;

            // Change the DB context to the new project
            ChangeProjectContext();
        }
    }
}
//---------------------------------------------------------------------------

void TChBldProjectDB::ChangeProjectContext()
{
    CS_SEND(L"ProjectDB::ChangeProjectContext");

    SQLite::TDatabase DB(SQLite::jmWal);

    // Create the sub-directory '__chbld' if it does not exist
    CreateWorkingDirIfRequired();

    // Create/open current project database
    DB.Open(FProjectPath + L"__chbld\\" + kDBFileName);

    __try
    {
        // Create table 'Tags' (if non existent)
        SQLite::TStatement CmdAddTableTags(
            DB,
            L"CREATE TABLE IF NOT EXISTS Tags("
                L"ID                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                L"Name              TEXT    NOT NULL,"
                L"QualifiedName     TEXT    NOT NULL,"
                L"FileID            TEXT    NOT NULL,"
                L"Address           TEXT    NOT NULL,"
                L"Kind              TEXT    NOT NULL,"
                L"LineNo            INT,"
                L"Namespace         TEXT,"
                L"Class             TEXT,"
                L"Struct            TEXT,"
                L"Access            TEXT,"
                L"Implementation    TEXT,"
                L"Signature         TEXT,"
                L"Typeref_A         TEXT,"
                L"Typeref_B         TEXT,"
                L"Inherits          TEXT"
                L");"
            );

        // Create table 'Files' (if non existent)
        SQLite::TStatement CmdAddTableFiles(
            DB,
            L"CREATE TABLE IF NOT EXISTS Files("
                L"ID        TEXT    NOT NULL PRIMARY KEY,"
                L"Name      TEXT    NOT NULL UNIQUE"
                L");"
            );

        // Create view 'Common' (if non existent)
        SQLite::TStatement CmdAddViewCommon(
            DB,
            L"CREATE VIEW IF NOT EXISTS Common AS "
                L"SELECT "
                    L"Tags.ID AS ID,"
                    L"Tags.Name AS TagName,"
                    L"Tags.QualifiedName AS QualifiedName,"
                    L"Files.Name AS FileName,"
                    L"Tags.Address AS Address,"
                    L"Tags.Kind AS Kind,"
                    L"Tags.LineNo AS LineNo,"
                    L"Tags.Namespace AS Namespace,"
                    L"Tags.Class AS Class,"
                    L"Tags.Struct AS Struct,"
                    L"Tags.Access AS Access,"
                    L"Tags.Implementation AS Implementation,"
                    L"Tags.Signature AS Signature,"
                    L"Tags.Typeref_A AS Typeref_A,"
                    L"Tags.Typeref_B AS Typeref_B,"
                    L"Tags.Inherits AS Inherits "
                L"FROM "
                    L"Tags "
                L"INNER JOIN "
                    L"Files ON Files.ID = Tags.FileID"
            L";"
            );

        try
        {
            // Begin transaction
            DB.BeginTransaction();

            // Run command 'AddTableTags'
            CmdAddTableTags.ExecuteStep();

            // Run command 'AddTableFiles'
            CmdAddTableFiles.ExecuteStep();

            // Run command 'AddViewCommon'
            CmdAddViewCommon.ExecuteStep();

            // Commit transaction
            DB.CommitTransaction();
        }
        catch (Exception& E)
        {
            // On exception, rollback transaction
            DB.RollbackTransaction();

            // Throw E again
            throw Exception(E.Message);
        }
        catch (...)
        {
            // On exception, rollback transaction
            DB.RollbackTransaction();

            // Throw Exception again
            throw Exception(L"Unknown exception");
        }

        // Create unique index for table 'Tags'
        SQLite::TStatement CmdAddUniqueIndexTags(
            DB,
            L"CREATE UNIQUE INDEX IF NOT EXISTS UniqueIndex "
            L"ON Tags ("
                L"Name,"
                L"QualifiedName,"
                L"FileID,"
                L"Address,"
                L"Kind,"
                L"LineNo,"
                L"Namespace,"
                L"Class,"
                L"Struct,"
                L"Access,"
                L"Implementation,"
                L"Signature,"
                L"Typeref_A,"
                L"Typeref_B,"
                L"Inherits"
                L");"
            );

        // Delete the content of 'Tags'
        SQLite::TStatement CmdClearTableTags(
            DB,
            L"DELETE FROM Tags;"
            );

        // Delete the content of 'Files'
        SQLite::TStatement CmdClearTableFiles(
            DB,
            L"DELETE FROM Files;"
            );

        try
        {
            // Begin transaction
            DB.BeginTransaction();

            // Run command 'ClearTableTags'
            CmdClearTableTags.ExecuteStep();

            // Run command 'ClearTableFiles'
            CmdClearTableFiles.ExecuteStep();

            // Run command 'AddUniqueIndexTags'
            CmdAddUniqueIndexTags.ExecuteStep();

            // Commit transaction
            DB.CommitTransaction();
        }
        catch (Exception& E)
        {
            // On exception, rollback transaction
            DB.RollbackTransaction();

            // Throw E again
            throw Exception(E.Message);
        }
        catch (...)
        {
            // On exception, rollback transaction
            DB.RollbackTransaction();

            // Throw Exception again
            throw Exception(L"Unknown exception");
        }
    }
    __finally
    {
        // Close the connection to database
        DB.Close();
    }
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


