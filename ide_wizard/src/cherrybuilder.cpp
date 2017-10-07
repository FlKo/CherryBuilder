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

//==========================================================================
// It's necessary to add "designide.bpi" to the additional linker options
//==========================================================================
// The registration into the IDE has to be done in registry:
// HKEY_CURRENT_USER/SOFTWARE/Embarcadero/BDS/XX.X/Experts/
// REG_SZ CherryBuilder with value 'PathToDLL'
//==========================================================================

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------

#include <ToolsAPI.hpp>

#include "cherrybuilder_wizard.h"

#include "cherrybuilder_debugtools.h"
//---------------------------------------------------------------------------

#pragma link "DesignIDE.bpi"

namespace Cherrybuilder
{

_di_IBorlandIDEServices LocalIDEServices;
//---------------------------------------------------------------------------

// Wizard DLL entry point
extern "C" bool __stdcall __declspec(dllexport) INITWIZARD0001(
    const _di_IBorlandIDEServices Service,
    TWizardRegisterProc RegisterWizard,
    TWizardTerminateProc&
    )
{
    // Get a reference to the BorlandIDEServices
    LocalIDEServices = Service;

    // Do the registration of the wizard
    RegisterWizard(new TChBldWiz(Service));

    return true;
}
//---------------------------------------------------------------------------

} // namespace Cherrybuilder

