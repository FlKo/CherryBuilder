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

#ifndef cherrybuilder_analyzerH
#define cherrybuilder_analyzerH
//---------------------------------------------------------------------------

#include <System.IniFiles.hpp>

#include <vector>
#include <map>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"
#include "cherrybuilder_ctags.h"
#include "cherrybuilder_projectdb.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class TChBldAnalyzer : public TThread
{
public:
    __fastcall TChBldAnalyzer(
        TMemIniFile* SettingsINI,
        const String& CtagsExePath,
        TChBldProjectDB& ProjectDB
        );

    __fastcall ~TChBldAnalyzer();

    void FullUpdate();
    void UpdateSettings();

private:
    void __fastcall Execute();

    void Parse(
        VString& EditorContentFiles,
        std::map<String, String>& FilenameLookupMap,
        Ctags::VTag& ParsingResults
        );

    void __fastcall SyncSetProjectPathDB();
    void __fastcall SyncEditorsContents();
    void __fastcall SyncSettings();

    TMutex          *FScanningMutex;
    Ctags::TParser  FCtagsParser;
    TChBldProjectDB &FProjectDB;
    bool            FFullUpdate;
    bool            FUpdateSettings;

    TMemIniFile                     *FSettingsINI;
    std::unique_ptr<TMemIniFile>    FLocalSettingsINI;
    std::map<String, String>        FContentFiles;
    std::map<String, String>        FChangedContentFiles;
};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#endif


