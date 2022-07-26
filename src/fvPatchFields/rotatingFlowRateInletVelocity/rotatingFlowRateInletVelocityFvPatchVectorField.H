/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2014 OpenFOAM Foundation
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::rotatingFlowRateVelocityInletFvPatchVectorField

Group
    grpInletBoundaryConditions

Description
    This boundary condition provides a velocity boundary condition, derived
    from the flux (volumetric or mass-based), whose direction is assumed
    to be normal to the patch plus a tangential component due to a rotating
    frame.

    For a mass-based flux:
    - the flow rate should be provided in kg/s
    - if \c rhoName is "none" the flow rate is in m3/s
    - otherwise \c rhoName should correspond to the name of the density field
    - if the density field cannot be found in the database, the user must
      specify the inlet density using the \c rhoInlet entry

    For a volumetric-based flux:
    - the flow rate is in m3/s

    \heading Patch usage

    \table
        Property     | Description             | Required    | Default value
        massFlowRate | mass flow rate [kg/s]   | no          |
        volumetricFlowRate | volumetric flow rate [m3/s]| no |
        rhoInlet     | inlet density           | no          |
        origin       | origin of rotation in Cartesian co-ordinates | yes|
        axis         | axis of rotation        | yes         |
        omega        | angular velocty of the frame [rad/s] | yes    |
    \endtable

    Example of the boundary condition specification for a volumetric flow rate:
    \verbatim
    myPatch
    {
        type        rotatingFlowRateInletVelocity;
        volumetricFlowRate  0.2;
        value       uniform (0 0 0); // placeholder

        origin          (0 0 0);
        axis            (0 0 1);
        omega           100;
    }
    \endverbatim

    Example of the boundary condition specification for a mass flow rate:
    \verbatim
    myPatch
    {
        type                rotatingFlowRateInletVelocity;
        massFlowRate        0.2;
        rho                 rho;
        rhoInlet            1.0;

        origin          (0 0 0);
        axis            (0 0 1);
        omega           100;
    }
    \endverbatim

    The \c flowRate entry is a \c DataEntry type, meaning that it can be
    specified as constant, a polynomial fuction of time, and ...

Note
    - \c rhoInlet is required for the case of a mass flow rate, where the
      density field is not available at start-up
    - the value is positive into the domain (as an inlet)
    - may not work correctly for transonic inlets
    - strange behaviour with potentialFoam since the U equation is not solved

SeeAlso
    Foam::DataEntry
    Foam::fixedValueFvPatchField

SourceFiles
    rotatingFlowRateInletVelocityFvPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef rotatingFlowRateInletVelocityFvPatchVectorField_H
#define rotatingFlowRateInletVelocityFvPatchVectorField_H

#include "fixedValueFvPatchFields.H"
//#include "DataEntry.H" //Vora: OF-3
#include "Function1.H" //Vora: OF-5.x

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
/*---------------------------------------------------------------------------*\
           Class rotatingFlowRateInletVelocityFvPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class rotatingFlowRateInletVelocityFvPatchVectorField
:
    public fixedValueFvPatchVectorField
{
    // Private data

        //- Inlet integral flow rate
//        autoPtr<DataEntry<scalar> > flowRate_; //Vora: OF-3
        autoPtr<Function1<scalar> > flowRate_; //Vora: OF-5.x DataEntry changed to FUnction1

        //- Is volumetric?
        bool volumetric_;

        //- Name of the density field used to normalize the mass flux
        word rhoName_;

        //- Rho initialisation value (for start; if value not supplied)
        scalar rhoInlet_;



        //- Origin of the rotation
        vector origin_;

        //- Axis of the rotation
        vector axis_;

        //- Rotational speed
//        autoPtr<DataEntry<scalar> > omega_; //Vora: OF-3
        autoPtr<Function1<scalar> > omega_; //Vora: OF-5.x DataEntry changed to Function1



public:

   //- Runtime type information
   TypeName("rotatingFlowRateInletVelocity");


   // Constructors

        //- Construct from patch and internal field
        rotatingFlowRateInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct from patch, internal field and dictionary
        rotatingFlowRateInletVelocityFvPatchVectorField
        (
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const dictionary&
        );

        //- Construct by mapping given
        //  rotatingFlowRateInletVelocityFvPatchVectorField
        //  onto a new patch
        rotatingFlowRateInletVelocityFvPatchVectorField
        (
            const rotatingFlowRateInletVelocityFvPatchVectorField&,
            const fvPatch&,
            const DimensionedField<vector, volMesh>&,
            const fvPatchFieldMapper&
        );

        //- Construct as copy
        rotatingFlowRateInletVelocityFvPatchVectorField
        (
            const rotatingFlowRateInletVelocityFvPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<fvPatchVectorField> clone() const
        {
            return tmp<fvPatchVectorField>
            (
                new rotatingFlowRateInletVelocityFvPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        rotatingFlowRateInletVelocityFvPatchVectorField
        (
            const rotatingFlowRateInletVelocityFvPatchVectorField&,
            const DimensionedField<vector, volMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<fvPatchVectorField> clone
        (
            const DimensionedField<vector, volMesh>& iF
        ) const
        {
            return tmp<fvPatchVectorField>
            (
                new rotatingFlowRateInletVelocityFvPatchVectorField(*this, iF)
            );
        }


    // Member functions

        // Access functions

            //- Return the origin of the rotation
            const vector& origin() const
            {
                return origin_;
            }

            //- Return the axis of the rotation
            const vector& axis() const
            {
                return axis_;
            }

            //- Return non-const access to the origin of the rotation
            vector& origin()
            {
                return origin_;
            }

            //- Return non-const access to the axis of the rotation
            vector& axis()
            {
                return axis_;
            }


        //- Update the coefficients associated with the patch field
        virtual void updateCoeffs();

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
