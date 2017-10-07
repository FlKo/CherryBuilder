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

#include "cherrybuilder_keybinder.h"

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

TChBldKeyBinder::TChBldKeyBinder(TChBldCodeInsightManager* CodeInsightManager)
    :   FCodeInsightManager(CodeInsightManager)
{

}
//---------------------------------------------------------------------------

TBindingType __fastcall TChBldKeyBinder::GetBindingType()
{
    CS_SEND(L"KeyBinder::GetBindingType");

    return btPartial;
}
//---------------------------------------------------------------------------

String __fastcall TChBldKeyBinder::GetDisplayName()
{
    CS_SEND(L"KeyBinder::GetDisplayName");    

    return L"CherryBuilder C++ Bindings";
}
//---------------------------------------------------------------------------

String __fastcall TChBldKeyBinder::GetName()
{
    CS_SEND(L"KeyBinder::GetName");

    return L"CherryBuilderCppBindings";
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::BindKeyboard(const _di_IOTAKeyBindingServices BindingServices)
{
    CS_SEND(L"KeyBinder::BindKeyboard");

    TShortCut   CtrlShiftDown[1];
    TShortCut   CtrlShiftUp[1];
    TShortCut   CtrlShiftLeft[1];
    TShortCut   CtrlShiftRight[1];

    CtrlShiftDown[0]    = vkDown | scShift | scCtrl;
    CtrlShiftUp[0]      = vkUp | scShift | scCtrl;
    CtrlShiftLeft[0]    = vkLeft | scShift | scCtrl;
    CtrlShiftRight[0]   = vkRight | scShift | scCtrl;

    BindingServices->AddKeyBinding(CtrlShiftDown, 0, JumpToImplementation, NULL);
    BindingServices->AddKeyBinding(CtrlShiftUp, 0, JumpToHeader, NULL);
    BindingServices->AddKeyBinding(CtrlShiftLeft, 0, DoJumpbackImplementation, NULL);
    BindingServices->AddKeyBinding(CtrlShiftRight, 0, DoJumpbackHeader, NULL);
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::JumpToImplementation(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    )
{
    CS_SEND(L"KeyBinder::JumpToImplementation");

    _di_IOTAModuleServices ModuleServices = IDE::GetInterface<_di_IOTAModuleServices>();
    _di_IOTAModule Module = ModuleServices->CurrentModule();
    _di_IOTAEditor Editor = Module->CurrentEditor;

    _di_IOTASourceEditor SourceEditor;

    if (Editor->Supports(SourceEditor))  
    {
        String FileName = SourceEditor->GetFileName();

        if (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName))
            FCodeInsightManager->JumpToImplementation();
    }    
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::JumpToHeader(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    )
{
    CS_SEND(L"KeyBinder::JumpToHeader");

    _di_IOTAModuleServices ModuleServices = IDE::GetInterface<_di_IOTAModuleServices>();
    _di_IOTAModule Module = ModuleServices->CurrentModule();
    _di_IOTAEditor Editor = Module->CurrentEditor;

    _di_IOTASourceEditor SourceEditor;

    if (Editor->Supports(SourceEditor))  
    {
        String FileName = SourceEditor->GetFileName();

        if (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName))
            FCodeInsightManager->JumpToHeader();
    }      
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::DoJumpbackImplementation(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    )
{
    _di_IOTAModuleServices ModuleServices = IDE::GetInterface<_di_IOTAModuleServices>();
    _di_IOTAModule Module = ModuleServices->CurrentModule();
    _di_IOTAEditor Editor = Module->CurrentEditor;

    _di_IOTASourceEditor SourceEditor;

    if (Editor->Supports(SourceEditor))
    {
        String FileName = SourceEditor->GetFileName();

        if (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName))
            FCodeInsightManager->DoJumpbackImplementation();
    }
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::DoJumpbackHeader(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    )
{
    _di_IOTAModuleServices ModuleServices = IDE::GetInterface<_di_IOTAModuleServices>();
    _di_IOTAModule Module = ModuleServices->CurrentModule();
    _di_IOTAEditor Editor = Module->CurrentEditor;

    _di_IOTASourceEditor SourceEditor;

    if (Editor->Supports(SourceEditor))
    {
        String FileName = SourceEditor->GetFileName();

        if (Environment::IsCppFile(FileName) || Environment::IsHppFile(FileName))
            FCodeInsightManager->DoJumpbackHeader();
    }
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::AfterSave()
{
    CS_SEND(L"KeyBinder::AfterSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::BeforeSave()
{
    CS_SEND(L"KeyBinder::BeforeSave");
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::Destroyed()
{
    CS_SEND(L"KeyBinder::Destroyed");
}
//---------------------------------------------------------------------------

void __fastcall TChBldKeyBinder::Modified()
{
    CS_SEND(L"KeyBinder::Modified");
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder
