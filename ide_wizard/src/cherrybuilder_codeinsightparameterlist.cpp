/*===========================================================================
 * CherryBuilder - The Productivity Extension for C++Builder®
 *---------------------------------------------------------------------------
 * Copyright (C) 2017 Florian Koch <flko@mail.de>
 * All Rights Reserved
 *---------------------------------------------------------------------------
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 *===========================================================================
 */

#include <vcl.h>
#pragma hdrstop

#include "cherrybuilder_codeinsightparameterlist.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

// Construktor
__fastcall TChBldCodeInsightParameterList::TChBldCodeInsightParameterList()
{

}
//---------------------------------------------------------------------------

// Destructor
__fastcall TChBldCodeInsightParameterList::~TChBldCodeInsightParameterList()
{

}
//---------------------------------------------------------------------------

// Returns a ParamQuery to the caller based upon the ProcIndex. There may be multiple items in list
// as we may be dealing with overloaded functions.
void __fastcall TChBldCodeInsightParameterList::GetParameterQuery(
    int ProcIndex,
    _di_IOTACodeInsightParamQuery &ParamQuery
    )
{

}
//---------------------------------------------------------------------------

// Return which character to use to delimit parameters in the parameter hint window.
// i.e.  C++ uses ',', Object Pascal uses ';'
wchar_t __fastcall TChBldCodeInsightParameterList::GetParamDelimiter()
{
    return L',';
}
//---------------------------------------------------------------------------

// Returns the count of procedures in the list
int __fastcall TChBldCodeInsightParameterList::GetProcedureCount()
{

}
//---------------------------------------------------------------------------

// Returns the parameters as a string from the procedure at index Index. The parameters
// should be delimited by a line ending (sLineBreak for instance).
String __fastcall TChBldCodeInsightParameterList::GetProcedureParamsText(int Index)
{

}
//---------------------------------------------------------------------------

TOTACharPos __fastcall TChBldCodeInsightParameterList::GetParmPos(int Index)
{

}
//---------------------------------------------------------------------------

int __fastcall TChBldCodeInsightParameterList::GetParmCount()
{

}
//---------------------------------------------------------------------------

String __fastcall TChBldCodeInsightParameterList::GetParmName(int Index)
{

}
//---------------------------------------------------------------------------

String __fastcall TChBldCodeInsightParameterList::GetParmHint(int Index)
{

}
//---------------------------------------------------------------------------

TOTACharPos __fastcall TChBldCodeInsightParameterList::GetCallStartPos()
{

}
//---------------------------------------------------------------------------

TOTACharPos __fastcall TChBldCodeInsightParameterList::GetCallEndPos()
{

}
//---------------------------------------------------------------------------

// Return 'true' if the position of X.Text is alphabetically preceding Y.Text
/*bool TChBldCodeInsightParameterList::SymbolTextCompare(const TChBldSymbol& X, const TChBldSymbol& Y)
{
    CS_SEND(L"ParameterList::SymbolTextCompare");

    return (AnsiCompareStr(X.Text, Y.Text) == -1);
}*/
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


