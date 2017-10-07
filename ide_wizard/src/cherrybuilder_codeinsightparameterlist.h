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

#ifndef cherrybuilder_codeinsightparameterlistH
#define cherrybuilder_codeinsightparameterlistH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include <vector>
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

struct TChBldParameter
{

};
//---------------------------------------------------------------------------

class TChBldCodeInsightParameterList : public TCppInterfacedObject<IOTACodeInsightParameterList>
{
public:
            __fastcall TChBldCodeInsightParameterList();
    virtual __fastcall ~TChBldCodeInsightParameterList();

    void    __fastcall GetParameterQuery(
                            int ProcIndex,
                            _di_IOTACodeInsightParamQuery &ParamQuery
                            );

    wchar_t     __fastcall  GetParamDelimiter();
    int         __fastcall  GetProcedureCount();
    String      __fastcall  GetProcedureParamsText(int Index);
    TOTACharPos __fastcall  GetParmPos(int Index);
    int         __fastcall  GetParmCount();
    String      __fastcall  GetParmName(int Index);
    String      __fastcall  GetParmHint(int Index);
    TOTACharPos __fastcall  GetCallStartPos();
    TOTACharPos __fastcall  GetCallEndPos();

    __property TOTACharPos  CallStartPos                    = {read=GetCallStartPos};
    __property TOTACharPos  CallEndPos                      = {read=GetCallEndPos};
    __property int          ParmCount                       = {read=GetParmCount};
    __property String       ParmHint[int Index]             = {read=GetParmHint};
    __property String       ParmName[int Index]             = {read=GetParmName};
    __property TOTACharPos  ParmPos[int Index]              = {read=GetParmPos};
    __property String       ProcedureParamsText[int Index]  = {read=GetProcedureParamsText};
    __property int          ProcedureCount                  = {read=GetProcedureCount};
    __property wchar_t      ParamDelimiter                  = {read=GetParamDelimiter};

private:
    //static bool SymbolTextCompare(const TChBldSymbol& AX, const TChBldSymbol& AY);

    std::vector<TChBldParameter> FParameters;
    TOTASortOrder FSortOrder;
};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#endif


