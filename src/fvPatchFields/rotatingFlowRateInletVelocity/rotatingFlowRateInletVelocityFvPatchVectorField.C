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

\*---------------------------------------------------------------------------*/

#include "rotatingFlowRateInletVelocityFvPatchVectorField.H"
#include "volFields.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "surfaceFields.H"

#include "makros.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::rotatingFlowRateInletVelocityFvPatchVectorField::
rotatingFlowRateInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(p, iF),
    flowRate_(),
    volumetric_(false),
    rhoName_("rho"),
    rhoInlet_(0.0),
    origin_(),
    axis_(vector::zero),
    omega_(0)
{}


Foam::rotatingFlowRateInletVelocityFvPatchVectorField::
rotatingFlowRateInletVelocityFvPatchVectorField
(
    const rotatingFlowRateInletVelocityFvPatchVectorField& ptf,
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<vector>(ptf, p, iF, mapper),
    flowRate_(ptf.flowRate_().clone().ptr()),
    volumetric_(ptf.volumetric_),
    rhoName_(ptf.rhoName_),
    rhoInlet_(ptf.rhoInlet_),
    origin_(ptf.origin_),
    axis_(ptf.axis_),
    omega_(ptf.omega_().clone().ptr())
{}


Foam::rotatingFlowRateInletVelocityFvPatchVectorField::
rotatingFlowRateInletVelocityFvPatchVectorField
(
    const fvPatch& p,
    const DimensionedField<vector, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<vector>(p, iF),
    rhoInlet_(dict.lookupOrDefault<scalar>("rhoInlet", -VGREAT)),
    origin_(dict.lookup("origin")),
    axis_(dict.lookup("axis")),
//    omega_(DataEntry<scalar>::New("omega", dict)) //Vora: OF-3
    omega_(Function1<scalar>::New("omega", dict)) //Vora: OF-5.x
{
    if (dict.found("volumetricFlowRate"))
    {
        volumetric_ = true;
//        flowRate_ = DataEntry<scalar>::New("volumetricFlowRate", dict); //Vora: OF-3
        flowRate_ = Function1<scalar>::New("volumetricFlowRate", dict); //Vora: OF-5.x
        rhoName_ = "rho";
    }
    else if (dict.found("massFlowRate"))
    {
        volumetric_ = false;
//        flowRate_ = DataEntry<scalar>::New("massFlowRate", dict); //Vora: OF-3
        flowRate_ = Function1<scalar>::New("massFlowRate", dict); //Vora: OF-5.x
        rhoName_ = word(dict.lookupOrDefault<word>("rho", "rho"));
    }
    else
    {
        FatalIOErrorIn
        (
            "rotatingFlowRateInletVelocityFvPatchVectorField::"
            "rotatingFlowRateInletVelocityFvPatchVectorField"
            "(const fvPatch&, const DimensionedField<vector, volMesh>&,"
            " const dictionary&)",
            dict
        )   << "Please supply either 'volumetricFlowRate' or"
            << " 'massFlowRate' and 'rho'" << exit(FatalIOError);
    }

    // Value field require if mass based
    if (dict.found("value"))
    {
        fvPatchField<vector>::operator=
        (
            vectorField("value", dict, p.size())
        );
    }
    else
    {
//        evaluate(Pstream::blocking);//Vora:OF-3
        evaluate(Pstream::commsTypes::blocking);//Vora: OF-5.x 'blocking' is not a member of 'Foam::Pstream' 
    }
}


Foam::rotatingFlowRateInletVelocityFvPatchVectorField::
rotatingFlowRateInletVelocityFvPatchVectorField
(
    const rotatingFlowRateInletVelocityFvPatchVectorField& ptf
)
:
    fixedValueFvPatchField<vector>(ptf),
    flowRate_(ptf.flowRate_().clone().ptr()),
    volumetric_(ptf.volumetric_),
    rhoName_(ptf.rhoName_),
    rhoInlet_(ptf.rhoInlet_),
    origin_(ptf.origin_),
    axis_(ptf.axis_),
    omega_(ptf.omega_().clone().ptr())
{}


Foam::rotatingFlowRateInletVelocityFvPatchVectorField::
rotatingFlowRateInletVelocityFvPatchVectorField
(
    const rotatingFlowRateInletVelocityFvPatchVectorField& ptf,
    const DimensionedField<vector, volMesh>& iF
)
:
    fixedValueFvPatchField<vector>(ptf, iF),
    flowRate_(ptf.flowRate_().clone().ptr()),
    volumetric_(ptf.volumetric_),
    rhoName_(ptf.rhoName_),
    rhoInlet_(ptf.rhoInlet_),
    origin_(ptf.origin_),
    axis_(ptf.axis_),
    omega_(ptf.omega_().clone().ptr())
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::rotatingFlowRateInletVelocityFvPatchVectorField::updateCoeffs()
{
    if (updated())
    {
        return;
    }

    const scalar t = db().time().timeOutputValue();
    scalar om = omega_->value(t);

    // Calculate the rotating wall velocity from the specification of the motion
    vectorField Up
    (
        (-om)*((patch().Cf() - origin_) ^ (axis_/mag(axis_)))
    );

    // Remove the component of Up normal to the wall
    // just in case it is not exactly circular
    const vectorField n(patch().nf());
    Up -= n*(n & Up);


    // a simpler way of doing this would be nice
    const scalar avgU = -flowRate_->value(t)/gSum(patch().magSf());


    if (volumetric_ || rhoName_ == "none")
    {
        // volumetric flow-rate or density not given
        operator==(n*avgU   +   Up);
    }
    else
    {
        // mass flow-rate
        if (db().foundObject<volScalarField>(rhoName_))
        {
            const fvPatchField<scalar>& rhop =
                patch().lookupPatchField<volScalarField, scalar>(rhoName_);

            operator==(n*avgU/rhop   +   Up);
        }
        else
        {
            // Use constant density
            if (rhoInlet_ < 0)
            {
                FatalErrorIn
                (
                    "rotatingFlowRateInletVelocityFvPatchVectorField::updateCoeffs()"
                )   << "Did not find registered density field " << rhoName_
                    << " and no constant density 'rhoInlet' specified"
                    << exit(FatalError);
            }
            operator==(n*avgU/rhoInlet_   +   Up);
        }
    }

    fixedValueFvPatchVectorField::updateCoeffs();
}


void Foam::rotatingFlowRateInletVelocityFvPatchVectorField::write(Ostream& os) const
{
    fvPatchField<vector>::write(os);
    flowRate_->writeData(os);
    if (!volumetric_)
    {
        writeEntryIfDifferent<word>(os, "rho", "rho", rhoName_);
        writeEntryIfDifferent<scalar>(os, "rhoInlet", -VGREAT, rhoInlet_);
    }
    os.writeKeyword("origin") << origin_ << token::END_STATEMENT << nl;
    os.writeKeyword("axis") << axis_ << token::END_STATEMENT << nl;
    omega_->writeData(os);
    writeEntry(os, "value", *this);//writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
   makePatchTypeField
   (
       fvPatchVectorField,
       rotatingFlowRateInletVelocityFvPatchVectorField
   );
}


// ************************************************************************* //
