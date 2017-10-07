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

#ifndef cherrybuilder_ideH
#define cherrybuilder_ideH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include <vector>
#include <memory>
#include <map>

#include <System.Win.Registry.hpp>

#include "cherrybuilder_environment.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

class IDE
{
public:
    static void SetIDEServices(_di_IBorlandIDEServices AIDESrv) { FIDESrv = AIDESrv; }

    template <typename T>
    static T GetInterface()
    {
        T SmartIntf;

        if (FIDESrv->Supports(SmartIntf))
            return SmartIntf;
        else
            throw Exception(L"Error: Interface not supported by 'IOTABorlandIDEServices'");
    }

    template <typename T1, typename T2>
    static T1 GetInterface(T2 BaseIntf)
    {
        T1 SmartIntf;

        if (BaseIntf->Supports(SmartIntf))
            return SmartIntf;
        else
            throw Exception(L"Error: Interface not supported");
    }

    static TMenuItem* AddToolsMenuItem(
        const String& ACaption,
        TNotifyEvent AOnClick,
        const String& AResourceImageName
        );

    static void     GetCurrentPlatformIncludePaths(VString& Paths,
                        bool GetProjectPathsInstead=false
                        );

    static String   GetExpandedPath(String Path);
    static String   GetNormalizedPath(const String& Path);
    static String   GetShortPath(const String& Path);

    static String   GetCurrentTargetOS();
    static String   GetCurrentPlatform();
    static int      GetCurrentEditorCppTabWidth();
    static String   GetCurrentEditorFontName();
    static int      GetCurrentEditorFontSize();
    static bool     GetCurrentEditorPos(int& Line, int& Column, String& FileName);

    static _di_IOTAEditActions GetCurrentEditActions();

    static RawByteString    GetEditorContentUTF8(_di_IOTASourceEditor SourceEditor);
    static String           GetEditorContent(_di_IOTASourceEditor SourceEditor);

    static void     ExtractAllEditorsContent(
                        const String& ProjectPath,
                        std::map<String, String>& Results,
                        std::map<String, String>* ChangedFiles=NULL
                        );

    static void     RemoveOldTempFiles(const String& ProjectPath);

    static bool     GetEditorMousePos(int& Line, int& Column);
    static bool     GetEditorCaretPixelPos(int& X, int& Y);

    static String   GetNormalizedEditorLineText(const int& Line);

    static String   GetSymbolNameAtEditorPos(const int& Line, const int& Column);
    static String   GetFunctionNameAtEditorPos(const int& Line, const int& Column, bool ReplaceOperators=false);

    static String   GetSymbolNameAtCursorPos();
    static String   GetSymbolNameAtMousePos();

    static void     ShowEditorLine(const String& FileName, int Line, int Column=1);

    static TOTASortOrder    GetCurrentSymbolListSortOrder();

private:
    static _di_IBorlandIDEServices FIDESrv;
};

} // namespace Cherrybuilder

#endif


