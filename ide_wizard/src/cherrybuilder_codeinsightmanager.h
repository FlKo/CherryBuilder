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

#ifndef cherrybuilder_codeinsightmanagerH
#define cherrybuilder_codeinsightmanagerH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include <System.IniFiles.hpp>

#include <Vcl.Imaging.pngimage.hpp>

#include <vector>
#include <boost/shared_ptr.hpp>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"
#include "cherrybuilder_codeinsightsymbollist.h"
#include "cherrybuilder_projectdb.h"
#include "cherrybuilder_commonhintform.h"
#include "cherrybuilder_symbolhintform.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

struct TChBldInvocationIconSet
{
    boost::shared_ptr<TPngImage> At;
    boost::shared_ptr<TPngImage> Attribute;
    boost::shared_ptr<TPngImage> Circle;
    boost::shared_ptr<TPngImage> Class;
    boost::shared_ptr<TPngImage> Cut;
    boost::shared_ptr<TPngImage> Enum;
    boost::shared_ptr<TPngImage> Field;
    boost::shared_ptr<TPngImage> GlobalIdent;
    boost::shared_ptr<TPngImage> Interface;
    boost::shared_ptr<TPngImage> Keyword;
    boost::shared_ptr<TPngImage> Method;
    boost::shared_ptr<TPngImage> Namespace;
    boost::shared_ptr<TPngImage> Tag;
    boost::shared_ptr<TPngImage> Unit;
    boost::shared_ptr<TPngImage> Word;
};
//---------------------------------------------------------------------------

enum TChBldInvocationOperator : int
{
    ivkUnknown      = 0,
    ivkDot          = 1,
    ivkArrow        = 2,
    ivkDoubleColon  = 3
};
//---------------------------------------------------------------------------

struct TChBldInvocationData
{
	TChBldInvocationData()
		:   SymbolList(new TChBldCodeInsightSymbolList),
        	InvocationOperator(ivkUnknown),
			Token(L""),
            InvocationType(itAuto),
            PreviousSelectionIndex(-1),
            PreviousSelectionRect(TRect(0, 0, 0, 0)),
            InvocationKey(L'\x00')
		{}

    ~TChBldInvocationData() {
        if (SymbolList)
        {
            delete SymbolList;
            SymbolList = NULL;
        }
    }

    TChBldCodeInsightSymbolList *SymbolList;
	TChBldInvocationOperator    InvocationOperator;
	String 					    Token;
    wchar_t                     InvocationKey;
    TOTAInvokeType              InvocationType;
    int                         PreviousSelectionIndex;
    TRect                       PreviousSelectionRect;
};
//---------------------------------------------------------------------------

struct TChBldJumpbackData
{
    int     Line;
    int     Column;
    String  FileName;
};
//---------------------------------------------------------------------------

class TChBldCodeInsightManager : public TCppInterfacedObject<IOTANotifier, IOTACodeInsightManager, INTACustomDrawCodeInsightViewer>
{
public:
            __fastcall TChBldCodeInsightManager(TMemIniFile* SettingsINI, TChBldProjectDB& ProjectDB);
    virtual __fastcall ~TChBldCodeInsightManager();

    String      __fastcall  GetName();
    String      __fastcall  GetIDString();
    bool        __fastcall  HandlesFile(const String FileName);
    bool        __fastcall  GetEnabled();
    void        __fastcall  SetEnabled(bool Value) ;
    void        __fastcall  AllowCodeInsight(bool& Allow, const wchar_t Key);
    TSysCharSet __fastcall  EditorTokenValidChars(bool PreValidating);
    bool        __fastcall  PreValidateCodeInsight(const String Str);
    bool        __fastcall  InvokeCodeCompletion(TOTAInvokeType HowInvoked, String& Str);
    void        __fastcall  GetSymbolList(_di_IOTACodeInsightSymbolList& SymbolList);
    bool        __fastcall  IsViewerBrowsable(int Index);
    bool        __fastcall  GetMultiSelect();
    void        __fastcall  OnEditorKey(wchar_t Key, bool& CloseViewer, bool& Accept);
    String      __fastcall  GetLongestItem();
    void        __fastcall  GetParameterList(_di_IOTACodeInsightParameterList& ParameterList);

    void        __fastcall  GetCodeInsightType(
                                wchar_t Char,
                                int Element,
                                TOTACodeInsightType& CodeInsightType,
                                TOTAInvokeType& InvokeType
                                );

    bool        __fastcall  InvokeParameterCodeInsight(TOTAInvokeType HowInvoked, int& SelectedIndex);
    void        __fastcall  ParameterCodeInsightAnchorPos(TOTAEditPos& EdPos);
    int         __fastcall  ParameterCodeInsightParamIndex(const TOTAEditPos& EdPos);
    String      __fastcall  GetHintText(int HintLine, int HintCol);
    String      __fastcall  GetOptionSetName();
    bool        __fastcall  GotoDefinition(String& FileName, int& LineNum, int Index=0xFFFFFFFF);
    void        __fastcall  Done(bool Accepted, bool& DisplayParams);

    void                    JumpToHeader();
    void                    JumpToImplementation();
    void                    DoJumpbackImplementation();
    void                    DoJumpbackHeader();

    void                    PerformBrowsingRequest();
    void                    PerformSymbolHintRequest();
    void                    PerformParameterHintRequest();

    __property String   Name        = {read=GetName};
    __property bool     MultiSelect = {read=GetMultiSelect};
    __property bool     Enabled     = {read=GetEnabled, write=SetEnabled};

    __property TChBldInvocationData InvocationData  = {read=FInvocationData};
    __property TChBldJumpbackData   JumpbackDataImplementation  = {read=FJumpbackDataImplementation};
    __property TChBldJumpbackData   JumpbackDataHeader          = {read=FJumpbackDataHeader};

    /* INTACustomDrawCodeInsightViewer */
    void __fastcall DrawLine(
        int Index,
        TCanvas* Canvas,
        TRect &Rect,
        bool DrawingHintText,
        bool DoDraw,
        bool &DefaultDraw
        );

    /* IOTANotifier */
    void __fastcall AfterSave() {}
    void __fastcall BeforeSave() {}
    void __fastcall Destroyed() {}
    void __fastcall Modified() {}

private:
    TChBldInvocationOperator    GetInvocationOperatorAndToken(const String& Str, String& CodePoint);

    void                        PerformInvocation(
                                    const String& Token,
                                    const int& Line,
                                    const int& Column,
                                    const String& FileName,
                                    TChBldInvocationOperator InvocationOperator,
                                    Ctags::VTag& MatchingIdentifiers,
                                    bool MustMatchInvocationOperator=true
                                    );

    void                        HandleInvocation(
                                    TChBldInvocationOperator InvocationOperator,
                                    const String& Token,
                                    Ctags::VTag& MatchingIdentifiers,
                                    const VString& Namespaces,
                                    const VString& ClassesAndStructs,
                                    bool MustMatchInvocationOperator=true
                                    );

    void                        GetMatchingObjectIdentifiers(
                                    const String& ObjectType,
                                    Ctags::VTag& MatchingIdentifiers
                                    );

    boost::shared_ptr<TPngImage> GetTypeIconByClassText(const String& ClassText);

    void                        DrawListItem(const int Index, TCanvas* Canvas, const TRect& Rect);
    String                      ReplaceWithTypeAbbreviations(String Text);

    void                        LoadIcons();

    TMemIniFile                 *FSettingsINI;
    TCommonHintForm             *FCommonHintForm;
    TSymbolHintForm             *FSymbolHintForm;
    TSysCharSet                 FSysCharSet;
    TChBldProjectDB             &FProjectDB;
    TChBldInvocationData        FInvocationData;
    TChBldJumpbackData          FJumpbackDataImplementation;
    TChBldJumpbackData          FJumpbackDataHeader;

    VString         FKnownTags;

    std::vector<TChBldInvocationIconSet> FCCIcons;
};

} // namespace Cherrybuilder

#endif


