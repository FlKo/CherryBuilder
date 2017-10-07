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

#ifndef cherrybuilder_environmentH
#define cherrybuilder_environmentH
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include <vector>
#include <map>

#include <boost/foreach.hpp>
#ifndef foreach_
    #define foreach_    BOOST_FOREACH
#endif
#ifndef foreach_r_
    #define foreach_r_  BOOST_REVERSE_FOREACH
#endif
//---------------------------------------------------------------------------

#define COMPLETIONCHARSET  (TSysCharSet() \
        << 'A' << 'B' << 'C' << 'D' << 'E' << 'F' << 'G' << 'H' << 'I' << 'J' << 'K' \
        << 'L' << 'M' << 'N' << 'O' << 'P' << 'Q' << 'R' << 'S' << 'T' << 'U' << 'V' << 'W' \
        << 'X' << 'Y' << 'Z' \
        << 'a' << 'b' << 'c' << 'd' << 'e' << 'f' << 'g' << 'h' << 'i' << 'j' << 'k' << 'l' \
        << 'm' << 'n' << 'o' << 'p' << 'q' << 'r' << 's' << 't' << 'u' << 'v' << 'w' << 'x' \
        << 'y' << 'z' \
        << '0' << '1' << '2' << '3' << '4' << '5' << '6' << '7' << '8' << '9' \
        << '.' << '-' << '>' << ':' << '~' << '\x00' << '\x01' << '\x02' << '\x02')
//---------------------------------------------------------------------------

namespace Cherrybuilder
{

const String kDBFileName = L"chbld_tags.db";
//---------------------------------------------------------------------------

enum TMatchMode
{
	mmComplete = 0,
	mmBegin,
	mmEnd,
	mmContain
};
//---------------------------------------------------------------------------

typedef std::vector<String> VString;

struct TChBldLockGuard
{
    TChBldLockGuard(TMutex* AMutex)
        :   Mutex(AMutex)
    {
        Mutex->Acquire();
    }

    ~TChBldLockGuard()
    {
        Mutex->Release();
    }

    TMutex *Mutex;
};
//---------------------------------------------------------------------------

class Environment
{
public:
    static String       GetProgramDataFolderName() { return Environment::FProgramDataFolderName; }
    static String       GetSettingsFileExtension() { return Environment::FSettingsFileExtension; }

    static String       GetCurrentComputerName();
    static String       GetCurrentUserName();

    static String       GetCurrentSettingsFolder();
    static String       GetCurrentSettingsFileName();
    static String       GetCurrentSettingsFilePath();

    static String       GetDLLFolder();

    static String       CreateGuidString(bool NoParantheses=false);

    static void         ExecuteCmd(const String& CommandLine);

    static String       GetWinTempPath();

    static String       GetWinAPILastErrorText();

    static bool         IsCppFile(const String& File);
    static bool         IsHppFile(const String& File);

    static String       ExtractToken(const String& Text);

    static VString      SplitStr(
                            String Text,
                            wchar_t Delim1,
                            wchar_t Delim2=L'\0',
                            wchar_t Delim3=L'\0'
                            );

    static std::pair<String, String> SplitToPair(
                                        String Text,
                                        wchar_t Delim
                                        );

    static VString      SplitCtagsObjectAncestors(
                            String Text
                        );

    static TColor       GetLighterColor(const TColor& Color, const unsigned char BrightnessDiff=110);
    static TColor       GetDarkerColor(const TColor& Color, const unsigned char BrightnessDIff=110);

private:
    static const        String FProgramDataFolderName;
    static const        String FSettingsFileExtension;

};
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

#endif


