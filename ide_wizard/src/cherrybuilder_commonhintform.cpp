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

#include "cherrybuilder_commonhintform.h"

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------

__fastcall TCommonHintForm::TCommonHintForm(TComponent* Owner)
    :   TForm(Owner),
        FLineHeight(16),
        FHighlightedParameter(0, 0) ,
        FBorderColor(static_cast<TColor>(0x00C8C8C8)),
        FBackgroundColor(static_cast<TColor>(0x00F2EFEF)),
        FTextColor(clBlack)
{
}
//---------------------------------------------------------------------------

void __fastcall TCommonHintForm::FormShow(TObject *Sender)
{
    // Read the initial key states
    for (int i = 0; i < 256; ++i)
        FKeyStates[i] = GetAsyncKeyState(i);
}
//---------------------------------------------------------------------------

void TCommonHintForm::SetFont(const String& Name, int Size)
{
    pbHintForm->Canvas->Font->Name      = Name;
    pbHintForm->Canvas->Font->Size      = Size;

    Canvas->Font->Name = Name;
    Canvas->Font->Size = Size;

    Font->Name = Name;
    Font->Size = Size;

    FLineHeight = Canvas->TextHeight(L"W") + 2;

    pbHintForm->Invalidate();
}
//---------------------------------------------------------------------------

void TCommonHintForm::SetColors(
    TColor BorderColor,
    TColor BackgroundColor,
    TColor TextColor
    )
{
    FBorderColor        = BorderColor;
    FBackgroundColor    = BackgroundColor;
    FTextColor          = TextColor;

    Color = FBorderColor;

    pbHintForm->Invalidate();
}
//---------------------------------------------------------------------------

void TCommonHintForm::ClearParams()
{
    FParamLines.clear();

    pbHintForm->Invalidate();
}
//---------------------------------------------------------------------------

void TCommonHintForm::AddParams(int Line, VString& Params)
{
    FParamLines[Line] = Params;

    // Set the window height
    ClientHeight = (FParamLines.size() * FLineHeight) + 3;

    pbHintForm->Invalidate();
}
//---------------------------------------------------------------------------

void TCommonHintForm::SetHighlightedParameter(int Line, int ParamNo)
{
    FHighlightedParameter.X = ParamNo;
    FHighlightedParameter.Y = Line;

    pbHintForm->Invalidate();
}
//---------------------------------------------------------------------------

void __fastcall TCommonHintForm::pbHintFormPaint(TObject *Sender)
{
    // Get the canvas
    TCanvas *Cnv = pbHintForm->Canvas;

    // Set the brush parameters
    Cnv->Brush->Color = FBackgroundColor;
    Cnv->Brush->Style = bsSolid;

    // Fill the background
    Cnv->FillRect(pbHintForm->ClientRect);

    // Set the brush style to clear
    Cnv->Brush->Style = bsClear;

    std::pair<int, VString > Line;

    int WindowWidth = 0;

    // Iterate over each text line to draw
    foreach_ (Line, FParamLines)
    {
        int ParamStartPos = 0;

        for (std::size_t i = 0; i < Line.second.size(); ++i)
        {
            // Get the current parameter text
            String Text = Line.second[i];

            if (i < Line.second.size() - 1)
                Text += L", ";

            if (    (static_cast<std::size_t>(FHighlightedParameter.X) == i)
                 && (FHighlightedParameter.Y == Line.first) )
            {
                Cnv->Font->Style = Cnv->Font->Style << fsBold;
            }
            else
            {
                Cnv->Font->Style = Cnv->Font->Style >> fsBold;
            }

            int ParamTextWidth = Cnv->TextWidth(Text);

            // Draw the parameter text
            Cnv->TextOutW(2 + ParamStartPos, 2 + (Line.first * FLineHeight), Text);

            ParamStartPos += ParamTextWidth;
        }

        if (ParamStartPos > WindowWidth)
            WindowWidth = ParamStartPos + 2;
    }

    // We have to set the new window width depending on the longest parameter line
    Width = Margins->Left + Margins->Right + WindowWidth;
}
//---------------------------------------------------------------------------

void __fastcall TCommonHintForm::tiKeyActivityTimer(TObject *Sender)
{
    if (Visible)
    {
        // Check each key code
        for (int i = 0; i < 256; ++i)
        {
            // Check if a key has changed it's state...
            if (FKeyStates[i] != GetAsyncKeyState(i))
            {
                // ...and handle a key press in this case
                HandleKeyPress(i);
            }
        }
    }
}
//---------------------------------------------------------------------------

void TCommonHintForm::HandleKeyPress(int VirtualKeyCode)
{
    switch (VirtualKeyCode)
    {
        case VK_ESCAPE:
            Visible = false;
        break;
    }
}
//---------------------------------------------------------------------------

