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

#ifndef cherrybuilder_commonhintformH
#define cherrybuilder_commonhintformH
//---------------------------------------------------------------------------

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>

#include <vector>
#include <map>

#include "cherrybuilder_environment.h"
//---------------------------------------------------------------------------

using namespace Cherrybuilder;

class TCommonHintForm : public TForm
{
__published:
    TPaintBox *pbHintForm;
    TTimer *tiKeyActivity;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall pbHintFormPaint(TObject *Sender);
    void __fastcall tiKeyActivityTimer(TObject *Sender);

public:
    __fastcall TCommonHintForm(TComponent* Owner);

    void SetFont(const String& Name, int Size);
    void SetColors(TColor BorderColor, TColor BackgroundColor, TColor TextColor);
    void ClearParams();
    void AddParams(int Line, VString& Params);
    void SetHighlightedParameter(int Line, int ParamNo);

private:
    void HandleKeyPress(int VirtualKeyCode);

    std::map<int, VString > FParamLines;
    int FLineHeight;
    TPoint FHighlightedParameter;
    TColor FBorderColor;
    TColor FBackgroundColor;
    TColor FTextColor;

    short FKeyStates[256];
};
//---------------------------------------------------------------------------

#endif
