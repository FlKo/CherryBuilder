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

#include "cherrybuilder_ide.h"

#include <System.Hash.hpp>
#include <System.IOUtils.hpp>

#include <Rtti.hpp>

#include <set>

#include "cherrybuilder_winmode.h"
#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

_di_IBorlandIDEServices IDE::FIDESrv;
//---------------------------------------------------------------------------

TMenuItem* IDE::AddToolsMenuItem(
    const String& ACaption,
    TNotifyEvent AOnClick,
    const String& AResourceImageName
    )
{
    // Create a TBitmap for the menu icon
    std::unique_ptr<TBitmap> MenuIcon(new TBitmap);

    // Load the root menu bitmap from the resources
    MenuIcon->LoadFromResourceName(reinterpret_cast<unsigned int>(HInstance), AResourceImageName);

    // Get NTAServices
    _di_INTAServices NTAServices = GetInterface<_di_INTAServices>();

    // Add the bitmap with color mask to the NTAServices
    int ImgIndex = NTAServices->AddMasked(
        MenuIcon.get(),
        static_cast<TColor>(0x00FF00FF),
        ACaption
        );

    // Find the 'Tools' main menu item (language specific)
    String ToolsMenuTranslations[3] = {L"Tools", L"Outils", L"ツール"};
    TMenuItem *mmiTools             = NULL;

    for (int i = 0; i < 3 ; ++i)
    {
        mmiTools = NTAServices->MainMenu->Items->Find(ToolsMenuTranslations[i]);

        if (mmiTools)
            break;
    }

    if (mmiTools)
    {
        // Find the first devider in the 'Tools' menu
        TMenuItem *mmiFirstDivider = mmiTools->Find(L"-");

        if (mmiFirstDivider)
        {
            // Create the new root menu item object
            TMenuItem *MenuItem = new TMenuItem(mmiTools);

            // Set the options
            MenuItem->ImageIndex   = ImgIndex;
            MenuItem->Caption      = ACaption;
            MenuItem->OnClick      = AOnClick;

            // Assign the object as new entry within the "Tools" menu
            mmiTools->Insert(mmiFirstDivider->MenuIndex + 1, MenuItem);
        }
    }

    return mmiTools;
}
//---------------------------------------------------------------------------

void IDE::GetCurrentPlatformIncludePaths(VString& Paths,
    bool GetProjectPathsInstead
    )
{
    // Clear paths list
    Paths.clear();

    // Get the active project
    _di_IOTAProject ActiveProject = GetInterface<_di_IOTAModuleServices>()->GetActiveProject();

    // Make some interface names shorter
    typedef _di_IOTAProjectOptions                  _di_IOTAPO;
    typedef _di_IOTAProjectOptionsConfigurations    _di_IOTAPOCs;
    typedef _di_IOTABuildConfiguration              _di_IOTABC;

    // Get the configurations interface
    _di_IOTAPOCs Configurations = GetInterface<_di_IOTAPOCs, _di_IOTAPO>(ActiveProject->ProjectOptions);

    // Get the active project configuration
    _di_IOTABC Configuration = Configurations->GetActiveConfiguration();

    if (!GetProjectPathsInstead)
    {
        // Get the active platform name
        String ActivePlatform = Configuration->GetPlatform();

        // Get the registry base path for the include keys, based on the active platform
        String PathKey =
            GetInterface<_di_IOTAServices>()->GetBaseRegistryKey()
                + L"\\C++\\Paths\\"
                + ActivePlatform
                + L"\\";

        VString PathList;

        // Open the platform key
        std::unique_ptr<TRegistry> Registry(new TRegistry(KEY_READ));
        Registry->RootKey = HKEY_CURRENT_USER;
        Registry->OpenKey(PathKey, false);

        __try
        {
            // Add the IDE path entries depending on the platform and the compiler
            if (Configurations->GetActiveConfiguration()->GetValue(L"BCC_UseClassicCompiler") == L"false")
            {
                PathList.push_back(Registry->ReadString(L"BrowsingPath_CLang32")); // Typo is intentional
                PathList.push_back(Registry->ReadString(L"IncludePath_Clang32"));
                PathList.push_back(Registry->ReadString(L"LibraryPath_Clang32"));
                PathList.push_back(Registry->ReadString(L"UserIncludePath_Clang32"));
            }
            else
            {
                PathList.push_back(Registry->ReadString(L"BrowsingPath"));
                PathList.push_back(Registry->ReadString(L"IncludePath"));
                PathList.push_back(Registry->ReadString(L"LibraryPath"));
                PathList.push_back(Registry->ReadString(L"UserIncludePath"));
            }
        }
        __finally
        {
            Registry->CloseKey();
        }

        VString IDELibPaths;

        // Iterate over each path to...
        foreach_ (String& Path, PathList)
        {
            IDELibPaths = Environment::SplitStr(Path, L';');

            for (std::size_t i = 0; i < IDELibPaths.size(); ++i)
            {
                // ...expand the environment variables
                String CurPath = GetExpandedPath(IDELibPaths[i]);

                // ...check if the expanded directory is existing
                if (DirectoryExists(CurPath))
                {
                    // ...normalize the path name
                    CurPath = GetNormalizedPath(CurPath);

                    // ...add it to the path list
                    Paths.push_back(CurPath);
                }
            }
        }
    }
    else
    {
        // There must be an open project
        if (!ActiveProject)
            return;

        // Create a StringList for the project include paths
        std::unique_ptr<TStringList> ProjectPaths(new TStringList);
        ProjectPaths->Delimiter         = L';';
        ProjectPaths->StrictDelimiter   = true;

        // Add the path values to the StringList
        Configuration->GetValues(L"IncludePath", ProjectPaths.get(), true);

        // Iterate over each path to...
        for (int i = 0; i < ProjectPaths->Count; ++i)
        {
            // ...expand the environment variables and relative paths
            String CurPath = GetExpandedPath(ProjectPaths->Strings[i]);

            // ...check if the expanded directory is existing
            if (DirectoryExists(CurPath))
            {
                // ...normalize the path name
                CurPath = GetNormalizedPath(CurPath);

                // ...add it to the path list
                Paths.push_back(CurPath);
            }
        }

        // Get the current project's base path and add it to the paths list, too
        String ProjectPath = ExtractFilePath(ActiveProject->GetFileName());

        Paths.push_back(GetNormalizedPath(ProjectPath));
    }
}
//---------------------------------------------------------------------------

String IDE::GetExpandedPath(String Path)
{
    wchar_t ExpandedBuf[MAX_PATH];
    String  ExpandedPath;

    // Change the 'Borland' IDE environment path variable delimiters to '%ABC%' ones
    Path = StringReplace(Path, L"$(", L"%", TReplaceFlags() << rfReplaceAll);
    Path = StringReplace(Path, L")", L"%", TReplaceFlags() << rfReplaceAll);

    _di_IOTAProject ActiveProject = GetInterface<_di_IOTAModuleServices>()->GetActiveProject();

    // Manually resolve the %Platform% variable (it's not resolved by 'ExpandEnvironmentStringsW')
    // --> a bug in 10.2 ?
    Path =
        StringReplace(
            Path,
            L"%Platform%",
            ActiveProject->GetPlatform(),
            TReplaceFlags() << rfReplaceAll << rfIgnoreCase
            );

    // Expand the other environment variables
    if (!ExpandEnvironmentStringsW(Path.w_str(), ExpandedBuf, sizeof(ExpandedBuf)))
        throw Exception(Environment::GetWinAPILastErrorText());

    ExpandedPath = String(ExpandedBuf);

    // If the path is non-existent we assume that this is a relative path...
    if (!DirectoryExists(ExpandedPath))
    {
        // Backup the 'CurrentDir'
        String CurDirBackup = GetCurrentDir();

        __try
        {
            // ...and try to expand it
            SetCurrentDir(ExtractFilePath(ActiveProject->FileName));
            ExpandedPath = ExpandFileName(ExpandedPath);
        }
        __finally
        {
            // Restore the 'CurrentDir'
            SetCurrentDir(CurDirBackup);
        }
    }

    return ExpandedPath;
}
//---------------------------------------------------------------------------

String IDE::GetNormalizedPath(const String& Path)
{
    wchar_t NormalizedPath[32767];

    // Normalize the path to a standard layout
    if (!GetLongPathName(Path.w_str(), NormalizedPath, sizeof(NormalizedPath)))
        throw Exception(Environment::GetWinAPILastErrorText());

    // Return in all lowercase with trailing path delimiter
    return IncludeTrailingPathDelimiter(String(NormalizedPath).LowerCase());
}
//---------------------------------------------------------------------------

String IDE::GetShortPath(const String& Path)
{
    wchar_t ShortPath[MAX_PATH];

    // Retrieve the 8.3 short path name
    if (GetShortPathNameW(Path.w_str(), ShortPath, sizeof(ShortPath)) == 0)
        throw Exception(Environment::GetWinAPILastErrorText());

    return String(ShortPath);
}
//---------------------------------------------------------------------------

String IDE::GetCurrentTargetOS()
{
    String CurrentPlatformToken = GetCurrentPlatform().SubString(1, 3).LowerCase();

    if (CurrentPlatformToken == L"and")
        return L"android";
    if (CurrentPlatformToken == L"ios")
        return L"ios";
    if (CurrentPlatformToken == L"osx")
        return L"osx";
    if (CurrentPlatformToken == L"win")
        return L"windows";

    return L"";
}
//---------------------------------------------------------------------------

String IDE::GetCurrentPlatform()
{
    _di_IOTAProject ActiveProject = GetInterface<_di_IOTAModuleServices>()->GetActiveProject();

    if (ActiveProject)
        return ActiveProject->GetPlatform();
    else
        return L"";
}
//---------------------------------------------------------------------------

int IDE::GetCurrentEditorCppTabWidth()
{
    int TabWidth = 4;

    // Get the registry base path for the editor options, based on the active platform
    String PathKey =
        GetInterface<_di_IOTAServices>()->GetBaseRegistryKey()
            + L"\\Editor\\Source Options\\Borland.EditOptions.C\\";

    // Open the key
    std::unique_ptr<TRegistry> Registry(new TRegistry(KEY_READ));
    Registry->RootKey = HKEY_CURRENT_USER;
    Registry->OpenKey(PathKey, false);

    __try
    {
        // Read the tab width
        TabWidth = Registry->ReadString(L"Tab Stops").ToInt();
    }
    __finally
    {
        Registry->CloseKey();
    }

    return TabWidth;
}
//---------------------------------------------------------------------------

String IDE::GetCurrentEditorFontName()
{
    String FontName = L"Courier New";

    // Get the registry base path for the editor options, based on the active platform
    String PathKey =
        GetInterface<_di_IOTAServices>()->GetBaseRegistryKey()
            + L"\\Editor\\Options\\";

    // Open the key
    std::unique_ptr<TRegistry> Registry(new TRegistry(KEY_READ));
    Registry->RootKey = HKEY_CURRENT_USER;
    Registry->OpenKey(PathKey, false);

    __try
    {
        // Read the editor font name
        FontName = Registry->ReadString(L"Editor Font");
    }
    __finally
    {
        Registry->CloseKey();
    }

    return FontName;
}
//---------------------------------------------------------------------------

int IDE::GetCurrentEditorFontSize()
{
    int FontSize = 10;

    // Get the registry base path for the editor options, based on the active platform
    String PathKey =
        GetInterface<_di_IOTAServices>()->GetBaseRegistryKey()
            + L"\\Editor\\Options\\";

    // Open the key
    std::unique_ptr<TRegistry> Registry(new TRegistry(KEY_READ));
    Registry->RootKey = HKEY_CURRENT_USER;
    Registry->OpenKey(PathKey, false);

    __try
    {
        // Read the font size
        FontSize = Registry->ReadInteger(L"Font Size");
    }
    __finally
    {
        Registry->CloseKey();
    }

    return FontSize;
}
//---------------------------------------------------------------------------

bool IDE::GetCurrentEditorPos(int& Line, int& Column, String& FileName)
{
    Line        = -1;
    Column      = -1;
    FileName    = L"";

    _di_IOTAModuleServices  ModuleServices  = GetInterface<_di_IOTAModuleServices>();
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();
    _di_IOTAModule          Module          = ModuleServices->CurrentModule();
    _di_IOTAEditor          Editor          = Module->CurrentEditor;
    _di_IOTAEditBuffer      Buffer          = EditorServices->TopBuffer;
    _di_IOTASourceEditor    SourceEditor;

    if (Buffer)
    {
        _di_IOTAEditPosition Position = Buffer->EditPosition;

        if (Position)
        {
            Line    = Position->GetRow();
            Column  = Position->GetColumn();
        }
    }

    if (Editor->Supports(SourceEditor))
    {
        // Get the current file name in lower case !!
        FileName = SourceEditor->GetFileName().LowerCase();
    }

    return ((Line != -1) && (Column != -1) && (!FileName.IsEmpty()));
}
//---------------------------------------------------------------------------

_di_IOTAEditActions IDE::GetCurrentEditActions()
{
    _di_IOTAModuleServices  ModuleServices  = GetInterface<_di_IOTAModuleServices>();
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();
    _di_IOTAEditView        EditView        = EditorServices->TopView;
    _di_IOTAEditActions     EditActions;

    if (EditView->Supports(EditActions))
        return EditActions;
    else
        return NULL;
}
//---------------------------------------------------------------------------

RawByteString IDE::GetEditorContentUTF8(_di_IOTASourceEditor SourceEditor)
{
    const int kChunkSize = 1024;

    char Buffer[kChunkSize];
    std::vector<char> RawStr;

    int Pos = 0;
    int BytesRead;

    // Get a reader with the editor content
    _di_IOTAEditReader Reader = SourceEditor->CreateReader();

    // Read ANSI/UTF8 encoded editor content in chunks of kChunkSize
    do
    {
        // Read a chunk
        BytesRead = Reader->GetText(Pos, Buffer, kChunkSize);

        // Append the chunk content to the vector
        for (int k = 0; k < BytesRead; ++k)
        {
            RawStr.push_back(Buffer[k]);
            ++Pos;
        }

    } while (BytesRead == kChunkSize);

    // Add the NULL termination
    RawStr.push_back('\0');

    // Return it as a UTF8-encoded RawByteString
    return &RawStr[0];
}
//---------------------------------------------------------------------------

String IDE::GetEditorContent(_di_IOTASourceEditor SourceEditor)
{
    return UTF8ToString(GetEditorContentUTF8(SourceEditor));
}
//---------------------------------------------------------------------------

void IDE::ExtractAllEditorsContent(
    const String& ProjectPath,
    std::map<String, String>& Results,
    std::map<String, String>* ChangedFiles
    )
{
    if (ChangedFiles)
        ChangedFiles->clear();

    _di_IOTAModuleServices ModuleServices = GetInterface<_di_IOTAModuleServices>();

    // Iterate over all project modules
    for (int i = 0; i < ModuleServices->ModuleCount; ++i)
    {
        // Get the current module
        _di_IOTAModule Module = ModuleServices->Modules[i];

        for (int j = 0; j < Module->ModuleFileCount; ++j)
        {
            _di_IOTAEditor Editor = Module->ModuleFileEditors[j];

            _di_IOTASourceEditor SourceEditor;

            if (Editor->Supports(SourceEditor))
            {
                // Get the current file name in lower case !!
                String FileName = SourceEditor->GetFileName().LowerCase();

                bool IsCpp = Environment::IsCppFile(FileName);
                bool IsHpp = Environment::IsHppFile(FileName);

                if (IsCpp || IsHpp)
                {
                    // Read the whole UTF8 editor content to a RawByteString
                    RawByteString Content = GetEditorContentUTF8(SourceEditor);

                    String ContentFileName;

                    // If we don't have a temporary file name yet
                    if (Results.count(FileName) == 0)
                    {
                        // Create a unique file name
                        ContentFileName =
                            ProjectPath
                            + L"__chbld\\chbld_"
                            + Environment::CreateGuidString()
                            + ExtractFileExt(FileName);
                    }
                    else
                    {
                        // Get the current file name
                        ContentFileName = Results[FileName];
                    }

                    bool Changed = true;

                    // Detect if an existing file differs from the current editor content
                    if (FileExists(ContentFileName))
                    {
                        String EditorHash   = THashMD5::GetHashString(Content);
                        String FileHash     = THashMD5::GetHashStringFromFile(ContentFileName);

                        if (EditorHash == FileHash)
                            Changed = false;
                    }

                    // Only overwrite the file if there really was a change in the editor
                    if (Changed)
                    {
                        if (ChangedFiles)
                        {
                            // Add the file to the 'changed' list
                            (*ChangedFiles)[FileName] = ContentFileName;
                        }

                        // Create a StreamWriter...
                        std::unique_ptr<TStreamWriter> ContentWriter(
                            new TStreamWriter(ContentFileName, false)
                            );

                        // ...and write the content down to the temporary file
                        ContentWriter->Write(Content);

                        // Add the filename/uniquename relationship to the map
                        Results[FileName] = ContentFileName;
                    }
                }
            }
        }
    }
}
//---------------------------------------------------------------------------

void IDE::RemoveOldTempFiles(const String& ProjectPath)
{
    TStringDynArray TempFiles =
        TDirectory::GetFiles(ProjectPath + L"__chbld\\", L"chbld_*");

    for (int i = 0; i < TempFiles.Length; ++i)
    {
        // Don't delete the database...
        if (TempFiles[i].Pos(kDBFileName) == 0)
        {
            // ...but every other temp file
            DeleteFile(TempFiles[i]);
        }
    }
}
//---------------------------------------------------------------------------

bool IDE::GetEditorMousePos(int& Line, int& Column)
{
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();
    _di_IOTAEditView        EditView        = EditorServices->TopView;

    POINT   CursorPos;
    RECT    EditorRect;

    // Get the current screen cursor pos
    GetCursorPos(&CursorPos);

    // Get the editor handle
    HWND EditorHandle = WindowFromPoint(CursorPos);

    if (EditorHandle)
    {
        // Convert the cursor pos to the cursor pos relative to the editor
        ScreenToClient(EditorHandle, &CursorPos);

        // Get the editor rect
        GetWindowRect(EditorHandle, &EditorRect);

        // Get the visible line count
        int LineCount = EditView->BottomRow - EditView->TopRow;

        // Get the line height in pixels
        int LineHeight = (EditorRect.bottom - EditorRect.top) / LineCount;

        // Get the left gutter width in pixels
        int LeftGutterWidth = EditView->Buffer->BufferOptions->LeftGutterWidth;

        // Create a canvas to get the needed widths
        std::unique_ptr<TBitmap> BMP(new TBitmap);

        // Assign the relevant current editor font data
        BMP->Canvas->Font->Name = GetCurrentEditorFontName();
        BMP->Canvas->Font->Size = GetCurrentEditorFontSize();

        // Get the column width in pixels (note: we assume the user uses a fixed-width font)
        int ColumnWidth = BMP->Canvas->TextWidth(L"W");

        // Get the line number width in pixels
        int LineNumberWidth = BMP->Canvas->TextWidth(String(EditView->BottomRow));

        // Calculate the full left border width up to the starting position of the line's content
        // (Note that we add a fixed 16 pixels determined by 'trial and error'
        int LeftBorderWidth = LeftGutterWidth + LineNumberWidth + 16;

        // Get the current line of the mouse cursor
        Line = ((((EditView->TopRow - 1) * LineHeight) + CursorPos.y) / LineHeight) + 1;

        // Get the current column of the mouse cursor
        Column = ((((EditView->LeftColumn - 1) * ColumnWidth) + (CursorPos.x - LeftBorderWidth)) / ColumnWidth) + 1;

        return true;
    }

    Line    = 0;
    Column  = 0;

    return false;
}
//---------------------------------------------------------------------------

bool IDE::GetEditorCaretPixelPos(int& X, int& Y)
{
    // Get the current editor
    TWinControl *Editor = Screen->ActiveControl;

    if (Editor)
    {
        if (Editor->Name == L"Editor")
        {
            // Get the RTTI type of the editor
            TRttiType *Type = TRttiContext().GetType(Editor->ClassInfo());

            // Get the requested properties...
            TRttiProperty *PropertyHandle   = Type->GetProperty(L"Handle");
            TRttiProperty *PropertyX        = Type->GetProperty(L"CaretPixelX");
            TRttiProperty *PropertyY        = Type->GetProperty(L"CaretPixelY");

            // ...and values
            TValue ValueHandle  = PropertyHandle->GetValue(Editor);
            TValue ValueX       = PropertyX->GetValue(Editor);
            TValue ValueY       = PropertyY->GetValue(Editor);

            // Convert the editor handle
            HWND Handle = reinterpret_cast<HWND>(ValueHandle.AsInteger());

            TPoint AbsolutePos;

            // Get the absolute base position of the editor
            if (ClientToScreen(Handle, &AbsolutePos))
            {
                // Calculate the absolute position of the caret
                X = AbsolutePos.X + ValueX.AsInteger();
                Y = AbsolutePos.Y + ValueY.AsInteger();

                return true;
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------

String IDE::GetNormalizedEditorLineText(const int& Line)
{
    _di_IOTAModuleServices  ModuleServices  = GetInterface<_di_IOTAModuleServices>();
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();
    _di_IOTAEditView        EditView        = EditorServices->TopView;
    _di_IOTAModule          Module          = ModuleServices->CurrentModule();
    _di_IOTAEditor          Editor          = Module->CurrentEditor;
    _di_IOTASourceEditor    SourceEditor;

    String LineText = L"";

    if (Editor->Supports(SourceEditor))
    {
        // Create a string list for the editor context
        // There are (for sure) more efficient and simpler methods than this
        // but as this is absolutely not time-critical we'll handle it this way
        std::unique_ptr<TStringList> SL(new TStringList);

        // Get the file content
        SL->Text = GetEditorContent(SourceEditor);

        // Get the current line content
        LineText = SL->Strings[Line-1];
    }

    if (!LineText.IsEmpty())
    {
        String TabReplacement = L"";
        String CppTabWidth = GetCurrentEditorCppTabWidth();

        for (int i = 0; i < CppTabWidth; ++i)
            TabReplacement += L" ";

        // Replace tabs with spaces
        LineText = StringReplace(LineText, L"\t", TabReplacement, TReplaceFlags() << rfReplaceAll);
    }

    return LineText;
}
//---------------------------------------------------------------------------

String IDE::GetSymbolNameAtEditorPos(const int& Line, const int& Column)
{
    // Get the normalized line text
    String LineText = GetNormalizedEditorLineText(Line);

    // Add '//' to the end of each line as the following routines do not work if there is
    // no more content in symbol line directly behind the symbol....#TODO# BUT WORKS AT THE
    // MOMENT
    LineText += L"//";

    wchar_t AllowedChars[] = {
        L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h', L'i', L'j', L'k', L'l', L'm',
        L'n', L'o', L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', L'y', L'z',
        L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J', L'K', L'L', L'M',
        L'N', L'O', L'P', L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', L'Y', L'Z',
        L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'_', L'~'
        };

    std::set<wchar_t> AllowedCharSet(
                        AllowedChars,
                        AllowedChars + (sizeof(AllowedChars) / sizeof(AllowedChars[0]))
                        );

    int StartPos    = Column;
    int EndPos      = Column;

    if ((LineText.Length() > StartPos) && (LineText.Length() > EndPos))
    {
        // Get the symbol start position
        while ((AllowedCharSet.find(LineText[StartPos]) != AllowedCharSet.end()) && (StartPos > 1))
            --StartPos;

        // Finally add 1 to start position
        ++StartPos;

        // Get the symbol end position
        while ((AllowedCharSet.find(LineText[EndPos]) != AllowedCharSet.end()) && (EndPos < LineText.Length()))
            ++EndPos;

        return LineText.SubString(StartPos, EndPos - StartPos);
    }

    return L"";
}
//---------------------------------------------------------------------------

String IDE::GetFunctionNameAtEditorPos(const int& Line, const int& Column, bool ReplaceOperators)
{
    // Get the normalized line text
    String LineText = GetNormalizedEditorLineText(Line);

    wchar_t AllowedChars[] = {
        L'a', L'b', L'c', L'd', L'e', L'f', L'g', L'h', L'i', L'j', L'k', L'l', L'm',
        L'n', L'o', L'p', L'q', L'r', L's', L't', L'u', L'v', L'w', L'x', L'y', L'z',
        L'A', L'B', L'C', L'D', L'E', L'F', L'G', L'H', L'I', L'J', L'K', L'L', L'M',
        L'N', L'O', L'P', L'Q', L'R', L'S', L'T', L'U', L'V', L'W', L'X', L'Y', L'Z',
        L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7', L'8', L'9', L'_', L'~',
        L'.', L'-', L'>', L':'    // <-- With seperators here
        };

    std::set<wchar_t> AllowedCharSet(
                        AllowedChars,
                        AllowedChars + (sizeof(AllowedChars) / sizeof(AllowedChars[0]))
                        );

    bool InSymbolName = false;
    bool AlphaNumCharFound = false;
    String SymbolName = L"";

    for (int i = Column-1; i > 0; --i)
    {
        if (InSymbolName)
        {
            if (LineText[i] == L' ')
            {
                if (AlphaNumCharFound)
                    break;
                else
                    continue;
            }

            if (AllowedCharSet.find(LineText[i]) != AllowedCharSet.end())
            {
                AlphaNumCharFound = true;

                SymbolName = String(LineText[i]) + SymbolName;
            }
        }

        if (LineText[i] == L'(')
            InSymbolName = true;
    }

    if (ReplaceOperators)
    {
        SymbolName = StringReplace(SymbolName, L"->", L".", TReplaceFlags() << rfReplaceAll);
        SymbolName = StringReplace(SymbolName, L"::", L".", TReplaceFlags() << rfReplaceAll);
    }

    return SymbolName;
}
//---------------------------------------------------------------------------

String IDE::GetSymbolNameAtCursorPos()
{
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();
    _di_IOTAEditBuffer      Buffer          = EditorServices->TopBuffer;

    String RetVal = L"";

    int Line;
    int Column;

    if (Buffer)
    {
        _di_IOTAEditPosition Position = Buffer->EditPosition;

        if (Position)
        {
            // Get line and column
            Line    = Position->GetRow();
            Column  = Position->GetColumn();

            RetVal = GetSymbolNameAtEditorPos(Line, Column);
        }
    }

    return RetVal;
}
//---------------------------------------------------------------------------

String IDE::GetSymbolNameAtMousePos()
{
    int Line;
    int Column;

    String RetVal = L"";

    if (GetEditorMousePos(Line, Column))
        RetVal = GetSymbolNameAtEditorPos(Line, Column);

    return RetVal;
}
//---------------------------------------------------------------------------

void IDE::ShowEditorLine(const String& FileName, int Line, int Column)
{
    _di_IOTAModuleServices  ModuleServices  = GetInterface<_di_IOTAModuleServices>();
    _di_IOTAEditorServices  EditorServices  = GetInterface<_di_IOTAEditorServices>();

    // Open the module containing the file (or select it)
    _di_IOTAModule Module = ModuleServices->OpenModule(FileName);

    // Open the file containing the header symbol
    Module->ShowFilename(FileName);

    // Get the Editor
    _di_IOTAEditor Editor = Module->CurrentEditor;

    // Get the EditBuffer
    _di_IOTAEditBuffer Buffer = EditorServices->TopBuffer;

    // Set the cursor line position
    Buffer->EditPosition->GotoLine(Line);

    // Set the cursor column position
    Buffer->EditPosition->MoveRelative(0, Column-1);
}
//---------------------------------------------------------------------------

TOTASortOrder IDE::GetCurrentSymbolListSortOrder()
{
    TOTASortOrder SortOrder = soAlpha;

    // Get the registry base path for the code insight options, based on the active platform
    String PathKey =
        GetInterface<_di_IOTAServices>()->GetBaseRegistryKey()
            + L"\\Code Insight\\Borland.EditOptions.C\\";

    // Open the key
    std::unique_ptr<TRegistry> Registry(new TRegistry(KEY_READ));
    Registry->RootKey = HKEY_CURRENT_USER;
    Registry->OpenKey(PathKey, false);

    __try
    {
        // Read the sort order
        SortOrder = Registry->ReadString(L"Scope Sort") == L"True" ? soScope : soAlpha;
    }
    __finally
    {
        Registry->CloseKey();
    }

    return SortOrder;
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


