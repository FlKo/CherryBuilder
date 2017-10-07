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

#include "cherrybuilder_codeinsightsymbollist.h"

#include <System.StrUtils.hpp>

#include <algorithm> // for std::sort

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{
Ctags::VTag    TChBldCodeInsightSymbolList::FSymbols;
Ctags::VTag    TChBldCodeInsightSymbolList::FCompleteSymbols;
TOTASortOrder               TChBldCodeInsightSymbolList::FSortOrder = soAlpha;
String                      TChBldCodeInsightSymbolList::FFilterText = L"";
//---------------------------------------------------------------------------

__fastcall TChBldCodeInsightSymbolList::TChBldCodeInsightSymbolList(bool Sort, TOTASortOrder SortOrder)
{
    //CS_SEND(L"SymbolList::Constructor");

    // Apply the current sorting if requested
    if (Sort)
    {
        // Sort order is 'alphabetically'
        if (SortOrder == soAlpha)
        {
            // Sort symbol vector alphabetically
            std::sort(FSymbols.begin(), FSymbols.end(), SymbolTextCompare);
        }
        // Sort order is 'by visibility'
        else if (SortOrder == soScope)
        {
            // Sort symbol vector by visibility
            std::sort(FSymbols.begin(), FSymbols.end(), SymbolVisibilityCompare);
        }
    }
}
//---------------------------------------------------------------------------

__fastcall TChBldCodeInsightSymbolList::~TChBldCodeInsightSymbolList()
{
    //CS_SEND(L"SymbolList::Destructor");
}
//---------------------------------------------------------------------------

void TChBldCodeInsightSymbolList::AddSymbol(const Ctags::TTag& Symbol)
{
    //CS_SEND(L"SymbolList::AddSymbol");

    FSymbols.push_back(Symbol);
    FCompleteSymbols.push_back(Symbol);
}
//---------------------------------------------------------------------------

String TChBldCodeInsightSymbolList::GetLongestItem()
{
    //CS_SEND(L"SymbolList::GetLongestItem");

    int         MaxLength   = 0;
    String      LongestItem = L"";

    foreach_ (Ctags::TTag& Symbol, FSymbols)
    {
        if (Symbol.Kind.Length() > MaxLength)
        {
            MaxLength   = Symbol.Kind.Length();
            LongestItem = Symbol.Kind;
        }
    }

    return LongestItem;
}
//---------------------------------------------------------------------------

// Implementor should clear its symbol list
void __fastcall TChBldCodeInsightSymbolList::Clear()
{
    //CS_SEND(L"SymbolList::Clear");

    FSymbols.clear();
    FCompleteSymbols.clear();
}
//----------------------------------------------------------------------------

// Given an identifier, return the index of the closest partial match
int __fastcall TChBldCodeInsightSymbolList::FindIdent(const String Ident)
{
    //CS_SEND(L"SymbolList::FindIdent(" + Ident + L")");

    // Extract the pure token
    String CodePoint = Environment::ExtractToken(Ident);

    for (std::size_t i = 0; i < FSymbols.size(); ++i)
    {
        if (AnsiStartsStr(CodePoint.LowerCase(), FSymbols[i].Name.LowerCase()))
            return static_cast<int>(i);
    }

    return -1;
}
//----------------------------------------------------------------------------

// Given an identifier, find the 'Index' of an exact match in the list and return true.
// Otherwise return false
bool __fastcall TChBldCodeInsightSymbolList::FindSymIndex(const String Ident, int& Index)
{
    //CS_SEND(L"SymbolList::FindSymIndex(" + Ident + L")");

    // Extract the pure token
    String CodePoint = Environment::ExtractToken(Ident);

    for (std::size_t i = 0; i < FSymbols.size(); ++i)
    {
        //if (CodePoint.LowerCase() == FSymbols[i].Name.LowerCase())
        if (AnsiStartsStr(CodePoint.LowerCase(), FSymbols[i].Name.LowerCase())) // Fuzzy search here, too
        {
            Index = i;

            return true;
        }
    }

    return false;
}
//----------------------------------------------------------------------------

// Set the lists filter to 'FilterText'.  It is up to the implementor to determine how to filter or
// if they even want to filter
void __fastcall TChBldCodeInsightSymbolList::SetFilter(const String FilterText)
{
    //CS_SEND(L"SymbolList::SetFilter(" + FilterText + L")");

    // Clear the (maxbe stripped-down) symbol list
    FSymbols.clear();

    // Add the complete symbol list
    FSymbols = FCompleteSymbols;

    // Extract the pure token as filter text
    FFilterText = Environment::ExtractToken(FilterText).LowerCase();

    // Filter with filter text
    if (!FFilterText.IsEmpty())
    {
        for (   Ctags::VTag::iterator it = FSymbols.begin();
                it != FSymbols.end();
                // --- no third parameter --
            )
        {
            if (AnsiStartsStr(FFilterText, it->Name.LowerCase()))
                ++it;
            else
                it = FSymbols.erase(it);
        }
    }
}
//----------------------------------------------------------------------------

// Returns the count of the symbols in the list - may be modified by setting a filter -
int __fastcall TChBldCodeInsightSymbolList::GetCount()
{
    //CS_SEND(L"SymbolList::GetCount(" + String(FSymbols.size()) + L")");

    return static_cast<int>(FSymbols.size());
}
//----------------------------------------------------------------------------

// Returns whether the symbol is able to be read from and written to
bool __fastcall TChBldCodeInsightSymbolList::GetSymbolIsReadWrite(int Index)
{
    //CS_SEND(L"SymbolList::GetSymbolIsReadWrite");

    // #TODO#

    if (Index < static_cast<int>(FSymbols.size()))
        return (FSymbols[Index].Access == L"public");
    else
        return false;
}
//----------------------------------------------------------------------------

// Returns whether the symbols is abstract. Viewer draws these in the 'need to implement' color
bool __fastcall TChBldCodeInsightSymbolList::GetSymbolIsAbstract(int Index)
{
    //CS_SEND(L"SymbolList::GetSymbolIsAbstract");

    if (Index < static_cast<int>(FSymbols.size()))
    {
        return (
            (FSymbols[Index].Implementation == L"virtual")
            ||  (FSymbols[Index].Implementation == L"pure virtual")
            );
    }
    else
    {
        return false;
    }
}
//----------------------------------------------------------------------------

// Return the symbol flags for the item at index 'Index'. Index is the index in the filtered list
TOTAViewerSymbolFlags __fastcall TChBldCodeInsightSymbolList::GetViewerSymbolFlags(int Index)
{
    //CS_SEND(L"SymbolList::GetViewerSymbolFlags");

    // vsfUnknown, vsfConstant, vsfType, vsfVariable, vsfProcedure, vsfFunction, vsfUnit, vsfLabel,
    // vsfProperty, vsfConstructor, vsfDestructor, vsfInterface, vsfEvent, vsfParameter,
    // vsfLocalVar, vsfReservedWord

    if (Index < static_cast<int>(FSymbols.size()))
    {
        Ctags::TTag &Symbol = FSymbols[Index];

        if (Symbol.Kind == L"macro")            return vsfReservedWord;
        if (Symbol.Kind == L"typedef")          return vsfType;
        if (Symbol.Kind == L"namespace")        return vsfReservedWord;
        if (Symbol.Kind == L"prototype")        return vsfReservedWord;
        if (Symbol.Kind == L"function")         return vsfFunction;
        if (Symbol.Kind == L"implementation")   return vsfReservedWord;
        if (Symbol.Kind == L"member")           return vsfLocalVar;
        if (Symbol.Kind == L"class")            return vsfType;
        if (Symbol.Kind == L"struct")           return vsfReservedWord;
        if (Symbol.Kind == L"variable")         return vsfVariable;
        if (Symbol.Kind == L"local")            return vsfLocalVar;
        if (Symbol.Kind == L"property")         return vsfProperty;
        if (Symbol.Kind == L"constructor")      return vsfConstructor;
        if (Symbol.Kind == L"destructor")       return vsfDestructor;
        if (Symbol.Kind == L"enum")             return vsfType;
        if (Symbol.Kind == L"enumerator")       return vsfType;
        if (Symbol.Kind == L"externvar")        return vsfVariable;
    }

    return vsfUnknown;
}
//----------------------------------------------------------------------------

// Return the visibility flags for the item at index 'Index'.
// Index is the index in the filtered list
int __fastcall TChBldCodeInsightSymbolList::GetViewerVisibilityFlags(int Index)
{
    //CS_SEND(L"SymbolList::GetViewerVisibilityFlags");

    if (Index < static_cast<int>(FSymbols.size()))
    {
        Ctags::TTag &Symbol = FSymbols[Index];

        if (Symbol.Access == L"private")
            return vvfPrivate;

        if (Symbol.Access == L"protected")
            return vvfProtected;

        if (Symbol.Access == L"public")
            return vvfPublic;


        // Note: '__published' is handled like 'public' by Ctags

        return vvfPublic;
    }
    else
    {
        return vvfDeprecated;
    }


    /*
    static const System::Int8 vvfPrivate    = System::Int8(0x0);
    static const System::Int8 vvfProtected  = System::Int8(0x1);
    static const System::Int8 vvfPublic     = System::Int8(0x2);
    static const System::Int8 vvfPublished  = System::Int8(0x3);

    static const System::Int8 vvfVisMask    = System::Int8(0x4);
    static const System::Int8 vvfDeprecated = System::Int8(0x8);
    */
}
//----------------------------------------------------------------------------

// Return the procedure flags for the item at index 'Index'. Index is the index in the filtered list
TOTAProcDispatchFlags __fastcall TChBldCodeInsightSymbolList::GetProcDispatchFlags(int Index)
{
    //CS_SEND(L"SymbolList::GetProcDispatchFlags");

    if (Index < static_cast<int>(FSymbols.size()))
    {
        Ctags::TTag &Symbol = FSymbols[Index];

        if (Symbol.Implementation == L"virtual")
            return pdfVirtual;

        /* ===== #TODO#: Dynamic ? === */
    }

     // pdfNone, pdfVirtual, pdfDynamic

    return pdfNone;
}
//----------------------------------------------------------------------------

// Returns the sort order of the list
TOTASortOrder __fastcall TChBldCodeInsightSymbolList::GetSortOrder()
{
    //CS_SEND(L"SymbolList::GetSortOrder");

    // soAlpha, soScope

    return FSortOrder;
}
//----------------------------------------------------------------------------

// The list was requested to be sorted by 'Value'
void __fastcall TChBldCodeInsightSymbolList::SetSortOrder(const TOTASortOrder Value)
{
    //CS_SEND(L"SymbolList::SetSortOrder");

    FSortOrder = Value;

    // Sort order is 'alphabetically'
    if (FSortOrder == soAlpha)
    {
        // Sort symbol vector alphabetically
        std::sort(FSymbols.begin(), FSymbols.end(), SymbolTextCompare);
    }
    // Sort order is 'by visibility'
    else if (FSortOrder == soScope)
    {
        // Sort symbol vector by visibility
        std::sort(FSymbols.begin(), FSymbols.end(), SymbolVisibilityCompare);
    }
}
//----------------------------------------------------------------------------

// Return the symbol text for item 'Index'.  i.e. Form1
String __fastcall TChBldCodeInsightSymbolList::GetSymbolText(int Index)
{
    //CS_SEND(L"SymbolList::GetSymbolText(" + String(Index) + L") \"" + FSymbols[Index].Name + L"\"");

    if (Index < static_cast<int>(FSymbols.size()))
        return FSymbols[Index].Name;
    else
        return L"";
}
//----------------------------------------------------------------------------

// Return the symbol type text for item 'Index'.  i.e. TForm1
String __fastcall TChBldCodeInsightSymbolList::GetSymbolTypeText(int Index)
{
    //CS_SEND(L"SymbolList::GetSymbolTypeText");

    if (Index < static_cast<int>(FSymbols.size()))
    {
        String SymbolType = FSymbols[Index].Typeref_B.Trim();

        if ((SymbolType.IsEmpty() || SymbolType == L"void"))
            return FSymbols[Index].Signature;

        return FSymbols[Index].Signature + L" : " + SymbolType;
    }
    else
    {
        return L"";
    }
}
//----------------------------------------------------------------------------

// Return the symbol class text for item 'Index'.  i.e. 'var', 'function', 'type', etc
String __fastcall TChBldCodeInsightSymbolList::GetSymbolClassText(int Index)
{
    //CS_SEND(L"SymbolList::GetSymbolClassText");

    if (Index < static_cast<int>(FSymbols.size()))
        return FSymbols[Index].Kind;
    else
        return L"";
}
//----------------------------------------------------------------------------

// Return 'true' if the position of X.Text is alphabetically preceding Y.Text
bool TChBldCodeInsightSymbolList::SymbolTextCompare(const Ctags::TTag& X, const Ctags::TTag& Y)
{
    //CS_SEND(L"SymbolList::SymbolTextCompare(" + X.Name + L"<>" + Y.Name + L")");

    return (AnsiCompareStr(X.Name, Y.Name) == -1);
}
//---------------------------------------------------------------------------

// Return 'true' if the visibility of X.VisibilityFlag is preceding Y.VisibilityFlag
// __published firt ... private last
bool TChBldCodeInsightSymbolList::SymbolVisibilityCompare(const Ctags::TTag& X, const Ctags::TTag& Y)
{
    //CS_SEND(L"SymbolList::SymbolVisibilityCompare");

    /*
    static const System::Int8 vvfPrivate    = System::Int8(0x0);
    static const System::Int8 vvfProtected  = System::Int8(0x1);
    static const System::Int8 vvfPublic     = System::Int8(0x2);
    static const System::Int8 vvfPublished  = System::Int8(0x3);

    static const System::Int8 vvfVisMask    = System::Int8(0x4);
    static const System::Int8 vvfDeprecated = System::Int8(0x8);
    */

    // Don't take 'vvVisMask' and 'vvfDeprecated' into account

    int XOrder;
    int YOrder;

    if      (X.Access == L"private")    XOrder =  0;
    else if (X.Access == L"protected")  XOrder =  1;
    else if (X.Access == L"public")     XOrder =  2;
    else                                XOrder = -1;

    if      (Y.Access == L"private")    YOrder =  0;
    else if (Y.Access == L"protected")  YOrder =  1;
    else if (Y.Access == L"public")     YOrder =  2;
    else                                YOrder = -1;

    // Note: '__published' is handled like 'public' by Ctags

    return ((XOrder) > (YOrder));
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder



