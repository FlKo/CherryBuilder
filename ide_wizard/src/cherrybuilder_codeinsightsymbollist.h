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

#ifndef cherrybuilder_codeinsightsymbollistH
#define cherrybuilder_codeinsightsymbollistH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include <vector>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ctags.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class TChBldCodeInsightSymbolList : public TCppInterfacedObject<IOTACodeInsightSymbolList>
{
public:
            __fastcall TChBldCodeInsightSymbolList(bool Sort=false, TOTASortOrder SortOrder=soAlpha);
            __fastcall TChBldCodeInsightSymbolList(TChBldCodeInsightSymbolList& SL);

    virtual __fastcall ~TChBldCodeInsightSymbolList();


    void                                AddSymbol(const Ctags::TTag& Symbol);
    String                              GetLongestItem();

    void                    __fastcall  Clear();
    int                     __fastcall  FindIdent(const String Ident);
    bool                    __fastcall  FindSymIndex(const String Ident, int& Index);
    void                    __fastcall  SetFilter(const String FilterText);

    __property int                      Count                           = {read=GetCount};
    __property bool                     SymbolIsReadWrite[int Index]    = {read=GetSymbolIsReadWrite};
    __property bool                     SymbolIsAbstract[int Index]     = {read=GetSymbolIsAbstract};
    __property TOTAViewerSymbolFlags    SymbolFlags[int i]              = {read=GetViewerSymbolFlags};
    __property int                      SymbolVisibility[int Index]     = {read=GetViewerVisibilityFlags};
    __property TOTAProcDispatchFlags    FuncDispatchFlags[int Index]    = {read=GetProcDispatchFlags};
    __property TOTASortOrder            SortOrder                       = {read=GetSortOrder, write=SetSortOrder};
    __property String                   SymbolText[int Index]           = {read=GetSymbolText};
    __property String                   SymbolTypeText[int Index]       = {read=GetSymbolTypeText};
    __property String                   SymbolClassText[int Index]      = {read=GetSymbolClassText};
    __property String                   FilterText                      = {read=FFilterText};

private:
    int                     __fastcall  GetCount();
    bool                    __fastcall  GetSymbolIsReadWrite(int Index);
    bool                    __fastcall  GetSymbolIsAbstract(int Andex);
    TOTAViewerSymbolFlags   __fastcall  GetViewerSymbolFlags(int Index);
    int                     __fastcall  GetViewerVisibilityFlags(int Index);
    TOTAProcDispatchFlags   __fastcall  GetProcDispatchFlags(int Index);
    TOTASortOrder           __fastcall  GetSortOrder();
    void                    __fastcall  SetSortOrder(const TOTASortOrder Value);
    String                  __fastcall  GetSymbolText(int Index);
    String                  __fastcall  GetSymbolTypeText(int Index);
    String                  __fastcall  GetSymbolClassText(int Index);

    static bool SymbolTextCompare(const Ctags::TTag& X, const Ctags::TTag& Y);
    static bool SymbolVisibilityCompare(const Ctags::TTag& X, const Ctags::TTag& Y);

    static Ctags::VTag     FSymbols;
    static Ctags::VTag     FCompleteSymbols;
    static TOTASortOrder                FSortOrder;
    static String                       FFilterText;

};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#endif

/*

    0: Result := skUnknown;   // vsfUnknown
    1: Result := skConstant;  // vsfConstant
    2: Result := skType;      // vsfType
    3: Result := skVariable;  // vsfVariable
    4: Result := skProcedure; // vsfProcedure
    5: Result := skFunction;  // vsfFunction
    6: Result := skUnit;      // vsfUnit
    7: Result := skLabel;     // vsfLabel
    8: Result := skProperty;  // vsfProperty
    9: Result := skConstructor; // vsfConstructor
    10: Result := skDestructor; // vsfDestructor
    11: Result := skInterface; // vsfInterfac
    12: Result := skEvent;      // vsfEvent

*/



