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

#include "cherrybuilder_settingsform.h"

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"

using namespace Cherrybuilder;

__fastcall TChBldSettingsForm::TChBldSettingsForm(
    TComponent* Owner,
    TMemIniFile* SettingsINI
    )
    :   TForm(Owner),
        FSettingsINI(SettingsINI)
{
    // Hide the tabs of the settings pages and add the captions as nodes to
    // the TreeView, so they can be accessed by selecting those nodes
    for (int i = 0; i< pcSelection->PageCount; ++i)
    {
        pcSelection->Pages[i]->TabVisible = false;
        tvSelection->Items->Add(NULL, pcSelection->Pages[i]->Caption);
    }

    // Get Ctags version
    meCtagsVersion->Lines->Text = GetCtagsVersionString();

    // Set focus to first TreeView entry
    tvSelection->Selected = tvSelection->TopItem;

    edAnalyzerSyncInterval->Tag         = reinterpret_cast<int>(udAnalyzerSyncInterval);
    edAnalyzerDeepRefreshInterval->Tag  = reinterpret_cast<int>(udAnalyzerDeepRefreshInterval);

    // Load the settings values
    LoadSettings();
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::btnOKClick(TObject *Sender)
{
    // Save the settings
    SaveSettings();

    ModalResult = mrOk;
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::btnCancelClick(TObject *Sender)
{
    ModalResult = mrCancel;
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::tvSelectionChange(TObject *Sender, TTreeNode *Node)
{
    // Show corresponding settings page to selection
    pcSelection->ActivePageIndex = Node->Index;
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::FormKeyPress(TObject *Sender, System::WideChar &Key)
{
    // If 'escape' was pressed, close the window
    if (Key == VK_ESCAPE)
        Close();
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::edAnalyzerIntervalExit(TObject *Sender)
{
    TEdit *Ed   = dynamic_cast<TEdit*>(Sender);
    TUpDown *Ud = reinterpret_cast<TUpDown*>(Ed->Tag);
    
    // Check lower bounds
    if (Ed->Text.ToInt() < Ud->Min)
       Ed->Text = String(Ud->Min);
       
    // Check upper bounds
    if (Ed->Text.ToInt() > Ud->Max)
       Ed->Text = String(Ud->Max);    
}
//---------------------------------------------------------------------------

String TChBldSettingsForm::GetCtagsVersionString()
{
    // Get the Ctags executable path
    String CtagsExe = IDE::GetShortPath(FSettingsINI->ReadString(L"CodeAnalyzer", L"CtagsExe", L""));

    // Concatenate the temp file path fpr Ctags version info
    String CtagsVersionInfoFile = Environment::GetWinTempPath() + L"ctagsversion.txt";  // 8.3 path

    String Version = L"";

    __try
    {
        // Build the command line
        String CommandLine = L"cmd.exe /C \"" + CtagsExe + L" --version > " + CtagsVersionInfoFile + L"\"";

        // Extract the ctags version info
        Environment::ExecuteCmd(CommandLine);

        if (FileExists(CtagsVersionInfoFile))
        {
            // Create a StreamReader
            std::unique_ptr<TStreamReader> VersionInfoFile(new TStreamReader(CtagsVersionInfoFile));

            // Get each line of the version info output file
            while (!VersionInfoFile->EndOfStream)
                Version += VersionInfoFile->ReadLine() + L"\r\n";
        }
    }
    __finally
    {
        DeleteFile(CtagsVersionInfoFile);
    }

    return Version;
}
//---------------------------------------------------------------------------

void TChBldSettingsForm::LoadSettings()
{
    // Code completion

    // Activation
    cbCompletionActive->Checked = 
        FSettingsINI->ReadBool(L"CodeCompletion", L"Active", true);

    // Enable symbol hints
    cbEnableSymbolHints->Checked =
        FSettingsINI->ReadBool(L"CodeCompletion", L"EnableSymbolHints", true);

    // Show private members
    cbShowPrivateMembers->Checked =
        FSettingsINI->ReadBool(L"CodeCompletion", L"ShowPrivateMembers", true);

    // Show implementations
    cbShowImplementations->Checked =
        FSettingsINI->ReadBool(L"CodeCompletion", L"ShowImplementations", false);

    // Enable Shift+Ctrl+Arrow key navigation
    cbEnableShiftCtrlArrowNavigation->Checked =
        FSettingsINI->ReadBool(L"CodeCompletion", L"EnableShiftCtrlArrowNavigation", true);

    // Enable Shift+Ctrl+Arrow jumpback
    cbEnableShiftCtrlArrowJumpback->Checked =
        FSettingsINI->ReadBool(L"CodeCompletioN", L"EnableShiftCtrlArrowJumpback", true);

    // Icons
    cmbIconSet->ItemIndex = FSettingsINI->ReadInteger(L"CodeCompletion", L"IconSet", 0);
    cbUseIcons->Checked = FSettingsINI->ReadBool(L"CodeCompletion", L"UseIcons", true);

    // Font
    cmbFontNames->Items->Assign(Screen->Fonts);
    cmbFontNames->ItemIndex = cmbFontNames->Items->IndexOf(
        FSettingsINI->ReadString(L"CodeCompletion", L"FontName", L"")
        );

    cmbFontSizes->ItemIndex = cmbFontSizes->Items->IndexOf(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"FontSize", 10)
        );

    // Colors
    pnlBackgroundColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"BackgroundColor", 0));

    pnlTextColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"TextColor", 0));

    pnlSymbolColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolColor", 0));

    pnlHighlightColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"HighlightColor", 0));

    pnlSelectionBorderColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"SelectionBorderColor", 0));

    pnlSelectionBackgroundColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"SelectionBackgroundColor", 0));

    pnlSymbolHintBorderColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBorderColor", 0));

    pnlSymbolHintBackgroundColor->Color =
        static_cast<TColor>(FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBackgroundColor", 0));

    // Known Type Abbreviations
    meTypeAbbreviations->Text = StringReplace(
                                    FSettingsINI->ReadString(
                                        L"CodeCompletion",
                                        L"KnownTypeAbbreviations",
                                        L""
                                        ),
                                    L"\t",
                                    L"\r\n",
                                    TReplaceFlags() << rfReplaceAll
                                    );

        

    // Code analyzer
    
    udAnalyzerSyncInterval->Position =
        FSettingsINI->ReadInteger(L"CodeAnalyzer", L"SyncInterval", 2000);

    udAnalyzerDeepRefreshInterval->Position =
        FSettingsINI->ReadInteger(L"CodeAnalyzer", L"DeepRefreshInterval", 600000) / 1000;

    edAnalyzerCtagsExecutable->Text =
        FSettingsINI->ReadString(L"CodeAnalyzer", L"CtagsExe", L"");
}
//---------------------------------------------------------------------------

void TChBldSettingsForm::SaveSettings()
{
    // Code completion

    // Activation
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"Active",
        cbCompletionActive->Checked
        );

    // Enable symbol hints
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableSymbolHints",
        cbEnableSymbolHints->Checked
        );

    // Show private members
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"ShowPrivateMembers",
        cbShowPrivateMembers->Checked
        );

    // Show implementations
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"ShowImplementations",
        cbShowImplementations->Checked
        );

    // Enable Shift+Ctrl+Arrow key navigation
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableShiftCtrlArrowNavigation",
        cbEnableShiftCtrlArrowNavigation->Checked
        );

    // Enable Shift+Ctrl+Arrow jumpback
    FSettingsINI->WriteBool(
        L"CodeCompletion",
        L"EnableShiftCtrlArrowJumpback",
        cbEnableShiftCtrlArrowJumpback->Checked
        );

    // Icons
    FSettingsINI->WriteInteger(L"CodeCompletion", L"IconSet", cmbIconSet->ItemIndex);
    FSettingsINI->WriteBool(L"CodeCompletion", L"UseIcons", cbUseIcons->Checked);

    // Font
    FSettingsINI->WriteString(
        L"CodeCompletion", 
        L"FontName", 
        cmbFontNames->Items->Strings[cmbFontNames->ItemIndex]
        );

    FSettingsINI->WriteInteger(
        L"CodeCompletion",
        L"FontSize", 
        cmbFontSizes->Items->Strings[cmbFontSizes->ItemIndex].ToInt()
        );

    // Colors
    FSettingsINI->WriteInteger(L"CodeCompletion", L"BackgroundColor", pnlBackgroundColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"TextColor", pnlTextColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"SymbolColor", pnlSymbolColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"HighlightColor", pnlHighlightColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"SelectionBorderColor", pnlSelectionBorderColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"SelectionBackgroundColor", pnlSelectionBackgroundColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"SymbolHintBorderColor", pnlSymbolHintBorderColor->Color);
    FSettingsINI->WriteInteger(L"CodeCompletion", L"SymbolHintBackgroundColor", pnlSymbolHintBackgroundColor->Color);

    // Known Type Abbreviations
    String TypeAbbrev = StringReplace(meTypeAbbreviations->Text, L"\r\n", L"\t", TReplaceFlags() << rfReplaceAll);

    FSettingsINI->WriteString(
        L"CodeCompletion",
        L"KnownTypeAbbreviations",
        TypeAbbrev
        );
    
    // Code analyzer
    
    FSettingsINI->WriteInteger(
        L"CodeAnalyzer", 
        L"SyncInterval", 
        edAnalyzerSyncInterval->Text.ToInt()
        );

    FSettingsINI->WriteInteger(
        L"CodeAnalyzer",
        L"DeepRefreshInterval",
        edAnalyzerDeepRefreshInterval->Text.ToInt() * 1000
        );
        
    FSettingsINI->WriteString(
        L"CodeAnalyzer", 
        L"CtagsExe", 
        edAnalyzerCtagsExecutable->Text
        );

    // Update the settings file
    FSettingsINI->UpdateFile();
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::aeConfigIdle(TObject *Sender, bool &Done)
{
    bool EnabledState = cbCompletionActive->Checked;

    SetChildControlsEnabledState(grbAppearance, EnabledState);
}
//---------------------------------------------------------------------------

void __fastcall TChBldSettingsForm::pnlColorClick(TObject *Sender)
{
    TPanel *Pnl = dynamic_cast<TPanel*>(Sender);

    // Create a color dialog
    std::unique_ptr<TColorDialog> CD(new TColorDialog(this));

    // Tell the dialog to open in 'full open' state
    CD->Options = CD->Options << cdFullOpen;

    // Assign the current panel color to the color dialog
    CD->Color = Pnl->Color;

    // If the user has confirmed a new color selection...
    if (CD->Execute())
    {
        // ...assign this color to the panel
        Pnl->Color = CD->Color;
    }
}
//---------------------------------------------------------------------------

void TChBldSettingsForm::SetChildControlsEnabledState(TWinControl* AParent, bool AEnabled)
{
    if (AParent)
    {
        for (int i = 0; i < AParent->ControlCount; ++i)
        {
            if (AParent->Controls[i]->ClassNameIs(L"TGroupBox") || AParent->Controls[i]->ClassNameIs(L"TPanel"))
                SetChildControlsEnabledState(dynamic_cast<TWinControl*>(AParent->Controls[i]), AEnabled);

            AParent->Controls[i]->Enabled = AEnabled;
        }
    }
}
//---------------------------------------------------------------------------

