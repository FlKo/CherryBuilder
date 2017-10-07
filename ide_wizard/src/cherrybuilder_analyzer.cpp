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

#include "cherrybuilder_analyzer.h"

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{
__fastcall TChBldAnalyzer::TChBldAnalyzer(
    TMemIniFile* SettingsINI,
    const String& CtagsExePath,
    TChBldProjectDB& ProjectDB
    )
    :   TThread(false),
        FSettingsINI(SettingsINI),
        FLocalSettingsINI(new TMemIniFile(NULL)),
        FCtagsParser(CtagsExePath),
        FProjectDB(ProjectDB),
        FScanningMutex(new TMutex(false)),
        FFullUpdate(false),
        FUpdateSettings(false)
{
    CS_SEND(L"Analyzer::Constructor");
}
//---------------------------------------------------------------------------

__fastcall TChBldAnalyzer::~TChBldAnalyzer()
{
    CS_SEND(L"Analyzer:Destructor");

    // Kill the thread
    Terminate();

    // Wait for the thread to fully terminate
    WaitFor();

    // Delete the scanning mutex
    if (FScanningMutex)
    {
        delete FScanningMutex;
        FScanningMutex = NULL;
    }
}
//---------------------------------------------------------------------------

void TChBldAnalyzer::FullUpdate()
{
    CS_SEND(L"Analyzer::FullUpdate");

    TChBldLockGuard LG(FScanningMutex);

    FFullUpdate = true;
}
//---------------------------------------------------------------------------

void TChBldAnalyzer::UpdateSettings()
{
    CS_SEND(L"Analyzer::UpdateSettings");

    TChBldLockGuard LG(FScanningMutex);

    FUpdateSettings = true;
}
//---------------------------------------------------------------------------

void __fastcall TChBldAnalyzer::Execute()
{
    CS_SEND(L"Analyzer::Execute (Thread, Begin)");

    VString IdeIncludePaths;
    VString ProjectIncludePaths;

    unsigned int    LastEditorContentsSync  = GetTickCount();
    unsigned int    LastFullUpdate          = GetTickCount();
    bool            ContinueFullUpdate      = false;

    // We must update the local settings in the first iteration
    FUpdateSettings = true;

    __try
    {
        while (!Terminated)
        {
            // Begin of interlock
            {
                TChBldLockGuard LG(FScanningMutex);

                // Handle a possible settings change
                if (FUpdateSettings)
                {
                    FUpdateSettings = false;

                    // Update the local settings
                    Synchronize(&SyncSettings);
                }

                // Handle a possible full update
                if (FFullUpdate)
                {
                    FFullUpdate = false;

                    // (Re)Init the DB with the new project path
                    Synchronize(&SyncSetProjectPathDB);

                    // There must be an open project to...
                    if (!FProjectDB.ProjectPath.IsEmpty())
                    {
                        // ...remove any old temp files
                        IDE::RemoveOldTempFiles(FProjectDB.ProjectPath);

                        // ...and do the first synchronization of the editor contents
                        Synchronize(&SyncEditorsContents);

                        // Clear all IDE and Project include path entries
                        IdeIncludePaths.clear();
                        ProjectIncludePaths.clear();

                        // Get the (possibly new) IDE and Project include paths
                        IDE::GetCurrentPlatformIncludePaths(IdeIncludePaths);
                        IDE::GetCurrentPlatformIncludePaths(ProjectIncludePaths, true);

                        // Continue the update in the 'unmutexed' section
                        ContinueFullUpdate = true;
                    }
                }
            }
            // End of interlock

            if (ContinueFullUpdate)
            {
                ContinueFullUpdate = false;

                // Inform the Ctags parser about some new paths
                FCtagsParser.SetProjectPath(FProjectDB.ProjectPath);
                FCtagsParser.SetIdeIncludePaths(IdeIncludePaths);
                FCtagsParser.SetProjectIncludePaths(ProjectIncludePaths);

                VString TempProjectFiles;
                std::pair<String,   String> ContentFile;

                // Get all relevant project files and assign them to a new string vector
                // (not the original files but their "temporary brothers")
                foreach_ (ContentFile, FContentFiles)
                    TempProjectFiles.push_back(ContentFile.second);

                Ctags::VTag ParsingResults;

                // Parse
                Parse(TempProjectFiles, FContentFiles, ParsingResults);

                // Add the results to the database
                FProjectDB.Refresh(ParsingResults, true);

                LastFullUpdate = GetTickCount();
            }

            // When the sync time has expired
            if ((GetTickCount() - LastEditorContentsSync) >
                static_cast<unsigned int>(
                    FLocalSettingsINI->ReadInteger(
                        L"CodeAnalyzer",
                        L"SyncInterval",
                        2000
                        )
                    )
                )
            {
                // If we have an open project
                if (!FProjectDB.ProjectPath.IsEmpty())
                {
                    // Get all of the editor contents
                    Synchronize(&SyncEditorsContents);

                    // If we have changes in the project files
                    if (FChangedContentFiles.size() > 0)
                    {
                        VString         ChangedFiles;
                        std::pair<String, String>   ChangedContentFile;

                        // Make a string vector from the changed file names
                        foreach_ (ChangedContentFile, FChangedContentFiles)
                            ChangedFiles.push_back(ChangedContentFile.second);

                        Ctags::VTag ParsingResults;

                        // Reparse the changed files
                        Parse(ChangedFiles, FContentFiles, ParsingResults);

                        // Add the results to the database after deleting everything
                        // related to those files from the DB
                        FProjectDB.Refresh(ParsingResults, false, true, &FChangedContentFiles);
                    }
                }

                LastEditorContentsSync = GetTickCount();
            }
            // When the deep refresh time has expired
            else if ((GetTickCount() - LastFullUpdate) >
                static_cast<unsigned int>(
                    FLocalSettingsINI->ReadInteger(
                        L"CodeAnalyzer",
                        L"DeepRefreshInterval",
                        600000
                        )
                    )
                )
            {
                // Begin of interlock
                {
                    TChBldLockGuard LG(FScanningMutex);

                    FFullUpdate = true;
                }
                // End of interlock
            }

            //  Wait ~50 ms to give the OS time to handle it's necessary stuff
            Sleep(50);
        }
    }
    __finally
    {
        CS_SEND(L"Analyzer::Execute(Thread, End)");
    }
}
//---------------------------------------------------------------------------

void TChBldAnalyzer::Parse(
    VString& EditorContentFiles,
    std::map<String, String>& FilenameLookupMap,
    Ctags::VTag& ParsingResults
    )
{
    CS_SEND(L"Analyzer::Parse(Begin)");
    unsigned int StartTicks = GetTickCount();

    // Clear the parsing results
    ParsingResults.clear();

    VString IncludeParsingResultFiles;

    // Do a full include file parsing with the files
    FCtagsParser.FullParseIncludes(EditorContentFiles, IncludeParsingResultFiles);

    String RtlIncludePath =
        IncludeTrailingPathDelimiter(GetEnvironmentVariable(L"BDSINCLUDE"))
            + IDE::GetCurrentTargetOS()
            + L"\\rtl\\";

    // Add some system files to the files list (they are needed but not included automatically)
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"systobj.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysclass.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"syscomp.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"syscurr.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysdefs.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysdyn.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysmac.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysopen.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"sysset.h").LowerCase());
    IncludeParsingResultFiles.push_back(String(RtlIncludePath + L"systdate.h").LowerCase());

    // Add the IDE editor content files to the files list
    foreach_ (String& EditorContentFile, EditorContentFiles)
        IncludeParsingResultFiles.push_back(EditorContentFile);

    // Do the full tag parsing
    FCtagsParser.ParseTags(IncludeParsingResultFiles, ParsingResults);

    // Iterate over each tag entry of the parsing results
    foreach_ (Ctags::TTag& ParsingResult, ParsingResults)
    {
        std::pair<String, String> FilenameLookup;

        // Iterate over each filename lookup name
        foreach_ (FilenameLookup, FilenameLookupMap)
        {
            // Replace double backslashes with only one
            ParsingResult.File =
                StringReplace(
                    ParsingResult.File,
                    L"\\\\",
                    L"\\",
                    TReplaceFlags() << rfReplaceAll
                    );

            // Replace temporary file names with the correct ones
            if (ParsingResult.File == FilenameLookup.second)
                ParsingResult.File = FilenameLookup.first;
        }
    }

    CS_SEND(L"Analyzer::Parse(End, " + String(GetTickCount() - StartTicks) + L")");
}
//---------------------------------------------------------------------------

void __fastcall TChBldAnalyzer::SyncSetProjectPathDB()
{
    CS_SEND(L"Analyzer::SyncSetProjectPathDB");

    _di_IOTAProject ActiveProject =
        IDE::GetInterface<_di_IOTAModuleServices>()->GetActiveProject();

    FProjectDB.ProjectPath = ExtractFilePath(ActiveProject->GetFileName());
}
//---------------------------------------------------------------------------

void __fastcall TChBldAnalyzer::SyncEditorsContents()
{
    if (!Terminated)
    {
        // Extract the contents of all project source files to temporary files
        // if they don't exist or have changed
        IDE::ExtractAllEditorsContent(FProjectDB.ProjectPath, FContentFiles, &FChangedContentFiles);
    }

    if (FChangedContentFiles.size() > 0)
        CS_SEND(L"Analyzer::SyncEditorsContents detected changed files: " + String(FChangedContentFiles.size()));
}
//---------------------------------------------------------------------------

void __fastcall TChBldAnalyzer::SyncSettings()
{
    CS_SEND(L"Analyzer::SyncSettings");

    std::unique_ptr<TStringList> Temp(new TStringList);

    // Update the local copy of the settings
    FSettingsINI->GetStrings(Temp.get());
    FLocalSettingsINI->SetStrings(Temp.get());
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


