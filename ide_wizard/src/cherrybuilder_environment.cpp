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

#include "cherrybuilder_environment.h"

#include <memory>

#include <Lmcons.h> // for 'UNLEN'
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

const String Environment::FProgramDataFolderName = L"CherryBuilder for Embarcadero RAD Studio";
const String Environment::FSettingsFileExtension = L".cfg";
//---------------------------------------------------------------------------

String Environment::GetCurrentComputerName()
{
    // Create array for the computer name
    wchar_t ComputerName[MAX_COMPUTERNAME_LENGTH+1];

    // Set the maximum computer name length
    unsigned long ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;

    // Get the current computer name
    GetComputerNameW(ComputerName, &ComputerNameLength);

    // Return it as UnicodeString
    return String(ComputerName);
}
//---------------------------------------------------------------------------

String Environment::GetCurrentUserName()
{
    // Create array for the user name
    wchar_t UserName[UNLEN+1];

    // Set the maximum user name length
    unsigned long UserNameLength = UNLEN + 1;

    // Get the current user name
    GetUserNameW(UserName, &UserNameLength);

    // Return it as UnicodeString
    return String(UserName);
}
//---------------------------------------------------------------------------

String Environment::GetCurrentSettingsFolder()
{
    // Join the settings file path
    return  GetEnvironmentVariable(L"ALLUSERSPROFILE")
                + L"\\"
                + FProgramDataFolderName
                + L"\\"
                + GetEnvironmentVariable(L"ProductVersion")
                + L"\\";
}
//---------------------------------------------------------------------------

String Environment::GetCurrentSettingsFileName()
{
    // Build the settings file name from the current user name
    return  StringReplace(
                String(GetCurrentUserName()).LowerCase(),
                L" ",
                L"_",
                TReplaceFlags() << rfReplaceAll
                )
            + FSettingsFileExtension;
}
//---------------------------------------------------------------------------

String Environment::GetCurrentSettingsFilePath()
{
    // Concatenate the settings folder and file name
    return GetCurrentSettingsFolder() + GetCurrentSettingsFileName();
}
//---------------------------------------------------------------------------

String Environment::GetDLLFolder()
{
    wchar_t ModuleFilePath[MAX_PATH];

    // Read current module absolute path
    if (GetModuleFileName(HInstance, ModuleFilePath, MAX_PATH) == 0)
        throw Exception(GetWinAPILastErrorText());

    // Return the containing folder path
    return IncludeTrailingPathDelimiter(ExtractFilePath(String(ModuleFilePath)));
}
//---------------------------------------------------------------------------

String Environment::CreateGuidString(bool NoParantheses)
{
    TGUID NewGUID;
    CreateGUID(NewGUID);

    if (!NoParantheses)
    {
        return GUIDToString(NewGUID);
    }
    else
    {
        return StringReplace(
            StringReplace(
                GUIDToString(NewGUID),
                L"{",
                L"",
                TReplaceFlags() << rfReplaceAll
                ),
            L"}",
            L"",
            TReplaceFlags() << rfReplaceAll
            );
    }
}
//---------------------------------------------------------------------------

void Environment::ExecuteCmd(const String& CommandLine)
{
    // Create a STARTUPINFO structure and fill it with the necessary content
    STARTUPINFO StartupInfo;
    memset(&StartupInfo, 0, sizeof(STARTUPINFO));

    StartupInfo.cb          = sizeof(STARTUPINFO);
    StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;

    // Create a PROCESS_INFORMATION structure
    PROCESS_INFORMATION ProcessInfo;
    memset(&ProcessInfo, 0, sizeof(PROCESS_INFORMATION));

    __try
    {
        // Run command
        if (!CreateProcessW(
                NULL,
                CommandLine.w_str(),
                NULL,
                NULL,
                FALSE,
                CREATE_NEW_CONSOLE,
                NULL,
                NULL,
                &StartupInfo,
                &ProcessInfo)
                )
        {
            throw Exception(Environment::GetWinAPILastErrorText());
        }

        // Wait for command to finish
        WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
    }
    __finally
    {
        // Close the handles
        CloseHandle(ProcessInfo.hThread);
        CloseHandle(ProcessInfo.hProcess);
    }
}
//---------------------------------------------------------------------------

String Environment::GetWinTempPath()
{
    return IncludeTrailingPathDelimiter(GetEnvironmentVariable(L"TEMP"));
}
//---------------------------------------------------------------------------

String Environment::GetWinAPILastErrorText()
{
	String ErrorTextStr;
	LPWSTR ErrorText = NULL;

	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM
		| FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&ErrorText,
		0,
		NULL
	);

	if (ErrorText)
    {
		// Store error text in string
		ErrorTextStr = ErrorText;

		// Release memory allocated by FormatMessage()
		LocalFree(ErrorText);
		ErrorText = NULL;
	}

	return ErrorTextStr;
}
//---------------------------------------------------------------------------

bool Environment::IsCppFile(const String& File)
{
    String FileExt = ExtractFileExt(File);

    if (    (FileExt == L".c")
        ||  (FileExt == L".cpp")
        ||  (FileExt == L".cc")
        ||  (FileExt == L".cxx")
        ||  (FileExt == L".cp")
        ||  (FileExt == L".c++")
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------

bool Environment::IsHppFile(const String& File)
{
    String FileExt = ExtractFileExt(File);

    if (    (FileExt == L".h")
        ||  (FileExt == L".hpp")
        ||  (FileExt == L".hh")
        ||  (FileExt == L".hp")
        ||  (FileExt == L".h++")
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//---------------------------------------------------------------------------

String Environment::ExtractToken(const String& Text)
{
    // Replace all invocation operators with '.'
    String ProcessedText = StringReplace(Text, L"->", L".", TReplaceFlags() << rfReplaceAll);
    ProcessedText = StringReplace(ProcessedText, L"::", L".", TReplaceFlags() << rfReplaceAll);

    // Extract the token
    VString TK = SplitStr(ProcessedText, L'.');

    if (TK.size() > 0)
        return TK[TK.size()-1];

    return L"";
}
//---------------------------------------------------------------------------

VString Environment::SplitStr(String Text, wchar_t Delim1, wchar_t Delim2, wchar_t Delim3)
{
    VString Splitteds;

    if (Delim2 != L'\0')
        Text = StringReplace(Text, Delim2, Delim1, TReplaceFlags() << rfReplaceAll);

    if (Delim3 != L'\0')
        Text = StringReplace(Text, Delim3, Delim1, TReplaceFlags() << rfReplaceAll);

    // Create a string list to extract the actual token
    std::unique_ptr<TStringList> SL(new TStringList);

    SL->Delimiter       = Delim1;
    SL->StrictDelimiter = true;
    SL->DelimitedText   = Text;

    for (int i = 0; i < SL->Count; ++i)
        Splitteds.push_back(SL->Strings[i]);

    return Splitteds;
}
//---------------------------------------------------------------------------

std::pair<String, String> Environment::SplitToPair(String Text, wchar_t Delim)
{
    // Create a string list to split
    std::unique_ptr<TStringList> SL(new TStringList);

    SL->Delimiter       = Delim;
    SL->StrictDelimiter = true;
    SL->DelimitedText   = Text;

    if (SL->Count >= 2)
        return std::make_pair<String, String>(SL->Strings[0], SL->Strings[1]);
    else if (SL->Count == 1)
        return std::make_pair<String, String>(SL->Strings[0], L"");
    else
        return std::make_pair<String, String>(L"", L"");
}
//---------------------------------------------------------------------------

VString Environment::SplitCtagsObjectAncestors(String Text)
{
    VString Splitteds;

    int AngleBrackets = 0;

    for (int i = 1; i <= Text.Length(); ++i)
    {
        if (Text[i] == L'<')
        {
            ++AngleBrackets;
        }
        else if (Text[i] == L'>')
        {
            if (AngleBrackets > 0)
                --AngleBrackets;
        }

        if (Text[i] == L',')
        {
            if (AngleBrackets > 0)
                Text[i] = L'\xFFFF';
        }
    }

    // Create a string list to split
    std::unique_ptr<TStringList> SL(new TStringList);

    SL->Delimiter       = L',';
    SL->StrictDelimiter = true;
    SL->DelimitedText   = Text;
    SL->Text            = StringReplace(SL->Text, L"\xFFFF", L",", TReplaceFlags() << rfReplaceAll);

    for (int i = 0; i < SL->Count; ++i)
        Splitteds.push_back(SL->Strings[i]);

    return Splitteds;
}
//---------------------------------------------------------------------------

TColor Environment::GetLighterColor(const TColor& Color, const unsigned char BrightnessDiff)
{
    unsigned int LighterR = GetRValue(Color) + BrightnessDiff;
    unsigned int LighterG = GetGValue(Color) + BrightnessDiff;
    unsigned int LighterB = GetBValue(Color) + BrightnessDiff;

    if (LighterR > 200)
        LighterR = 200;

    if (LighterG > 200)
        LighterG = 200;

    if (LighterB > 200)
        LighterB = 200;

    return static_cast<TColor>(RGB(LighterR, LighterG, LighterB));
}
//---------------------------------------------------------------------------

TColor Environment::GetDarkerColor(const TColor& Color, const unsigned char BrightnessDiff)
{
    int DarkerR = GetRValue(Color) - BrightnessDiff;
    int DarkerG = GetGValue(Color) - BrightnessDiff;
    int DarkerB = GetBValue(Color) - BrightnessDiff;

    if (DarkerR < 55)
        DarkerR = 55;

    if (DarkerG < 55)
        DarkerG = 55;

    if (DarkerB < 55)
        DarkerB = 55;

    return static_cast<TColor>(RGB(DarkerR, DarkerG, DarkerB));
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#pragma package(smart_init)


