/*---------------------------------------------------------------------------*\
      _________________________________________________________
     /                                                        /|
    /                                                        / |
   |--------------------------------------------------------|  |
   |        _    ____ ____  _____                           |  |
   |       / \  | __ ) ___||  ___|__   __ _ _ __ ___        |  |
   |      / _ \ |  _ \___ \| |_ / _ \ / _` | '_ ` _ \       |  |
   |     / ___ \| |_) |__) |  _| (_) | (_| | | | | | |      |  |
   |    /_/   \_\____/____/|_|  \___/ \__,_|_| |_| |_|      |  |
   |                                                        |  |
   |    Arbitrary  Body  Simulation    for    OpenFOAM      | /
   |________________________________________________________|/


\*---------------------------------------------------------------------------*/

#ifndef fixedSlipLengthFvPatchFields_H
#define fixedSlipLengthFvPatchFields_H

#include "fixedSlipLengthFvPatchField.H"
#include "fieldTypes.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//makePatchTypeFieldTypedefs(fixedSlipLength);
//typedef fixedSlipLengthFvPatchField<vector> fixedSlipLengthFvPatchVectorField; // Vora: OF-3.0
//Vora: OF-5.x
typedef fixedSlipLengthFvPatchField<vector> \
        CAT4(fixedSlipLength, FvPatch, CAPITALIZE(vector), Field);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
