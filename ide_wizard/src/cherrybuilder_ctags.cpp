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

#include <System.RegularExpressions.hpp>

#include "cherrybuilder_ctags.h"

#include <System.StrUtils.hpp>

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)

namespace Cherrybuilder
{

namespace Ctags
{
TParser::TParser(const String& CtagsExe)
    :   FCtagsExe(CtagsExe),
        FProjectPath(L"")
{
    FRelatedFileExtensions.push_back(L".c");
    FRelatedFileExtensions.push_back(L".h") ;
    FRelatedFileExtensions.push_back(L".cpp");
    FRelatedFileExtensions.push_back(L".hpp");
    FRelatedFileExtensions.push_back(L".cc");
    FRelatedFileExtensions.push_back(L".hh");
    FRelatedFileExtensions.push_back(L".cxx");
    FRelatedFileExtensions.push_back(L".hxx");
    FRelatedFileExtensions.push_back(L".cp");
    FRelatedFileExtensions.push_back(L".hp");
    FRelatedFileExtensions.push_back(L".c++");
    FRelatedFileExtensions.push_back(L".h++");
}
//---------------------------------------------------------------------------

TParser::~TParser()
{
}
//---------------------------------------------------------------------------

void TParser::SetProjectPath(const String& ProjectPath)
{
    FProjectPath = ProjectPath;
}
//---------------------------------------------------------------------------

void TParser::SetIdeIncludePaths(const VString& Paths)
{
    FIDEIncludePaths = Paths;
}
//---------------------------------------------------------------------------

void TParser::SetProjectIncludePaths(const VString& Paths)
{
    FProjectIncludePaths = Paths;
}
//---------------------------------------------------------------------------

void TParser::ParseIncludes(VString& Queue, VString& Includes)
{
    // Clear the 'Includes' file list
    Includes.clear();

    // Create a random file name for temporary 'queue' file in 'temp' dir (returns an 8.3 path)
    String QueueFile = FProjectPath + L"__chbld\\chbld_" + Environment::CreateGuidString();

    // Create a random file name for temporary include output file in 'temp' dir (returns an 8.3 path)
    String OutFile = FProjectPath + L"__chbld\\chbld_" + Environment::CreateGuidString();

    __try
    {
        {
            // Create a StreamWriter...
            std::unique_ptr<TStreamWriter> FileQueue(new TStreamWriter(QueueFile, false));

            // ...and write the file names down to the queue file
            for (std::size_t i = 0; i < Queue.size(); ++i)
                FileQueue->WriteLine(Queue[i]);
        }

        // Build the Ctags command line
        String CmdInclude =
			FCtagsExe + 					// Ctags exe
			L" -o\"" + OutFile + L"\""		// Path of the output file
			L" --extras=r"					// Include reference tags
			L" --kinds-C++=h"
			L" --language-force=C++"		// Force C++
			L" -L\"" + QueueFile + L"\"";	// Path of the input queue file

        // Run the command
        Environment::ExecuteCmd(CmdInclude);

        {
            // Create a StreamReader
            std::unique_ptr<TStreamReader> IncludeList(new TStreamReader(OutFile));

            // Interpret each line of the ctags output file
            while (!IncludeList->EndOfStream)
            {
                TIncludeTag IncludeTag;

                String Line = IncludeList->ReadLine();

                // Extract data from the line
                if (InterpretIncludeData(Line, IncludeTag))
                {
                    // If file is a system include...
                    if (IncludeTag.Address.Pos(L"<") != 0)
                    {
                        // ...search for the file in the IDE include paths
                        foreach_ (String& IDEIncludePath, FIDEIncludePaths)
                        {
                            String FileName = IDEIncludePath + IncludeTag.Name.LowerCase();

                            FileName =
                                StringReplace(
                                    FileName,
                                    L"/",
                                    L"\\",
                                    TReplaceFlags() << rfReplaceAll
                                );

                            // If the file exists, add it to the include list
                            if (FileExists(FileName))
                                Includes.push_back(FileName);

                            // Check for related files with different extensions
                            foreach_ (String& RelatedFileExtension, FRelatedFileExtensions)
                            {
                                String RelatedFileName = ChangeFileExt(FileName, RelatedFileExtension);

                                if (FileExists(RelatedFileName) && (FileName != RelatedFileName))
                                    Includes.push_back(FileName);
                            }

                        }
                    }
                    // If file is a user include...
                    else if (IncludeTag.Address.Pos(L"\"") != 0)
                    {
                        // ...search for the file in the project include paths
                        foreach_ (String& ProjectIncludePath, FProjectIncludePaths)
                        {
                            String FileName = ProjectIncludePath + IncludeTag.Name.LowerCase();

                            FileName =
                                StringReplace(
                                    FileName,
                                    L"/",
                                    L"\\",
                                    TReplaceFlags() << rfReplaceAll
                                );

                            // If the file exists, add it to the include list
                            if (FileExists(FileName))
                                Includes.push_back(FileName);

                            // Check for related files with different extensions
                            foreach_ (String& RelatedFileExtension, FRelatedFileExtensions)
                            {
                                String RelatedFileName = ChangeFileExt(FileName, RelatedFileExtension);

                                if (FileExists(RelatedFileName) && (FileName != RelatedFileName))
                                    Includes.push_back(FileName);
                            }
                        }
                    }
                }
            }
        }
    }
    __finally
    {
        DeleteFile(QueueFile);
        DeleteFile(OutFile);
    }
}
//---------------------------------------------------------------------------

void TParser::FullParseIncludes(VString& Queue, VString& Includes)
{
    // Clear the 'Includes' file list
    Includes.clear();

    VString ParseResults;

    // Perform the first include parsing
    // (this parsing takes the current active configuration and IDE settings into account)
    ParseIncludes(Queue, ParseResults);

    // Add the resulting include files to a TStringList
    // in which duplicates are ignored, so the same file cannot
    // appear twice (the file names are normalized in 'ParseIncludes')
    std::unique_ptr<TStringList> ParseList(new TStringList);
    ParseList->Sorted     = true;
    ParseList->Duplicates = System::Types::dupIgnore;

    foreach_ (String& ParseResult, ParseResults)
        ParseList->Add(ParseResult);

    int ParseListCount = 0;

    // Reparse the include files for further includes as long as there are new files
    while (ParseList->Count > ParseListCount)
    {
        VString ParseVec;

        for (int i = 0; i < ParseList->Count; ++i)
            ParseVec.push_back(ParseList->Strings[i]);

        ParseIncludes(ParseVec, ParseResults);

        foreach_ (String& ParseResult, ParseResults)
            ParseList->Add(ParseResult);

        ParseListCount = ParseList->Count;
    }

    //=====================================================================
    // At this point 'ParseList' contains all the ABSOLUTE PATHS of
    // ALL EXISTENT INCLUDE FILES referenced by any of the 'Queue' files
    //=====================================================================

    // Put the 'ParseList' content into the 'Includes' vector
    for (int i = 0; i < ParseList->Count; ++i)
        Includes.push_back(ParseList->Strings[i]);
}
//---------------------------------------------------------------------------

void TParser::ParseTags(const VString& Queue, VTag& Results)
{
    // Clear the 'Results' vector
    Results.clear();

    // Create a random file name for temporary 'queue' file in 'temp' dir (returns a 8.3 path)
    String QueueFile = FProjectPath + L"__chbld\\chbld_" + Environment::CreateGuidString();

    // Create a random file name for temporary output file in 'temp' dir (returns a 8.3 path)
    String OutFile = FProjectPath + L"__chbld\\chbld_" + Environment::CreateGuidString();

    __try
    {
        {
            // Create a StreamWriter...
            std::unique_ptr<TStreamWriter> FileQueue(new TStreamWriter(QueueFile, false));

            // ...and write the file names down to the queue file
            for (std::size_t i = 0; i < Queue.size(); ++i)
                FileQueue->WriteLine(Queue[i]);
        }

        /*
        --fields=[+|-]flags
        Specifies the available extension fields which are to be included in the entries of the tag
        file (see TAG FILE FORMAT, below, for more information). The parameter flags is a set of
        one-letter flags, each representing one type of extension field to include, with the
        following meanings (disabled by default unless indicated):
            a   Access (or export) of class members
            f   File-restricted scoping [enabled]
            i   Inheritance information
            k   Kind of tag as a single letter [enabled]
            K   Kind of tag as full name
            l   Language of source file containing tag
            m   Implementation information
            n   Line number of tag definition
            s   Scope of tag definition [enabled]
            S   Signature of routine (e.g. prototype or parameter list)
            z   Include the "kind:" key in kind field
            t   Type and name of a variable or typedef as "typeref:" field [enabled]

        Each letter or group of letters may be preceded by either '+' to add it to the default set,
        or '-' to exclude it. In the absence of any preceding '+' or '-' sign, only those kinds
        explicitly listed in flags will be included in the output (i.e. overriding the default set).
        This option is ignored if the option --format=1 has been specified.
        The default value of this option is fkst.

        --c-kinds flags
        c  classes
        d  macro definitions
        e  enumerators (values inside an enumeration)
        f  function definitions
        g  enumeration names
        l  local variables [off]
        m  class, struct, and union members
        n  namespaces
        p  function prototypes [off]
        s  structure names
        t  typedefs
        u  union names
        v  variable definitions
        x  external and forward variable declarations [off]
        */

        // Build the Ctags command line
        String CmdToken =
            FCtagsExe +
            L" --excmd=pattern"									// Use only search patterns for all tags
            L" --sort=no"										// No sorting
            L" --fields=laKmSsnitz"
            L" --c-kinds=+plx"
            L" --C++-kinds=+p"
            L" -D \"__interface=class\""
            L" -D \"__published=public\""
            L" -D \"__try=try\""

			/* ===== Dirty hack begin ======================================= */
            L" --langdef=\"cppbuilder{base=C++}\""          	// Uh oh - A dirty hack here to
            L" --kinddef-cppbuilder=\"P,property,properties\""	// make Ctags output '__property'
            L" --regex-cppbuilder=\"/__property[ \t]{1,}.{1,}" 	// tags and their parent class of
                L"[ \t]{1,}([a-zA-Z_][a-zA-Z0-9_]*)"          	// the property, too. See
                L"[ \t]*=[ \t]*\{/\1/P/\""                     	// https://github.com/
            L" -D \"__property=__property struct\""				// 		universal-ctags/ctags/issues/1499
			/* ===== Dirty hack end ========================================= */

            L" -L\"" + QueueFile + L"\""
            L" -f\"" + OutFile + L"\"";

        // Run the command
        Environment::ExecuteCmd(CmdToken);

        if (FileExists(OutFile))
        {
            // Create a StreamReader
            std::unique_ptr<TStreamReader> TagList(new TStreamReader(OutFile));

            // Interpret each line of the ctags output file
            while (!TagList->EndOfStream)
            {
                String Line = TagList->ReadLine();

                // Skip lines beginning with '!_TAG_'
                if (Line.SubString(1, 6) != L"!_TAG_")
                {
                    TTag Tag;

                    // Interpret the line data
                    if (InterpretLineData(Line, Tag))
                    {
                         // Add the tag
                        Results.push_back(Tag);
                    }
                }
            }

        }
    }
    __finally
    {
        DeleteFile(QueueFile);
        DeleteFile(OutFile);
    }
}
//---------------------------------------------------------------------------

bool TParser::InterpretIncludeData(const String& LineText, TIncludeTag& IncludeTag)
{
    VString Includes = Environment::SplitStr(LineText, L'\t');

    if (Includes.size() < 4)
        return false;

    IncludeTag.Name         = Includes[0];
    IncludeTag.File         = Includes[1];
    IncludeTag.Address      = Includes[2];

    IncludeTag.Kind         = Includes[3];

    return true;
}
//---------------------------------------------------------------------------

bool TParser::InterpretLineData(const String& LineText, TTag& Tag)
{
    // Extract the 'Address' part via regex
    String Address = TRegEx::Match(LineText, L"(?<=\\/\\^)(.*)(?=\\;\\\")").Value.Trim();

    // Replace the 'Address' part in the line text with an empty string so the line contains
    // '/^$/;"' instead (no more tab chars)
    String ReplacedText = StringReplace(LineText, Address, L"", TReplaceFlags() << rfReplaceAll);

	//=======================================================================
	// Rearrange the address part
	//=======================================================================
    // Delete the remainings of the address regex
    if (RightStr(Address, 2) == L"$/")
        Address = LeftStr(Address, Address.Length() - 2);
    if (RightStr(Address, 1) == L"/")
        Address = LeftStr(Address, Address.Length() - 1);

    // Remove the escaping of slashes
    Address = StringReplace(Address, L"\\/", L"/", TReplaceFlags() << rfReplaceAll);

    // Change tabs to spaces
    Address = StringReplace(Address, L"\t", L" ", TReplaceFlags() << rfReplaceAll);

    // Replace two spaces with one until there is no more than one consecutive space in each place
    int AddressLength = MAXINT;

    while (Address.Length() < AddressLength)
    {
        AddressLength   = Address.Length();
        Address         = StringReplace(Address, L"  ", L" ", TReplaceFlags() << rfReplaceAll);
    }

    // Get the data types from '__property' tags (they are not extracted by Ctags, even with
    // those 'dirty hacking' methods in 'ParseTags')
    bool    IsProperty          = false;
    String  PropertyDataType    = L"";

    // If there is the phrase '__property' in the address
    if (Address.Pos(L"__property"))
	{
        // Seperate each token in address
        VString Tokens = Environment::SplitStr(Address, L' ');

        // Iterate over each address token
		for (std::size_t i = 0; i < Tokens.size(); ++i)
		{
            // Find the '__property' token
			if ((Tokens[i] == L"__property") && (i < (Tokens.size() - 2)))
			{
                // Set the 'IsProperty' flag
				IsProperty          = true;

                // Get the property data type
				PropertyDataType    = Tokens[i+1];

				break;
			}
		}
	}
	//=======================================================================


	//=======================================================================
	// Remove tokens unnecessary for code completion from replaced text
	//=======================================================================
	// Remove all calling conventions
    ReplacedText = StringReplace(ReplacedText, L"__cdecl", L"", TReplaceFlags() << rfReplaceAll);
    ReplacedText = StringReplace(ReplacedText, L"__clrcall", L"", TReplaceFlags() << rfReplaceAll);
    ReplacedText = StringReplace(ReplacedText, L"__stdcall", L"", TReplaceFlags() << rfReplaceAll);
    ReplacedText = StringReplace(ReplacedText, L"__fastcall", L"", TReplaceFlags() << rfReplaceAll);
    ReplacedText = StringReplace(ReplacedText, L"__thiscall", L"", TReplaceFlags() << rfReplaceAll);
    ReplacedText = StringReplace(ReplacedText, L"__vectorcall", L"", TReplaceFlags() << rfReplaceAll);

    // Remove Object Pascal specific keywords
    if (!Address.Pos(L"#define DELPHI_PACKAGE"))
        ReplacedText = StringReplace(ReplacedText, L"DELPHI_PACKAGE", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define PACKAGE"))
        ReplacedText = StringReplace(ReplacedText, L"PACKAGE", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define DELPHICLASS"))
        ReplacedText = StringReplace(ReplacedText, L"DELPHICLASS", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define PASCALIMPLEMENTATION"))
        ReplacedText = StringReplace(ReplacedText, L"PASCALIMPLEMENTATION", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define HIDESBASE"))
        ReplacedText = StringReplace(ReplacedText, L"HIDESBASE", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define HIDESBASEDYNAMIC"))
        ReplacedText = StringReplace(ReplacedText, L"HIDESBASEDYNAMIC", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define DYNAMIC"))
        ReplacedText = StringReplace(ReplacedText, L"DYNAMIC", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define MESSAGE"))
        ReplacedText = StringReplace(ReplacedText, L"MESSAGE", L"", TReplaceFlags() << rfReplaceAll);

    if (!Address.Pos(L"#define _DELPHICLASS_TOBJECT"))
        ReplacedText = StringReplace(ReplacedText, L"_DELPHICLASS_TOBJECT", L"", TReplaceFlags() << rfReplaceAll);

    // Remove 'classmethod' keyword
    ReplacedText = StringReplace(ReplacedText, L"__classmethod", L"", TReplaceFlags() << rfReplaceAll);

    // Remove 'closure' keyword
    ReplacedText = StringReplace(ReplacedText, L"__closure", L"", TReplaceFlags() << rfReplaceAll);
	//=======================================================================

    // Seperate the tag line tokens
    VString Tags = Environment::SplitStr(ReplacedText, L'\t');

    if (Tags.size() < 4)
        return false;

    if (Tags[0].Trim().IsEmpty())
        return false;

    Tag.Name            = Tags[0];
    Tag.File            = Tags[1];
    Tag.Address         = Address;

    Tag.Kind            = L"";
    Tag.LineNo          = -1;
    Tag.Namespace       = L"";
    Tag.Class           = L"";
    Tag.Struct          = L"";
    Tag.Access          = L"";
    Tag.Implementation  = L"";
    Tag.Signature       = L"";
    Tag.Typeref_A       = L"";
    Tag.Typeref_B       = L"";
    Tag.Inherits        = L"";

    std::unique_ptr<TStringList> Values(new TStringList);

    Values->Delimiter       = L':';
    Values->StrictDelimiter = true;

    for (std::size_t i = 3; i < Tags.size(); ++i)
    {
        // Replace consecutive colons with a unicode replacement char and assign it as delimited
        // text
        Values->DelimitedText =
            StringReplace(
                Tags[i],
                L"::",
                L"\xFFFF",
                TReplaceFlags() << rfReplaceAll
                );

        // Undo the replacements after the assignment
        Values->Text =
            StringReplace(
                Values->Text,
                L"\xFFFF",
                L"::",
                TReplaceFlags() << rfReplaceAll
                );

        if (Values->Count > 1)
        {
            String FieldName    = Values->Strings[0].Trim();
            String Value        = Values->Strings[1].Trim();
            String Value_2      = L"";

            if (Values->Count > 2)
                Value_2 = Values->Strings[2].Trim();

            if (FieldName == L"kind")
            {
                // If the current tag is of type '__property'
                // we have to replace the entry 'member' with 'property'
                // and assign the data type to 'Typeref_B' here,
                // because properties have no filled entry for 'typeref'
                if (IsProperty)
                {
                    Tag.Kind        = L"property";
                    Tag.Typeref_B   = PropertyDataType;
                }
                else
                {
                    Tag.Kind = Value;
                }
            }
            else if (FieldName  == L"line")
                Tag.LineNo          = Value.ToInt();
            else if (FieldName  == L"namespace")
                Tag.Namespace       = Value;
            else if (FieldName  == L"class")
                Tag.Class           = Value;
            else if (FieldName  == L"struct")
                Tag.Struct          = Value;
            else if (FieldName  == L"access")
                Tag.Access          = Value;
            else if (FieldName  == L"implementation")
                Tag.Implementation  = Value;
            else if (FieldName  == L"signature")
                Tag.Signature       = Value;
            else if (FieldName  == L"typeref")
            {
                Tag.Typeref_A       = Value;
                Tag.Typeref_B       = Value_2;
            }
            else if (FieldName  == L"inherits")
                Tag.Inherits        = Value;
        }
    }

    // We must build the full-qualified name for the respective field
    if (!Tag.Class.IsEmpty())
        Tag.QualifiedName = Tag.Class + L"::" + Tag.Name;
    else if (!Tag.Struct.IsEmpty())
        Tag.QualifiedName = Tag.Struct + L"::" + Tag.Name;
    else if (!Tag.Namespace.IsEmpty())
        Tag.QualifiedName = Tag.Namespace + L"::" + Tag.Name;
    else
        Tag.QualifiedName = Tag.Name;

    // We have to change the tokens
    //  'prototype' to 'function'
    //  'function' to 'implementation' and
    //  'member' to 'variable'
    // to make the meaning more clear in completion list
    if (Tag.Kind == L"prototype")
        Tag.Kind = L"function";
    else if (Tag.Kind == L"function")
        Tag.Kind = L"implementation";
    else if (Tag.Kind == L"member")
        Tag.Kind = L"variable";

    // Detect, if we have a constructor or destructor
    if ((Tag.Kind == L"function") && Tag.Typeref_B.IsEmpty() && !Tag.Address.Pos(L"~"))
        Tag.Kind = L"constructor";
    else if ((Tag.Kind == L"function") && Tag.Typeref_B.IsEmpty())
        Tag.Kind = L"destructor";

    // Double check the correctness of detecting all properties as 'property'
    if (Tag.Address.Trim().SubString(1, 11) == L"__property ")
    {
        if (Tag.Kind != L"property")
            Tag.Kind = L"property";
    }

    // Properties can be defined in derived classes without specifying their type again.
    // As Ctags could not read further informations about this it falsely reads
    // the property's name as the value Typeref_B, so we set this here to an empty string in
    // this case

    // If the tag is a property...
    if (Tag.Kind == L"property")
    {
        // ...check if 'Typeref_B' contains the same text as the name...
        if (Tag.Name == Tag.Typeref_B)
        {
            // ...and clear it if it is the case
            Tag.Typeref_B = L"";
        }
    }

    // If we have a forward declaration, we don't need to include that
    if ((Tag.Typeref_A == L"class") || (Tag.Typeref_A == L"struct"))
        return false;

    // If we have a virtual declaration we don't need to include that
    //if (Tag.Address.Trim().SubString(1, 8) == L"virtual ")
    //    return false;

    return true;
}
//---------------------------------------------------------------------------

} // namespace Ctags

} // namespace Cherrybuilder

