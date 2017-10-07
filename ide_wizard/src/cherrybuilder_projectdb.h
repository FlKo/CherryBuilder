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

#ifndef cherrybuilder_projectdbH
#define cherrybuilder_projectdbH
//---------------------------------------------------------------------------

#include <vector>
#include <map>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_sqlite.h"
#include "cherrybuilder_ctags.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class TChBldProjectDB
{
public:
    TChBldProjectDB();
    ~TChBldProjectDB();

    void Refresh(
        const Ctags::VTag& Tags,
        bool DeepRefresh=false,
        bool DeleteFilesContent=false,
        const std::map<String, String>* ChangedContentFiles=NULL
        );

    void GetMatchingIdentifierList(const String& Query, Ctags::VTag& List, bool ClearList=true);

    void GetPosNamespaces(
        const String& FileName, const int Line, const int Column, VString& Namespaces
        );

    void GetPosClassesAndStructs(
        const String& FileName, const int Line, const int Column, VString& Classes
        );

    Ctags::TTag GetPosSymbol(const String& SymbolText);

    Ctags::TTag GetPosImplementation(const String& FileName, const int Line, const int Column);
    Ctags::TTag GetPosHeader(const String& FileName, const int Line, const int Column);

    Ctags::TTag GetPosHeaderTarget(Ctags::TTag& Symbol);
    Ctags::TTag GetPosImplementationTarget(Ctags::TTag& Symbol);

    __property String ProjectPath = {read=FProjectPath, write=SetProjectPath};
    __property TMutex* UpdateMutex = {read= FUpdateMutex};

private:

    void CreateWorkingDirIfRequired();

    void SetProjectPath(const String& AProjectPath);

    void ChangeProjectContext();

    String FProjectPath;

    TMutex *FUpdateMutex;
};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#endif
