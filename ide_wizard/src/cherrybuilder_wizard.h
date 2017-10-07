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

#ifndef cherrybuilder_wizardH
#define cherrybuilder_wizardH
//---------------------------------------------------------------------------

#include <System.IniFiles.hpp>

#include <ToolsAPI.hpp>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"
#include "cherrybuilder_analyzer.h"
#include "cherrybuilder_idenotifier.h"
#include "cherrybuilder_keybinder.h"
#include "cherrybuilder_codeinsightmanager.h"
#include "cherrybuilder_settingsform.h"
#include "cherrybuilder_projectdb.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

const String WizTitle =
    L"CherryBuilder - The Productivity Extension for C++Builder®";

const String WizLicStatus =
    L"Early Showcase Edition - Expiration date: 2017/08/31";

const String WizASKUName =
    L"SKU Build";
//---------------------------------------------------------------------------

class PACKAGE TChBldWiz : public TCppInterfacedObject<IOTANotifier, IOTAWizard>
{
public:
    __fastcall TChBldWiz(_di_IBorlandIDEServices AIDESrv);
    __fastcall ~TChBldWiz();

private:
    void Init();
    void DeInit();

    void SettingsIniDefaultInit();

    void __fastcall ToolsMenuClick(TObject* Sender);

    _di_IBorlandIDEServices     FIDESrv;

    _di_IOTAIDENotifier80       FIDENotifier;
    int                         FIDENotifierIndex;

    _di_IOTAKeyboardBinding     FKeyBinder;
    int                         FKeyBinderIndex;

    _di_IOTACodeInsightManager  FCodeInsightManager;
    int                         FCodeInsightManagerIndex;

    int                         FAboutPluginIndex;

    TChBldProjectDB             FProjectDB;
    TMemIniFile                 *FSettingsINI;
    TChBldAnalyzer              *FAnalyzer;

    /* IOTAWizard */
    String          __fastcall GetIDString();
    String          __fastcall GetName();
    TWizardState    __fastcall GetState();
    void            __fastcall Execute();

    /* IOTANotifier */
    void __fastcall AfterSave();
    void __fastcall BeforeSave();
    void __fastcall Destroyed();
    void __fastcall Modified();
};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


#endif
