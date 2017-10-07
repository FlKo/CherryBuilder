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

#include "cherrybuilder_codeinsightmanager.h"

#include <System.StrUtils.hpp>

#include <memory>
#include <map>

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

__fastcall TChBldCodeInsightManager::TChBldCodeInsightManager(
    TMemIniFile* SettingsINI,
    TChBldProjectDB& ProjectDB
    )
    :   FSettingsINI(SettingsINI),
        FProjectDB(ProjectDB),
        FCommonHintForm(new TCommonHintForm(NULL)),
        FSymbolHintForm(new TSymbolHintForm(NULL))
{
    CS_SEND(L"CodeInsightManager::Constructor");

    FSysCharSet = COMPLETIONCHARSET;

    LoadIcons();
}
//---------------------------------------------------------------------------

__fastcall TChBldCodeInsightManager::~TChBldCodeInsightManager()
{
    CS_SEND(L"CodeInsightManager::Destructor");

    if (FCommonHintForm)
    {
        delete FCommonHintForm;
        FCommonHintForm = NULL;
    }

    if (FSymbolHintForm)
    {
        delete FSymbolHintForm;
        FSymbolHintForm = NULL;
    }
}
//---------------------------------------------------------------------------

// Returns a description of the language which we handle
String __fastcall TChBldCodeInsightManager::GetName()
{
    CS_SEND(L"CodeInsightManager::GetName");

    return L"CherryBuilder C++";
}
//----------------------------------------------------------------------------

// Returns a unique IDString to the services module
String __fastcall TChBldCodeInsightManager::GetIDString()
{
    //CS_SEND(L"CodeInsightManager::GetIDString");

    return L"FlKo.CherryBuilder";
}
//----------------------------------------------------------------------------

// Returns true if this manager should handle this file
bool __fastcall TChBldCodeInsightManager::HandlesFile(const String FileName)
{
    CS_SEND(L"CodeInsightManager::HandlesFile");

    return (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName));
}
//----------------------------------------------------------------------------

// Returns whether we should be able to be invoked or not
bool __fastcall TChBldCodeInsightManager::GetEnabled()
{
    CS_SEND(L"CodeInsightManager::GetEnabled");

    return FSettingsINI->ReadBool(L"CodeCompletion", L"Active", true);
}
//----------------------------------------------------------------------------

// Sets the active state to Value so this manager may be turned off
void __fastcall TChBldCodeInsightManager::SetEnabled(bool Value)
{
    CS_SEND(L"CodeInsightManager::SetEnabled");

    FSettingsINI->WriteBool(L"CodeCompletion", L"Active", Value);
}
//----------------------------------------------------------------------------

/* The implementor should set Allow to True if it wishes to be invoked for the key 'Key'.
   'Key' is the key which the user pressed to invoke code completion.
   There are four special values to 'Key' when invoked by the code insight timer.

   They are as follows:
   #0 : Code completion was requested.
   #1 : Parameter insight was requested.
   #2 : A browse was requested.
   #3 : A symbol hint was requested.
*/
void __fastcall TChBldCodeInsightManager::AllowCodeInsight(bool& Allow, const wchar_t Key)
{
    CS_SEND(L"CodeInsightManager::AllowCodeInsight(" + String((int)Key) + L")");

    // Code completion request
    if (Key == L'\x00')
    {
        Allow = true;
    }
    // Parameter insight request
    else if (Key == L'\x01')
    {
        Allow = true;
    }
    // Browser request
    else if (Key == L'\x02')
    {
        // We can't call 'PerformBrowsingRequest' directly from here as it renders
        // access violations. Instead we'll handle this in the 'BeforeCompile' event
        // which is fired by the IDE afterwards if we allow CC

        Allow = true;
    }
    // Symbol hint request
    else if (Key == L'\x03')
    {
        // We can't call 'PerformSymbolHint' directly from here as it renders
        // access violations. Instead we'll handle this in the 'BeforeCompile' event
        // which is fired by the IDE afterwards if we allow CC

        Allow = true;
    }
    // Operator request
    else if ((Key == L'.') || (Key == L'>') || (Key == L':'))
    {
        Allow = true;
    }
    else
    {
        Allow = false;
    }

    if (Allow)
        FInvocationData.InvocationKey = Key;
}
//----------------------------------------------------------------------------

/* Returns a charset used to get the token at the current editor position.  This is
   used for retrieving the seed text when code completion is invoked as well as
   retrieving the token from the editor when we are typing for look ahead.
   The PreValidating parameter should be used to add special tokens to the charset for retrieval
   from the editor.  For instance, C++ might add ['.', '-', '>'] to the returned charset
   when it is prevalidating.
*/
TSysCharSet __fastcall TChBldCodeInsightManager::EditorTokenValidChars(bool PreValidating)
{
    CS_SEND(L"CodeInsightManager::EditorTokenValidChars");

    return FSysCharSet;
}
//----------------------------------------------------------------------------

// The implementor should return true if it wishes to allow the token 'Str' to be a valid code
// point for Code Insight.
bool __fastcall TChBldCodeInsightManager::PreValidateCodeInsight(const String Str)
{
    CS_SEND(L"CodeInsightManager::PreValidateCodeInsight(" + Str + L")");

    // Seperate the invocation operator and the token
    FInvocationData.InvocationOperator = GetInvocationOperatorAndToken(Str, FInvocationData.Token);

    switch (FInvocationData.InvocationOperator)
    {
        case ivkDot:
        case ivkArrow:
        case ivkDoubleColon:
            return true;

        default:
            return false;
    }
}
//----------------------------------------------------------------------------

// Returns true if invocation was successful.  HowInvoked informs the implementor whether it was
// invoked via timer, manual, etc...  Str is the text to seed to viewer with and is used for the
// initial filtering in the viewer.
bool __fastcall TChBldCodeInsightManager::InvokeCodeCompletion(
    TOTAInvokeType HowInvoked,
    String& Str
    )
{
    CS_SEND(L"CodeInsightManager::InvokeCodeCompletion(" + String(HowInvoked) + L")");

    // Clear the current symbol list
    FInvocationData.SymbolList->Clear();

    // Get the current invocation type
    FInvocationData.InvocationType = HowInvoked;

    switch (FInvocationData.InvocationType)
    {
        // Invoked by pressing '.', '->' or '::'
        case itAuto:
        // Invoked by 'Ctrl+Space'
        case itManual:

            {
                Screen->Cursor = crAppStart;

                // Begin of interlock
                __try
                {
                    TChBldLockGuard LG(FProjectDB.UpdateMutex);

                    int Line;
                    int Column;
                    String FileName;

                    // Get the current editor data
                    if (!IDE::GetCurrentEditorPos(Line, Column, FileName))
                        return false;

                    Ctags::VTag  MatchingIdentifiers;

                    // Perform the invocation
                    PerformInvocation(
                        FInvocationData.Token,
                        Line,
                        Column,
                        FileName,
                        FInvocationData.InvocationOperator,
                        MatchingIdentifiers
                        );

                    // Add the resulting data to the symbol list
                    foreach_ (Ctags::TTag& Symbol, MatchingIdentifiers)
                        FInvocationData.SymbolList->AddSymbol(Symbol);
                }
                // End of interlock
                __finally
                {
                    Screen->Cursor = crDefault;
                }
            }

        break;

        // Invoked by ?
        case itTimer:
        break;
    }

    return true;
}
//----------------------------------------------------------------------------

// Returns the symbol list to the caller
void __fastcall TChBldCodeInsightManager::GetSymbolList(_di_IOTACodeInsightSymbolList& SymbolList)
{
    CS_SEND(L"CodeInsightManager::GetSymbolList");

    // Create a sorted 'clone' instance of the invocation symbol list (IDE will later call destructor)
    TChBldCodeInsightSymbolList *NewSymbolList =
        new TChBldCodeInsightSymbolList(true, IDE::GetCurrentSymbolListSortOrder());

    // Return the interface of the new symbol list instance clone
    NewSymbolList->QueryInterface(IID_IOTACodeInsightSymbolList, (void**)&SymbolList);
}
//----------------------------------------------------------------------------

// Returns whether the symbol at index 'Index' is browseable in the code completion viewer
bool __fastcall TChBldCodeInsightManager::IsViewerBrowsable(int Index)
{
    CS_SEND(L"CodeInsightManager::IsViewerBrowsable");

    return true;
}
//----------------------------------------------------------------------------

// Returns whether the code completion viewer allows multi-select
bool __fastcall TChBldCodeInsightManager::GetMultiSelect()
{
    CS_SEND(L"CodeInsightManager::GetMultiSelect");

    return false;
}
//----------------------------------------------------------------------------

/* Determines whether or not the key 'Key' which was entered into the editor should close
   the code completion viewer or not (set CloseViewer to true or false depending on your choice).
   Also, the implementor should inform the manager whether or not it should accept the symbol
   at the currently selected index/indices.
*/
void __fastcall TChBldCodeInsightManager::OnEditorKey(wchar_t Key, bool& CloseViewer, bool& Accept)
{
    CS_SEND(L"CodeInsightManager::OnEditorKey");

    switch (Key)
    {
        case VK_RETURN:
        case VK_SPACE:

            CloseViewer = true;
            Accept      = true;

        break;

        case VK_ESCAPE:
        case VK_TAB:

            CloseViewer = true;
            Accept      = false;

        break;


        default:

            CloseViewer = false;
            Accept      = false;
    }
}
//----------------------------------------------------------------------------

// Returns the longest symbol class text for measurement for the viewer.  i.e.  'constructor' is
// longer than 'var'
String __fastcall TChBldCodeInsightManager::GetLongestItem()
{
    CS_SEND(L"CodeInsightManager::GetLongestItem");

    return FInvocationData.SymbolList->GetLongestItem();
}
//----------------------------------------------------------------------------

// Returns a parameter list to the manager
void __fastcall TChBldCodeInsightManager::GetParameterList(
    _di_IOTACodeInsightParameterList& ParameterList
    )
{
    CS_SEND(L"CodeInsightManager::GetParameterList");

    // We don't need this function as we handle parameter requests ourselves
}
//----------------------------------------------------------------------------

// Given key 'AChar' which was entered into the editor and the current element (atComment,
// atIdentifier, etc.), return how code insight should be invoked and which type of invocation it
// should be.
void __fastcall TChBldCodeInsightManager::GetCodeInsightType(
    wchar_t Char,
    int Element,
    TOTACodeInsightType& CodeInsightType,
    TOTAInvokeType& InvokeType
    )
{
    CS_SEND(L"CodeInsightManager::GetCodeInsightType");

    // Get the current editor actions interface
    _di_IOTAEditActions EditActions = IDE::GetCurrentEditActions();

    if (EditActions)
    {
        // csCodelist       = 0x01;
        // csParamList      = 0x02;
        // csManual         = 0x80;

        // Invoke the compiler again
        EditActions->CodeCompletion(csParamList);
    }
}
//----------------------------------------------------------------------------

// Returns true if invocation was successful. HowInvoked informs the implementor whether
// it was invoked via timer, manual, etc...  SelectedIndex is the index of the current parameter
// for the method/proc.
bool __fastcall TChBldCodeInsightManager::InvokeParameterCodeInsight(
    TOTAInvokeType HowInvoked,
    int&  SelectedIndex
    )
{
    CS_SEND(L"CodeInsightManager::InvokeParameterCodeInsight");

    switch (HowInvoked)
    {
        case itAuto:
        break;

        case itManual:
        break;

        case itTimer:
        break;
    }

    return true; // #TODO#
}
//----------------------------------------------------------------------------

// Tells the manager where it should anchor the parameter hint window.
// A default value (EdPos) is provided for the implementor to change if they so wish.
void __fastcall TChBldCodeInsightManager::ParameterCodeInsightAnchorPos(TOTAEditPos& EdPos)
{
    CS_SEND(L"CodeInsightManager::ParameterCodeInsightAnchorPos");
}
//----------------------------------------------------------------------------

/* Returns the index of the parameter which should be highlighted based upon EdPos.
   This is used to reduce extra codeinsight invocations as an implementor might
   store off the editor positions of parameters on the first invocation.
   return a -1 if you want to be reinvoked.
*/
int __fastcall TChBldCodeInsightManager::ParameterCodeInsightParamIndex(const TOTAEditPos& EdPos)
{
    CS_SEND(L"CodeInsightManager::ParameterCodeInsightParamIndex");

    return 0;
}
//----------------------------------------------------------------------------

// Return the hint string for the position in the editor
// (HintLine/HintCol are the editor coordinates)
String __fastcall TChBldCodeInsightManager::GetHintText(int HintLine, int HintCol)
{
    CS_SEND(L"CodeInsightManager::GetHintText");

    return L"NoHint";
}
//----------------------------------------------------------------------------

// Return the text to be shown on the IDE options dialog for 'CodeInsight'
String __fastcall TChBldCodeInsightManager::GetOptionSetName()
{
    CS_SEND(L"CodeInsightManager::GetOptionSetName");

    // The configuration of CherryBuilder takes place in it's custom dialog
    // so we won't show up in the IDE Settings
    return L"";
}
//---------------------------------------------------------------------------

// Return a FileName and LineNumber for the symbol which is requested to be browsed to.
// if Index > -1 then it is an index into the symbol list and the browse was requested
// by a user clicking in the code completion viewer.
// Return false if you'd like to inform the user that the requested operation failed otherwise
// return true. If you wish to fail by not informing the user, set AFileName = '' and ALineNum = 0.
// If Index is -1, you should use the global CodeInsightServices() and request the EditView from it.
// This should be able to give you any information you require.
bool __fastcall TChBldCodeInsightManager::GotoDefinition(
    String &FileName,
    int& LineNum,
    int Index
    )
{
    CS_SEND(L"CodeInsightManager::GotoDefinition");

    // This is only for the user selecting a symbol in the symbol list by mouse click
    // The browsing requests are not handled here
    if (Index > -1)
    {
        _di_IOTACodeInsightServices CIServices = IDE::GetInterface<_di_IOTACodeInsightServices>();
        _di_IOTACodeInsightViewer   CIViewer;

        // Get the current code insight viewer
        CIServices->GetViewer(CIViewer);

        // Insert the selected text at the editor pos
        CIServices->InsertText(
            CIViewer->SelectedString,
            true
            );
    }

    return true;
}
//----------------------------------------------------------------------------

/* Called when the code completion is completed.  Accepted is true if the user has requested
   the item hinted to them in the viewer otherwise Accepted is false.
   DisplayParams should be set to true if the implementor would like to be requeried
   for parameter invocation.  It is up to the implementor to insert the text into the editor.
   One way might be to use CodeInsightServices.InsertText(StrToInsert, ShouldReplace);
   Another might be to acquire the EditView from CodeInsightServices.GetEditView() and do
   the insertion yourself.
*/
void __fastcall TChBldCodeInsightManager::Done(bool Accepted, bool& DisplayParams)
{
    CS_SEND(L"CodeInsightManager::Done");

    if (Accepted)
    {
        _di_IOTACodeInsightServices CIServices = IDE::GetInterface<_di_IOTACodeInsightServices>();
        _di_IOTACodeInsightViewer   CIViewer;

        // Get the current code insight viewer
        CIServices->GetViewer(CIViewer);

        String SelectedText = CIViewer->SelectedString;

        // If there is a '(void)' in the symbol type text we let the manager invoke the parameter invocation
        // and append a '()' to the selected text
        if (FInvocationData.SymbolList->SymbolTypeText[CIViewer->GetManagerSelectedIndex(this)].Pos(L"(void)"))
        {
            DisplayParams = true;

            SelectedText += L"()";
        }
        // If there is a '(' in the symbol type text we let the manager invoke the parameter invocation
        // and append a '(' to the selected text
        else if (FInvocationData.SymbolList->SymbolTypeText[CIViewer->GetManagerSelectedIndex(this)].Pos(L"("))
        {
            DisplayParams = true;

            SelectedText += L"(";
        }
        else
        {
            DisplayParams = false;
        }

        // Insert the selected text at the editor pos
        CIServices->InsertText(
            SelectedText,
            true
            );
    }
}
//----------------------------------------------------------------------------

/*
  Called when the viewer draws the item at index 'Index'.  If DoDraw is false, then only a
  rectangle calculation is being requested.  The rectangle should be returned by the 'Rect'
  out parameter.
*/
void __fastcall TChBldCodeInsightManager::DrawLine(
    int Index,
    TCanvas* Canvas,
    TRect &Rect,
    bool DrawingHintText,
    bool DoDraw,
    bool &DefaultDraw
    )
{
    //CS_SEND(L"CodeInsightManager::DrawLine(" + String(Index) + L")");

    if (DoDraw)
    {
        // If there was a different previous selected list item
        if (    (FInvocationData.PreviousSelectionIndex != -1)
             && (FInvocationData.PreviousSelectionIndex != Index) )
        {
            // Delete the former selection rect
            DrawListItem(
                FInvocationData.PreviousSelectionIndex,
                Canvas,
                FInvocationData.PreviousSelectionRect
                );
        }

        // Draw the current selection
        DrawListItem(Index, Canvas, Rect);

        // Update the previous selection index
        FInvocationData.PreviousSelectionIndex = Index;

        // Update the previous selection rect
        FInvocationData.PreviousSelectionRect = Rect;

        DefaultDraw = false;
        DoDraw      = true;
    }
    else
    {
        DefaultDraw = true;
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::JumpToHeader()
{
    CS_SEND(L"CodeInsightManager::JumpToHeader");

    if (FSettingsINI->ReadBool(L"CodeCompletion", L"EnableShiftCtrlArrowNavigation", true))
    {
        int Line;
        int Column;
        String FileName;

        // Get the current editor data
        if (IDE::GetCurrentEditorPos(Line, Column, FileName))
        {
            // Get the implementation symbol
            Ctags::TTag ImplementationSymbol = FProjectDB.GetPosImplementation(FileName, Line, Column);

            // Get the header symbol
            Ctags::TTag HeaderSymbol = FProjectDB.GetPosHeaderTarget(ImplementationSymbol);

            // Check if the file exists
            if (FileExists(HeaderSymbol.File))
            {
                // Open the file and go to line
                IDE::ShowEditorLine(HeaderSymbol.File, HeaderSymbol.LineNo);
            }

            // Store the jumpback data
            FJumpbackDataImplementation.Line      = Line;
            FJumpbackDataImplementation.Column    = Column;
            FJumpbackDataImplementation.FileName  = FileName;
        }
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::JumpToImplementation()
{
    CS_SEND(L"CodeInsightManager::JumpToImplementation");

    if (FSettingsINI->ReadBool(L"CodeCompletion", L"EnableShiftCtrlArrowNavigation", true))
    {
        int Line;
        int Column;
        String FileName;

        // Get the current editor data
        if (IDE::GetCurrentEditorPos(Line, Column, FileName))
        {
            // Get the header symbol
            Ctags::TTag HeaderSymbol = FProjectDB.GetPosHeader(FileName, Line, Column);

            // Get the implementation symbol
            Ctags::TTag ImplementationSymbol = FProjectDB.GetPosImplementationTarget(HeaderSymbol);

            // Check if the file exists
            if (FileExists(ImplementationSymbol.File))
            {
                // Open the file and go to line
                IDE::ShowEditorLine(ImplementationSymbol.File, ImplementationSymbol.LineNo);
            }
        }

        // Store the jumpback data
        FJumpbackDataHeader.Line      = Line;
        FJumpbackDataHeader.Column    = Column;
        FJumpbackDataHeader.FileName  = FileName;
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::DoJumpbackImplementation()
{
    CS_SEND(L"CodeInsightManager::DoJumpbackImplementation");

    if (FSettingsINI->ReadBool(L"CodeCompletion", L"EnableShiftCtrlArrowJumpback", true))
    {
        // Check if the file exists
        if (FileExists(FJumpbackDataImplementation.FileName))
        {
            // Open the file and go to line
            IDE::ShowEditorLine(
                FJumpbackDataImplementation.FileName,
                FJumpbackDataImplementation.Line,
                FJumpbackDataImplementation.Column
                );
        }
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::DoJumpbackHeader()
{
    CS_SEND(L"CodeInsightManager::DoJumpbackHeader");

    if (FSettingsINI->ReadBool(L"CodeCompletion", L"EnableShiftCtrlArrowJumpback", true))
    {
        // Check if the file exists
        if (FileExists(FJumpbackDataHeader.FileName))
        {
            // Open the file and go to line
            IDE::ShowEditorLine(
                FJumpbackDataHeader.FileName,
                FJumpbackDataHeader.Line,
                FJumpbackDataHeader.Column
                );
        }
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::PerformBrowsingRequest()
{
    CS_SEND(L"CodeInsightManager::PerformBrowsingRequest");

    // Get the symbol text for the current cursor position
    String SymText = IDE::GetSymbolNameAtCursorPos();

    // Get the symbol tag
    Ctags::TTag Symbol = FProjectDB.GetPosSymbol(SymText);

    // Check if the file exists
    if (FileExists(Symbol.File))
    {
        // Open the file and go to line
        IDE::ShowEditorLine(Symbol.File, Symbol.LineNo);
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::PerformSymbolHintRequest()
{
    CS_SEND(L"CodeInsightManager::PerformSymbolHintRequest");

    // Get the colors
    TColor SymbolHintBorderColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBorderColor", 0x0063C0F1)
        );

    TColor SymbolHintBackgroundColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBackgroundColor", 0x00CBEFFF)
        );

    // Get the symbol text below the current mouse position
    String SymText = IDE::GetSymbolNameAtMousePos();

    // Get the symbol tag
    Ctags::TTag Symbol = FProjectDB.GetPosSymbol(SymText);

    if (!Symbol.Name.IsEmpty())
    {
        FSymbolHintForm->lblName->Caption       = Symbol.QualifiedName;
        FSymbolHintForm->lblKind->Caption       = Symbol.Kind;

        FSymbolHintForm->Color                          = SymbolHintBorderColor;
        FSymbolHintForm->spDividerTop->Pen->Color       = Environment::GetDarkerColor(SymbolHintBorderColor, 20);
        FSymbolHintForm->spDividerBottom->Pen->Color    = SymbolHintBorderColor;
        FSymbolHintForm->pnlValues->Color               = SymbolHintBackgroundColor;

        if (Symbol.Kind == L"function")
        {
            FSymbolHintForm->lblReturns->Caption    = Symbol.Typeref_B.IsEmpty() ? String(L"-") : Symbol.Typeref_B;
            FSymbolHintForm->lblType->Caption       = L"-";
        }
        else
        {
            FSymbolHintForm->lblReturns->Caption    = L"-";
            FSymbolHintForm->lblType->Caption       = Symbol.Typeref_B.IsEmpty() ? String(L"-") : Symbol.Typeref_B;
        }

        FSymbolHintForm->lblFile->Caption       = ExtractFileName(Symbol.File);
        FSymbolHintForm->lblLine->Caption       = String(Symbol.LineNo);

        // Get the symbol icon...
        boost::shared_ptr<TPngImage> Icon = GetTypeIconByClassText(Symbol.Kind);

        FSymbolHintForm->imgSymbol->Picture->Assign(Icon.get());

        // Note: We set the form's position in the 'OnShow' handler due to problems occuring otherwise
        FSymbolHintForm->Show();
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::PerformParameterHintRequest()
{
    CS_SEND(L"CodeInsightManager::PerformParameterHintRequest");

    // Get the colors
    TColor HintBorderColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBorderColor", 0x0063C0F1)
        );

    TColor HintBackgroundColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolHintBackgroundColor", 0x00CBEFFF)
        );

    TColor HintTextColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"TextColor", 0x00000000)
        );

    // Get the font data
    String FontName = FSettingsINI->ReadString(
        L"CodeCompletion",
        L"FontName",
        L"Segoe UI"
        );

    int FontSize = FSettingsINI->ReadInteger(
        L"CodeCompletion",
        L"FontSize",
        9
        );

    int Line        = 1;
    int Column      = 1;
    String FileName = L"";

    // Get the current editor caret pos
    IDE::GetCurrentEditorPos(Line, Column, FileName);

    // Extract the qualified function name with all operators replaces with '.'
    String FunctionName = IDE::GetFunctionNameAtEditorPos(Line, Column, true);

    if (!FunctionName.IsEmpty())
    {
        FCommonHintForm->SetColors(HintBorderColor, HintBackgroundColor, HintTextColor);
        FCommonHintForm->SetFont(FontName, FontSize);

        FCommonHintForm->ClearParams();

        // Create a vector from each symbol part
        VString Symbols = Environment::SplitStr(FunctionName, L'.');

        Ctags::TTag Tag = FProjectDB.GetPosSymbol(Symbols.back());

        /*

        // Begin of interlock
        {
            TChBldLockGuard LG(FProjectDB.UpdateMutex);

            // Resolve the classes/structs for the function name
            for (std::size_t i = 0; i < Symbols.size() - 1; ++i)
            {
                // Perform the invocation
                PerformInvocation(
                    Symbols[i],
                    Line,
                    Column,
                    FileName,
                    ivkDot,
                    MatchingIdentifiers,
                    false
                    );

                foreach_ (Ctags::TTag Identifier, MatchingIdentifiers)



                // If it's not the last symbol in the row...
                if (i < (Symbols.size() - 1))
                {
                    // ...find the entries with the current symbol name
                    foreach_ (Ctags::TTag Identifier, MatchingIdentifiers)
                    {
                        if (Identifier.Name == Symbols[i+1])
                    }
                }

            }
        }
        // End of interlock

        */

        //for (std::size_t i = 0; i < MatchingIdentifiers.size(); ++i)
        //{
        //    Ctags::TTag Tag = MatchingIdentifiers[i];
        //}

        if (!Tag.Signature.Trim().IsEmpty())
        {
            VString Params = Environment::SplitStr(Tag.Signature, L',');

            FCommonHintForm->AddParams(0, Params);

            int X;
            int Y;

            // Get the absolute pixel position of the editor caret pos
            if (IDE::GetEditorCaretPixelPos(X, Y))
            {
                FCommonHintForm->Left   = X;
                FCommonHintForm->Top    = Y;

                // Show common hint form without giving it the focus
                ShowWindow(FCommonHintForm->Handle, SW_SHOWNOACTIVATE);
                FCommonHintForm->Visible = true;
            }
        }
    }
}
//---------------------------------------------------------------------------

TChBldInvocationOperator TChBldCodeInsightManager::GetInvocationOperatorAndToken(
    const String& Str,
    String& CodePoint
    )
{
    CS_SEND(L"CodeInsightManager::GetInvocationOperatorAndToken");

    TChBldInvocationOperator InvocationOperator = ivkUnknown;
    String CdPoint = L"";

    if (RightStr(Str, 1) == L".")
    {
        CdPoint = LeftStr(Str, Str.Length() - 1);

        InvocationOperator = ivkDot;
    }

    if (RightStr(Str, 2) == L"->")
    {
        CdPoint = LeftStr(Str, Str.Length() - 2);

        InvocationOperator = ivkArrow;
    }

    if (RightStr(Str, 2) == L"::")
    {
        CdPoint = LeftStr(Str, Str.Length() - 2);

        InvocationOperator = ivkDoubleColon;
    }

    CodePoint = Environment::ExtractToken(CdPoint);

    return InvocationOperator;
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::PerformInvocation(
    const String& Token,
    const int& Line,
    const int& Column,
    const String& FileName,
    TChBldInvocationOperator InvocationOperator,
    Ctags::VTag& MatchingIdentifiers,
    bool MustMatchInvocationOperator
    )
{
    VString Namespaces;
    VString ClassesAndStructs;

    // Get the namespaces inclusions for the current position
    FProjectDB.GetPosNamespaces(FileName, Line, Column, Namespaces);

    // Get the class and struct inclusions for the current position
    FProjectDB.GetPosClassesAndStructs(FileName, Line, Column, ClassesAndStructs);

    // If token is not empty...
    if (!Token.IsEmpty())
    {
        // ...we handle the invocation...
        HandleInvocation(
            InvocationOperator,
            Token,
            MatchingIdentifiers,
            Namespaces,
            ClassesAndStructs,
            MustMatchInvocationOperator
            );
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::HandleInvocation(
    TChBldInvocationOperator InvocationOperator,
    const String& Token,
    Ctags::VTag& MatchingIdentifiers,
    const VString& Namespaces,
    const VString& ClassesAndStructs,
    bool MustMatchInvocationOperator
    )
{
    Ctags::VTag Matches;
    String ObjectType       = L"";
    String NamespaceIdent   = L"";

    // Make a single string from the namespaces
    foreach_ (String Namespace, Namespaces)
        NamespaceIdent += L"::" + Namespace;

    // Cut off the leading '::'
    if (!NamespaceIdent.IsEmpty())
        NamespaceIdent = RightStr(NamespaceIdent, NamespaceIdent.Length() - 2);

    // Get token object type
    FProjectDB.GetMatchingIdentifierList(
        L"SELECT * FROM Common "
            L"WHERE TagName = '" + Token + L"' "
            L"AND Namespace = '" + NamespaceIdent + L"';",
        Matches
        );

    if (Matches.size() > 0)
    {
        // Get the Typeref
        String TyperefB = Matches[0].Typeref_B.Trim();

        // Check the invocation operators and cancel the invocation if they doesn't fit to the type
        if (MustMatchInvocationOperator)
        {
            if (RightStr(TyperefB, 1) == L"*")
            {
                if (InvocationOperator != ivkArrow)
                    return;
            }
            else if (RightStr(TyperefB, 1) == L"&")
            {
                if (InvocationOperator != ivkDot)
                    return;
            }
            else
            {
                if ((InvocationOperator != ivkDot) && (InvocationOperator != ivkDoubleColon))
                    return;
            }
        }

        // Delete some unneeded characters
        ObjectType = StringReplace(TyperefB, L"&", L"", TReplaceFlags() << rfReplaceAll);
        ObjectType = StringReplace(ObjectType, L"*", L"", TReplaceFlags() << rfReplaceAll);
        ObjectType = ObjectType.Trim();
    }

    if (!ObjectType.IsEmpty())
    {
        // Clear the known tags list
        FKnownTags.clear();

        // Get the matching object identifier
        GetMatchingObjectIdentifiers(ObjectType, MatchingIdentifiers);
    }
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::GetMatchingObjectIdentifiers(
    const String& ObjectType,
    Ctags::VTag& MatchingIdentifiers
    )
{
    CS_SEND(L"ChBldCodeInsightManager::GetMatchingObjectIdentifiers(" + ObjectType + L")");

    Ctags::VTag Matches;

    bool ShowPrivateMembers     = FSettingsINI->ReadBool(L"CodeCompletion", L"ShowPrivateMembers", true);
    bool ShowImplementations    = FSettingsINI->ReadBool(L"CodeCompletion", L"ShowImplementations", false);

    // Get public/__published members of object to show in completion list
    FProjectDB.GetMatchingIdentifierList(
        L"SELECT * FROM Common "
            L"WHERE (Class    LIKE '%" + ObjectType + L"' "
            L"OR    Struct    LIKE '%" + ObjectType + L"' "
            L"OR    Namespace LIKE '%" + ObjectType + L"') "
            L"AND   ((Access = 'public') OR (Access = 'protected') "
            + (ShowPrivateMembers ? L"OR (Access = 'private'))" : L")") +
            L"AND   Kind <> 'constructor' "
            + (ShowImplementations ? L";" : L"AND   Kind <> 'implementation';"),
        MatchingIdentifiers,
        false
        );

    // Get the class/struct definition itself to determine if there are ancestors
    FProjectDB.GetMatchingIdentifierList(
        L"SELECT * FROM Common "
            L"WHERE (QualifiedName  LIKE '%" + ObjectType + L"' "
            L"AND "
            L"(Kind = 'class' OR Kind = 'struct')"
            L");",
        Matches
        );

    // If we have a class/struct definition...
    if (Matches.size() > 0)
    {
        Ctags::TTag &Tag = Matches[0];

        // ...and if our object is derived from one or more classes/structs...
        if (!Tag.Inherits.IsEmpty())
        {
            // ...read and split the ancestor field values (C++ supports multiple inheritance!!)
            VString Ancestors = Environment::SplitCtagsObjectAncestors(Tag.Inherits);

            // Iterate recursively over each inherited object type
            foreach_ (String Ancestor, Ancestors)
            {
                GetMatchingObjectIdentifiers(Ancestor, MatchingIdentifiers);
            }
        }
    }
}
//---------------------------------------------------------------------------

boost::shared_ptr<TPngImage> TChBldCodeInsightManager::GetTypeIconByClassText(const String& ClassText)
{
    // Get the current icon set
    int IconSet = FSettingsINI->ReadInteger(L"CodeCompletion", L"IconSet", 0);

    // Get the current symbol set from the icon set
    TChBldInvocationIconSet &Symbols = FCCIcons[IconSet];

    if ((ClassText == L"class")
        || (ClassText == L"struct")
        || (ClassText == L"constructor")
        || (ClassText == L"destructor"))
        return Symbols.Class;
    else if (ClassText == L"function")
        return Symbols.Method;
    else if ((ClassText == L"variable") || (ClassText == L"local"))
        return Symbols.Field;
    else if (ClassText == L"implementation")
        return Symbols.Word;
    else if (ClassText == L"property")
        return Symbols.Attribute;
    else if (ClassText == L"externvar")
        return Symbols.GlobalIdent;
    else if (ClassText == L"macro")
        return Symbols.Tag;
    else if (ClassText == L"typedef")
        return Symbols.Unit;
    else if (ClassText == L"enum")
        return Symbols.Enum;
    else if (ClassText == L"enumerator")
        return Symbols.Circle;
    else
        return Symbols.Keyword;
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::DrawListItem(const int Index, TCanvas* Canvas, const TRect& Rect)
{
    // Increase the height of Rect' by one line
    TRect SelRect = Rect;
    SelRect.BottomRight().y++;

    // Get the content for the current index
    String ClassText        = FInvocationData.SymbolList->SymbolClassText[Index];
    String FilterText       = FInvocationData.SymbolList->FilterText;
    String Text             = FInvocationData.SymbolList->SymbolText[Index];
    String TypeText         = FInvocationData.SymbolList->SymbolTypeText[Index];
    int Visibility          = FInvocationData.SymbolList->SymbolVisibility[Index];
    String LongestClassText = FInvocationData.SymbolList->GetLongestItem();

    // Get the icon setting
    bool UseIcons = FSettingsINI->ReadInteger(L"CodeCompletion", L"UseIcons", true);

    // Get the colors
    TColor BackgroundColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"BackgroundColor", 0x00FFFFFF)
        );

    TColor TextColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"TextColor", 0x00FF0000)
        );

    TColor SymbolColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SymbolColor", 0x00FF0000)
        );

    TColor HighlightColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"HighlightColor", 0x00FF0000)
        );

    TColor SelectionBorderColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SelectionBorderColor", 0x0063C0F1)
        );

    TColor SelectionBackgroundColor = static_cast<TColor>(
        FSettingsINI->ReadInteger(L"CodeCompletion", L"SelectionBackgroundColor", 0x00CBEFFF)
        );

    // Set the general canvas styling
    Canvas->Font->Name  = FSettingsINI->ReadString(L"CodeCompletion", L"FontName", L"Segoe UI");
    Canvas->Font->Size  = FSettingsINI->ReadInteger(L"CodeCompletion", L"FontSize", 9);

    int MaxClassTextWidth   = Canvas->TextWidth(LongestClassText);
    int MaxLetterWidth      = Canvas->TextWidth(L"W");

    _di_IOTACodeInsightServices CIServices = IDE::GetInterface<_di_IOTACodeInsightServices>();
    _di_IOTACodeInsightViewer   CIViewer;

    // Get the current code insight viewer
    CIServices->GetViewer(CIViewer);

    // Get the 'IsSelected' state
    bool IsSelected = (CIViewer->GetManagerSelectedIndex(this) == Index);

    Canvas->Brush->Style = bsSolid;

    if (IsSelected)
    {
        // Set the selection rect colors
        Canvas->Pen->Color      = SelectionBorderColor;
        Canvas->Brush->Color    = SelectionBackgroundColor;

        // Draw selection rect...
        if (Index > 0)
        {
            Canvas->Rectangle(SelRect);
        }
        else
        {
            // ...if the first item is selected, draw the 'limited' border, because the 'extended'
            //    one gets cutted off at the bottom by the first complete redraw of the selection list
            Canvas->Rectangle(Rect);
        }
    }
    else
    {
        // Set the background color
        Canvas->Brush->Color = BackgroundColor;

        // Fill the background
        Canvas->FillRect(SelRect);
    }

    Canvas->Brush->Style = bsClear;

    // Set the selected text color to the chosen one if we have a public, protected member or a variable
    if ((Visibility == vvfPublic) || (Visibility == vvfProtected))
    {
        Canvas->Font->Color = TextColor;
    }
    // Lighten up the text color if we have a private member
    else
    {
        Canvas->Font->Color = Environment::GetLighterColor(TextColor);
    }

    if (!UseIcons)
    {
        // Draw the class text
        Canvas->TextOutW(SelRect.Left, SelRect.Top, ClassText);
    }
    else
    {
        // Draw the class icon
        boost::shared_ptr<TPngImage> Icon = GetTypeIconByClassText(ClassText);

        Icon->Draw(Canvas, TRect(SelRect.Left, SelRect.Top - 2, SelRect.Left + 20, SelRect.Top - 2 + 20));

        MaxClassTextWidth   = 21;
        MaxLetterWidth      = 0;
    }

    Canvas->Font->Style = Canvas->Font->Style << fsBold;

    // Draw the text
    Canvas->TextOutW(
        SelRect.Left + MaxClassTextWidth + (MaxLetterWidth * 4),
        SelRect.Top,
        Text
        );

    // Set the selected highlight color
    Canvas->Font->Color = HighlightColor;

    // Overpaint the filter text ('FilterText' is lower case thus this string handling)
    Canvas->TextOutW(
        SelRect.Left + MaxClassTextWidth + (MaxLetterWidth * 4),
        SelRect.Top,
        Text.SubString(1, FilterText.Length())
        );

    int TextWidth = Canvas->TextWidth(Text);

    Canvas->Font->Style = Canvas->Font->Style >> fsBold;

    // Set the selected type text color to the chosen one if we have a public, protected member or a variable
    if ((Visibility == vvfPublic) || (Visibility == vvfProtected))
    {
        Canvas->Font->Color = SymbolColor;
    }
    // Lighten up the type text color if we have a private member
    else
    {
        Canvas->Font->Color = Environment::GetLighterColor(SymbolColor);
    }

    Canvas->TextOutW(
        SelRect.Left + MaxClassTextWidth + (MaxLetterWidth * 4) + TextWidth + 1,
        SelRect.Top,
        ReplaceWithTypeAbbreviations(TypeText)
        );
}
//---------------------------------------------------------------------------

String TChBldCodeInsightManager::ReplaceWithTypeAbbreviations(String Text)
{
    std::map<String, String> TypeAbbrevs;

    VString Abbrevs = Environment::SplitStr(
                                        FSettingsINI->ReadString(
                                            L"CodeCompletion",
                                            L"KnownTypeAbbreviations",
                                            L""
                                            ),
                                        L'\t'
                                        );

    foreach_ (String Abbrev, Abbrevs)
        TypeAbbrevs.insert(Environment::SplitToPair(Abbrev, L'='));

    std::pair<String, String> Pair;

    foreach_ (Pair, TypeAbbrevs)
        Text = StringReplace(Text, Pair.first, Pair.second, TReplaceFlags() << rfReplaceAll);

    return Text;
}
//---------------------------------------------------------------------------

void TChBldCodeInsightManager::LoadIcons()
{
    TChBldInvocationIconSet IconSet;

    // CherryBuider Lite
    IconSet.At = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.At->LoadFromResourceName((int)HInstance, L"cc_lite_at");

    IconSet.Attribute = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Attribute->LoadFromResourceName((int)HInstance, L"cc_lite_attribute");

    IconSet.Circle = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Circle->LoadFromResourceName((int)HInstance, L"cc_lite_circle");

    IconSet.Class = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Class->LoadFromResourceName((int)HInstance, L"cc_lite_class");

    IconSet.Cut = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Cut->LoadFromResourceName((int)HInstance, L"cc_lite_cut");

    IconSet.Enum = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Enum->LoadFromResourceName((int)HInstance, L"cc_lite_enum");

    IconSet.Field = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Field->LoadFromResourceName((int)HInstance, L"cc_lite_field");

    IconSet.GlobalIdent = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.GlobalIdent->LoadFromResourceName((int)HInstance, L"cc_lite_globalident");

    IconSet.Interface = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Interface->LoadFromResourceName((int)HInstance, L"cc_lite_interface");

    IconSet.Keyword = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Keyword->LoadFromResourceName((int)HInstance, L"cc_lite_keyword");

    IconSet.Method = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Method->LoadFromResourceName((int)HInstance, L"cc_lite_method");

    IconSet.Tag = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Tag->LoadFromResourceName((int)HInstance, L"cc_lite_tag");

    IconSet.Unit = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Unit->LoadFromResourceName((int)HInstance, L"cc_lite_unit");

    IconSet.Word = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Word->LoadFromResourceName((int)HInstance, L"cc_lite_word");

    FCCIcons.push_back(IconSet);

    // CherryBuilder Dark
    IconSet.At = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.At->LoadFromResourceName((int)HInstance, L"cc_dark_at");

    IconSet.Attribute = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Attribute->LoadFromResourceName((int)HInstance, L"cc_dark_attribute");

    IconSet.Circle = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Circle->LoadFromResourceName((int)HInstance, L"cc_dark_circle");

    IconSet.Class = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Class->LoadFromResourceName((int)HInstance, L"cc_dark_class");

    IconSet.Cut = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Cut->LoadFromResourceName((int)HInstance, L"cc_dark_cut");

    IconSet.Enum = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Enum->LoadFromResourceName((int)HInstance, L"cc_dark_enum");

    IconSet.Field = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Field->LoadFromResourceName((int)HInstance, L"cc_dark_field");

    IconSet.GlobalIdent = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.GlobalIdent->LoadFromResourceName((int)HInstance, L"cc_dark_globalident");

    IconSet.Interface = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Interface->LoadFromResourceName((int)HInstance, L"cc_dark_interface");

    IconSet.Keyword = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Keyword->LoadFromResourceName((int)HInstance, L"cc_dark_keyword");

    IconSet.Method = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Method->LoadFromResourceName((int)HInstance, L"cc_dark_method");

    IconSet.Tag = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Tag->LoadFromResourceName((int)HInstance, L"cc_dark_tag");

    IconSet.Unit = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Unit->LoadFromResourceName((int)HInstance, L"cc_dark_unit");

    IconSet.Word = boost::shared_ptr<TPngImage>(new TPngImage);
    IconSet.Word->LoadFromResourceName((int)HInstance, L"cc_dark_word");

    FCCIcons.push_back(IconSet);
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder


