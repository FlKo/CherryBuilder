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

#ifndef cherrybuilder_ctagsH
#define cherrybuilder_ctagsH
//---------------------------------------------------------------------------

#include <System.SysUtils.hpp>

#include <ToolsAPI.hpp>
#include <PlatformAPI.hpp>

#include <vector>
#include <memory>

#include "cherrybuilder_environment.h"
#include "cherrybuilder_ide.h"
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

namespace Ctags
{

struct TIncludeTag
{
    String Name;
    String File;
    String Address;

    String Kind;
};
//---------------------------------------------------------------------------

struct TTag
{
    String Name;
    String QualifiedName;
    String File;
    String Address;

    String Kind;
    int    LineNo;
    String Namespace;
    String Class;
    String Struct;
    String Access;
    String Implementation;
    String Signature;
    String Typeref_A;
    String Typeref_B;
    String Inherits;
};
//---------------------------------------------------------------------------

typedef std::vector<TIncludeTag>    VIncludeTag;
typedef std::vector<TTag>           VTag;
//---------------------------------------------------------------------------

class TParser
{
public:
    TParser(const String& CtagsExe);
    ~TParser();

    void    SetProjectPath(const String& ProjectPath);

    void    SetIdeIncludePaths(const VString& Paths);
    void    SetProjectIncludePaths(const VString& Paths);

    void    ParseIncludes(VString& Queue, VString& Includes);
    void    FullParseIncludes(
                VString& Queue,
                VString& Includes
                );

    void    ParseTags(const VString& Queue, VTag& Results);

private:
    bool    InterpretIncludeData(const String& LineText, TIncludeTag& Record);
    bool    InterpretLineData(const String& LineText, TTag& Record);

    String FCtagsExe;
    String FProjectPath;

    VString FRelatedFileExtensions;
    VString FIDEIncludePaths;
    VString FProjectIncludePaths;
};

} // namespace Ctags

} // namespace Cherrybuilder

#endif


