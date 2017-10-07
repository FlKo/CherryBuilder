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

#include "cherrybuilder_wizard.h"

#include <memory>

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

__fastcall TChBldWiz::TChBldWiz(_di_IBorlandIDEServices AIDESrv)
    :   FIDESrv(AIDESrv),
        FIDENotifier(NULL),
        FIDENotifierIndex(0),
        FKeyBinder(NULL),
        FKeyBinderIndex(0),
        FCodeInsightManager(NULL),
        FCodeInsightManagerIndex(0)
{
    CS_SEND(L"Wizard::Constructor");

    Init();
}
//---------------------------------------------------------------------------

__fastcall TChBldWiz::~TChBldWiz()
{
    CS_SEND(L"Wizard::Destructor");

    DeInit();
}
//---------------------------------------------------------------------------

void TChBldWiz::Init()
{
    CS_SEND(L"Wizard::Init");

    // Set the IDEServices interface in the 'static' IDE class
    IDE::SetIDEServices(FIDESrv);

    // If the settings file path does not exist, create it
    if (!DirectoryExists(Environment::GetCurrentSettingsFolder(), false))
        ForceDirectories(Environment::GetCurrentSettingsFolder());

    // Create and/or open the settings file
    FSettingsINI = new TMemIniFile(Environment::GetCurrentSettingsFilePath(), TEncoding::UTF8);

    // If settings file is non-existent...
    if (!FileExists(Environment::GetCurrentSettingsFilePath()))
    {
        // ...create the default settings
        SettingsIniDefaultInit();
    }

    // Add the CherryBuilder tools menu entry
    IDE::AddToolsMenuItem(
        L"CherryBuilder Configuration",
        &ToolsMenuClick,
        L"chbld_16"
        );

    // Create and run the analyzer thread
    FAnalyzer = new TChBldAnalyzer(
        FSettingsINI,
        FSettingsINI->ReadString(L"CodeAnalyzer", L"CtagsExe", L""),
        FProjectDB
        );

    // Create CodeInsightManager object
    TChBldCodeInsightManager *CIM = new TChBldCodeInsightManager(FSettingsINI, FProjectDB);

    // Assign the interface
    FCodeInsightManager = CIM;

    // Add it to the IDE
    FCodeInsightManagerIndex =
        IDE::GetInterface<_di_IOTACodeInsightServices>()->AddCodeInsightManager(FCodeInsightManager);

    // Create IDENotifier
    FIDENotifier = new TChBldIDENotifier(FSettingsINI, FAnalyzer, CIM);

    // Add it to the IDE
    FIDENotifierIndex = IDE::GetInterface<_di_IOTAServices>()->AddNotifier(FIDENotifier);

    // Create KeyBinder
    FKeyBinder = new TChBldKeyBinder(CIM);

    // Add it to the IDE
    FKeyBinderIndex = IDE::GetInterface<_di_IOTAKeyboardServices>()->AddKeyboardBinding(FKeyBinder);

    // Load 24x24 about box icon
    HBITMAP AboutBoxBitmap = LoadBitmap(HInstance , L"chbld_24");

    // Get the plugin info text from the settings form (so we have to create an instance here)
    std::unique_ptr<TChBldSettingsForm> SettingsForm(new TChBldSettingsForm(NULL, FSettingsINI));

    String PluginInfoText = SettingsForm->meWizDesc->Text;

    // Add plugin info to the IDE
    FAboutPluginIndex =
        IDE::GetInterface<_di_IOTAAboutBoxServices>()->AddPluginInfo(
            WizTitle,
            PluginInfoText,
            AboutBoxBitmap,
            false,
            WizLicStatus,
            WizASKUName
            );

    if (SplashScreenServices)
    {
        // Load 24x24 splash screen bitmap (same as about box bitmap)
        HBITMAP SplashScreenBitmap = LoadBitmap(HInstance , L"chbld_24");

        // Add plugin bitmap to IDE
        SplashScreenServices->AddPluginBitmap(
                WizTitle,
                SplashScreenBitmap,
                false,
                WizLicStatus
                );
    }
}
//---------------------------------------------------------------------------

void TChBldWiz::DeInit()
{
    CS_SEND(L"Wizard::DeInit");

    // Unregister AboutBox from the IDE
    if (FAboutPluginIndex > 0)
        IDE::GetInterface<_di_IOTAAboutBoxServices>()->RemovePluginInfo(FAboutPluginIndex);

    // Unregister CodeInsightManager from the IDE
    if (FCodeInsightManagerIndex > 0)
        IDE::GetInterface<_di_IOTACodeInsightServices>()->RemoveCodeInsightManager(FCodeInsightManagerIndex);

    // Unregister IDENotifier from the IDE
    if (FIDENotifierIndex > 0)
        IDE::GetInterface<_di_IOTAServices>()->RemoveNotifier(FIDENotifierIndex);

    // Unregister KeyBinder from the IDE
    if (FKeyBinderIndex > 0)
        IDE::GetInterface<_di_IOTAKeyboardServices>()->RemoveKeyboardBinding(FKeyBinderIndex);

    // Delete the analyzer object
    if (FAnalyzer)
    {
        delete FAnalyzer;
        FAnalyzer = NULL;
    }

    // Delete the settings file object
    if (FSettingsINI)
    {
        delete FSettingsINI;
        FSettingsINI = NULL;
    }
}
//---------------------------------------------------------------------------

void TChBldWiz::SettingsIniDefaultInit()
{
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"Active",
        true
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableSymbolHints",
        true
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"ShowPrivateMembers",
        true
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"ShowImplementations",
        false
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableShiftCtrlArrowNavigation",
        true
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableShiftCtrlArrowJumpback",
        true
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"IconSet",
        0
        );

    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"UseIcons",
        true
        );

    FSettingsINI->WriteString(
        L"CodeCompletion",
        L"FontName",
        L"Segoe UI"
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"FontSize",
        9
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"BackgroundColor",
        0x00FFFFFF  // clWhite
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"TextColor",
        0x00000000  // clBlack
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"SymbolColor",
        0x00800000  // clNavy
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"HighlightColor",
        0x00DF0000  // A darker red
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"SelectionBorderColor",
        0x0063C0F1
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"SelectionBackgroundColor",
        0x00CBEFFF
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"SymbolHintBorderColor",
        0x00C8C8C8
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"SymbolHintBackgroundColor",
        0x00F2EFEF
        );


    FSettingsINI->WriteString(
        L"CodeCompletion",
        L"KnownTypeAbbreviations",
        L"System::=	Types::=	Classes::=	TComponentName=UnicodeString	TCaption=UnicodeString"
        );

    FSettingsINI->WriteString(
        L"CodeAnalyzer",
        L"CtagsExe",
        Environment::GetDLLFolder() + L"thirdparty\\ctags.exe"
        );

    FSettingsINI->WriteInteger(
        L"CodeAnalyzer",
        L"SyncInterval",
        1000
        );

    FSettingsINI->WriteInteger(
        L"CodeAnalyzer",
        L"DeepRefreshInterval",
        600000
        );

    // ...and save them
    FSettingsINI->UpdateFile();
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::ToolsMenuClick(TObject* Sender)
{
    std::unique_ptr<TChBldSettingsForm> SettingsForm(new TChBldSettingsForm(NULL, FSettingsINI));

    if (SettingsForm->ShowModal() == mrOk)
        FAnalyzer->UpdateSettings();
}
//---------------------------------------------------------------------------

String __fastcall TChBldWiz::GetIDString()
{
    CS_SEND(L"Wizard::GetIDString");

    return L"FlKo.CherryBuilder";
}
//---------------------------------------------------------------------------

String __fastcall TChBldWiz::GetName()
{
    CS_SEND(L"Wizard::GetName");

    return L"CherryBuilder C++";
}
//---------------------------------------------------------------------------

TWizardState __fastcall TChBldWiz::GetState()
{
	CS_SEND(L"Wizard::GetState");

    return TWizardState() << wsEnabled;
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::Execute()
{
	CS_SEND(L"Wizard::Execute");
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::AfterSave()
{
	CS_SEND(L"Wizard::AfterSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::BeforeSave()
{
	CS_SEND(L"Wizard::BeforeSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::Destroyed()
{
	CS_SEND(L"Wizard::Destroyed");
}
//---------------------------------------------------------------------------

void __fastcall TChBldWiz::Modified()
{
	CS_SEND(L"Wizard::Modified");
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder
