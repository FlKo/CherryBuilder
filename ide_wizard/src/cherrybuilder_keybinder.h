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

#ifndef cherrybuilder_keybinderH
#define cherrybuilder_keybinderH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include "cherrybuilder_codeinsightmanager.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class TChBldKeyBinder : public TCppInterfacedObject<IOTAKeyboardBinding>
{
public:
    TChBldKeyBinder(TChBldCodeInsightManager* CodeInsightManager);

    /* IOTAKeyboardBinding */
    TBindingType    __fastcall GetBindingType();
    String          __fastcall GetDisplayName();
    String          __fastcall GetName();
    void            __fastcall BindKeyboard(const _di_IOTAKeyBindingServices BindingServices);

	__property TBindingType BindingType = {read=GetBindingType};
	__property System::UnicodeString DisplayName = {read=GetDisplayName};
	__property System::UnicodeString Name = {read=GetName};

void __fastcall JumpToImplementation(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    );

void __fastcall JumpToHeader(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    );

void __fastcall DoJumpbackImplementation(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    );

void __fastcall DoJumpbackHeader(
    const _di_IOTAKeyContext Context,
    TShortCut KeyCode,
    TKeyBindingResult &BindingResult
    );

private:

   TChBldCodeInsightManager *FCodeInsightManager;

    /* IOTANotifier */
    void __fastcall AfterSave();
    void __fastcall BeforeSave();
    void __fastcall Destroyed();
    void __fastcall Modified();

};
} // namespace Cherrybuilder

#endif
