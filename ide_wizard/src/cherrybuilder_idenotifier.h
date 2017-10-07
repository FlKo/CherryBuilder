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


#ifndef cherrybuilder_idenotifierH
#define cherrybuilder_idenotifierH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include "cherrybuilder_analyzer.h"
#include "cherrybuilder_codeinsightmanager.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class TChBldIDENotifier : public TCppInterfacedObject<IOTAIDENotifier, IOTAIDENotifier50, IOTAIDENotifier80>
{
public:
    __fastcall TChBldIDENotifier(
        TMemIniFile* SettingsINI,
        TChBldAnalyzer* Analyzer,
        TChBldCodeInsightManager* CodeInsightManager
        );

    __fastcall ~TChBldIDENotifier();

private:

    /* IOTAIDENotifier */
	void __fastcall FileNotification(
        TOTAFileNotification NotifyCode,
        const String FileName,
        bool& Cancel
        );

	void __fastcall BeforeCompile(const _di_IOTAProject Project, bool& Cancel);
	void __fastcall AfterCompile(bool Succeeded);

    /* IOTAIDENotifier50 */
	void __fastcall BeforeCompile(const _di_IOTAProject Project, bool IsCodeInsight, bool& Cancel);
	void __fastcall AfterCompile(bool Succeeded, bool IsCodeInsight);

    /* IOTAIDENotifier80 */
	void __fastcall AfterCompile(const _di_IOTAProject Project, bool Succeeded, bool IsCodeInsight);

    /* IOTANotifier */
    void __fastcall AfterSave();
    void __fastcall BeforeSave();
    void __fastcall Destroyed();
    void __fastcall Modified();

    TChBldAnalyzer              *FAnalyzer;
    TChBldCodeInsightManager    *FCodeInsightManager;
    TMemIniFile                 *FSettingsINI;
};


} // namespace Cherrybuilder

#endif
