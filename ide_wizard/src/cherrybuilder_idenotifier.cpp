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

#include "cherrybuilder_idenotifier.h"

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

__fastcall TChBldIDENotifier::TChBldIDENotifier(TMemIniFile* SettingsINI, TChBldAnalyzer* Analyzer, TChBldCodeInsightManager* CodeInsightManager)
    :   FSettingsINI(SettingsINI),
        FCodeInsightManager(CodeInsightManager),
        FAnalyzer(Analyzer)
{
    CS_SEND(L"IDENotifier::Constructor");
}
//---------------------------------------------------------------------------

__fastcall TChBldIDENotifier::~TChBldIDENotifier()
{
    CS_SEND(L"IDENotifier::Destructor");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::FileNotification(
    TOTAFileNotification NotifyCode,
    const String FileName,
    bool& Cancel
    )
{
    switch (NotifyCode)
    {
        case ofnFileOpening:
            CS_SEND(L"IDENotifier::FileNotification:ofnFileOpening(" + FileName + L")");
        break;

        case ofnFileOpened:
            CS_SEND(L"IDENotifier::FileNotification:ofnFileOpened(" + FileName + L")");
        break;

        case ofnFileClosing:
            CS_SEND(L"IDENotifier::FileNotification:ofnFileClosing(" + FileName + L")");
        break;

        case ofnDefaultDesktopLoad:
            CS_SEND(L"IDENotifier::FileNotification:ofnDefaultDesktopLoad(" + FileName + L")");
        break;

        case ofnDefaultDesktopSave:
            CS_SEND(L"IDENotifier::FileNotification:ofnDefaultDesktopSave(" + FileName + L")");
        break;

        case ofnProjectDesktopLoad:
            CS_SEND(L"IDENotifier::FileNotification:ofnProjectDesktopLoad(" + FileName + L")");
        break;

        case ofnProjectDesktopSave:
            CS_SEND(L"IDENotifier::FileNotification:ofnProjectDesktopSave(" + FileName + L")");
        break;

        case ofnPackageInstalled:
            CS_SEND(L"IDENotifier::FileNotification:ofnPackageInstalled(" + FileName + L")");
        break;

        case ofnPackageUninstalled:
            CS_SEND(L"IDENotifier::FileNotification:ofnPackageUninstalled(" + FileName + L")");
        break;

        case ofnActiveProjectChanged:
            CS_SEND(L"IDENotifier::FileNotification:ofnActiveProjectChanged(" + FileName + L")");

            // Initiate a full update in the analyzer
            FAnalyzer->FullUpdate();

        break;

        case ofnProjectOpenedFromTemplate:
            CS_SEND(L"IDENotifier::FileNotification:ofnProjectOpenedFromTemplate(" + FileName + L")");
        break;

        default:
            CS_SEND(L"IDENotifier::FileNotification(" + FileName + L")");
    }
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::BeforeCompile(const _di_IOTAProject Project, bool& Cancel)
{
    CS_SEND(L"IDENotifier::BeforeCompile(_di_IOTAProject, bool&)");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::AfterCompile(bool Succeeded)
{
    CS_SEND(L"IDENotifier::AfterCompile(bool)");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::BeforeCompile(
    const _di_IOTAProject Project,
    bool IsCodeInsight,
    bool& Cancel
    )
{
    CS_SEND(L"IDENotifier::BeforeCompile(_di_IOTAProject, bool, bool&)");

    int Line;
    int Column;
    String FileName;

    // Get the current line, column and file name
    IDE::GetCurrentEditorPos(Line, Column, FileName);

    // If we have a supported file deactivate built-in code insight compiling
    // when CherryBuilder is active
    if (IsCodeInsight)
    {
        if (FSettingsINI->ReadBool(L"CodeCompletion", L"Active", true))
        {
            if (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName))
            {
                // Check if...

                // ...invocation is a browsing request
                if (FCodeInsightManager->InvocationData.InvocationKey == L'\x02')
                {
                    // Handle the browsing request
                    FCodeInsightManager->PerformBrowsingRequest();
                }
                // ...invocation is a symbol hint request
                else if (FCodeInsightManager->InvocationData.InvocationKey == L'\x03')
                {
                    if (FSettingsINI->ReadBool(L"CodeCompletion", L"EnableSymbolHints", true))
                    {
                        // Handle the symbol hint request
                        FCodeInsightManager->PerformSymbolHintRequest();
                    }
                }
                // ...invocation is a parameter hint request
                else if (FCodeInsightManager->InvocationData.InvocationKey == L'\x01')
                {
                    FCodeInsightManager->PerformParameterHintRequest();
                }


                // Prevent the compiler from being invoked
                Cancel = true;
            }
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::AfterCompile(bool Succeeded, bool IsCodeInsight)
{
    CS_SEND(L"IDENotifier::AfterCompile(bool, bool)");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::AfterCompile(
    const _di_IOTAProject Project,
    bool Succeeded,
    bool IsCodeInsight
    )
{
    CS_SEND(L"IDENotifier::AfterCompile(_di_IOTAProject, bool, bool)");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::AfterSave()
{
    CS_SEND(L"IDENotifier::AfterSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::BeforeSave()
{
    CS_SEND(L"IDENotifier::BeforeSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::Destroyed()
{
    CS_SEND(L"IDENotifier::Destroyed");
}
//---------------------------------------------------------------------------

void __fastcall TChBldIDENotifier::Modified()
{
    CS_SEND(L"IDENotifier::Modified");
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder
