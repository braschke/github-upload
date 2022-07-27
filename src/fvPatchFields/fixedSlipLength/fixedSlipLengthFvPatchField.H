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

#ifndef fixedSlipLengthFvPatchField_H
#define fixedSlipLengthFvPatchField_H

#include "Random.H"
#include "fixedValueFvPatchFields.H"
//#include "DataEntry.H" //Vora: OF-3
#include "Function1.H" //Vora: OF-5.x

#include "makros.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class fixedSlipLengthFvPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class fixedSlipLengthFvPatchField
:
    public fixedValueFvPatchField<Type>
{
    // Private data
    scalar   slipLength_;

public:

    //- Runtime type information
    TypeName("fixedSlipLength");


    // Constructors

        //- Construct from patch and internal field
    fixedSlipLengthFvPatchField
        (
            const fvPatch&,
            const DimensionedField<Type, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
    fixedSlipLengthFvPatchField
        (
            const fvPatch&,
            const DimensionedField<Type, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given fixedSlipLengthFvPatchField
        //  onto a new patch
    fixedSlipLengthFvPatchField
        (
            const fixedSlipLengthFvPatchField<Type>&,
            const fvPatch&,
            const DimensionedField<Type, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        fixedSlipLengthFvPatchField
        (
            const fixedSlipLengthFvPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchField<Type> > clone() const
        {
            return tmp<fvPatchField<Type> >
            (
                new fixedSlipLengthFvPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        fixedSlipLengthFvPatchField
        (
            const fixedSlipLengthFvPatchField<Type>&,
            const DimensionedField<Type, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchField<Type> > clone
        (
            const DimensionedField<Type, volMesh>& iF
        ) const
        {
            return tmp<fvPatchField<Type> >
            (
                new fixedSlipLengthFvPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Access

        // Mapping functions


        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "fixedSlipLengthFvPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
