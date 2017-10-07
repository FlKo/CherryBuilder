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

#ifndef cherrybuilder_settingsformH
#define cherrybuilder_settingsformH

#include <System.Classes.hpp>
#include <System.IniFiles.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.CheckLst.hpp>
#include <Vcl.AppEvnts.hpp>

#include <memory>
//---------------------------------------------------------------------------

class TChBldSettingsForm : public TForm
{
__published:
    TPanel *Panel1;
    TBevel *Bevel1;
    TButton *btnCancel;
    TButton *btnOK;
    TTreeView *tvSelection;
    TPageControl *pcSelection;
    TTabSheet *tsAcknowledgements;
    TGroupBox *GroupBox3;
    TTabSheet *TabSheet1;
    TGroupBox *GroupBox4;
    TPanel *Panel3;
    TTabSheet *TabSheet2;
    TTabSheet *TabSheet3;
    TGroupBox *GroupBox1;
    TGroupBox *GroupBox5;
    TSplitter *Splitter1;
    TGroupBox *GroupBox2;
    TEdit *edAnalyzerCtagsExecutable;
    TLabel *Label1;
    TGroupBox *GroupBox6;
    TLabel *Label2;
    TEdit *edAnalyzerSyncInterval;
    TUpDown *udAnalyzerSyncInterval;
    TUpDown *udAnalyzerDeepRefreshInterval;
    TEdit *edAnalyzerDeepRefreshInterval;
    TLabel *Label3;
    TCheckBox *cbCompletionActive;
    TApplicationEvents *aeConfig;
    TPanel *Panel2;
    TImage *Image1;
    TBevel *Bevel2;
    TLabel *Label12;
    TLabel *Label13;
    TGroupBox *grbAppearance;
    TGroupBox *GroupBox8;
    TLabel *Label11;
    TComboBox *cmbIconSet;
    TCheckBox *cbUseIcons;
    TGroupBox *grbFont;
    TLabel *Label5;
    TLabel *Label10;
    TComboBox *cmbFontNames;
    TComboBox *cmbFontSizes;
    TLabel *Label4;
    TGroupBox *GroupBox7;
    TLabel *Label9;
    TLabel *Label8;
    TLabel *Label7;
    TLabel *Label6;
    TLabel *Label14;
    TLabel *Label15;
    TPanel *pnlBackgroundColor;
    TPanel *pnlTextColor;
    TPanel *pnlSymbolColor;
    TPanel *pnlHighlightColor;
    TPanel *pnlSelectionBorderColor;
    TPanel *pnlSelectionBackgroundColor;
    TTabSheet *tsTypeAbbreviations;
    TGroupBox *GroupBox10;
    TMemo *meTypeAbbreviations;
    TLabel *Label16;
    TLabel *Label17;
    TPanel *pnlSymbolHintBorderColor;
    TPanel *pnlSymbolHintBackgroundColor;
    TMemo *meWizDesc;
    TGroupBox *GroupBox9;
    TCheckBox *cbEnableSymbolHints;
    TCheckBox *cbShowPrivateMembers;
    TCheckBox *cbShowImplementations;
    TCheckBox *cbEnableShiftCtrlArrowNavigation;
    TMemo *meCtagsVersion;
    TCheckBox *cbEnableShiftCtrlArrowJumpback;
    void __fastcall btnOKClick(TObject *Sender);
    void __fastcall btnCancelClick(TObject *Sender);
    void __fastcall tvSelectionChange(TObject *Sender, TTreeNode *Node);
    void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
    void __fastcall edAnalyzerIntervalExit(TObject *Sender);
    void __fastcall aeConfigIdle(TObject *Sender, bool &Done);
    void __fastcall pnlColorClick(TObject *Sender);

public:
    __fastcall TChBldSettingsForm(TComponent* Owner, TMemIniFile* SettingsINI);

private:
    String GetCtagsVersionString();

    void LoadSettings();
    void SaveSettings();

    void SetChildControlsEnabledState(TWinControl* AParent, bool AEnabled);

    TMemIniFile *FSettingsINI;
};

#endif
