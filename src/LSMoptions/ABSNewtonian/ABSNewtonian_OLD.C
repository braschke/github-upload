/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2015 OpenFOAM Foundation
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

#include "ABSNewtonian.H"
#include "addToRunTimeSelectionTable.H"
#include "surfaceFields.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace viscosityModels
{
    defineTypeNameAndDebug(ABSNewtonian, 0);
    addToRunTimeSelectionTable(viscosityModel, ABSNewtonian, dictionary);
}
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::viscosityModels::ABSNewtonian::ABSNewtonian
(
    const word& name,
    const dictionary& viscosityProperties,
    const volVectorField& U,
    const surfaceScalarField& phi
)
:
    viscosityModel(name, viscosityProperties, U, phi),
    nu0_("nu", dimViscosity, viscosityProperties_),
    nuSolid0_("nuSolid", dimViscosity, viscosityProperties_),
    nu_
    (
        IOobject
        (
            name,
            U_.time().timeName(),
            U_.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        U_.mesh(),
        nu0_
    )
{
//   const volScalarField& voidFrac = (U_.mesh()).lookupObject<volScalarField>("voidFrac");
//   const autoPtr<volScalarField> voidFrac = (U_.mesh()).lookupObject<volScalarField>("voidFrac");


if(! (U_.mesh()).foundObject<volScalarField>("voidFrac") )
  {
    // create field for void fraction in main mesh
    new volScalarField
               (
                 IOobject
                 (
                     "voidFrac",
                     U_.time().timeName(),
                     U_.mesh(),
                     IOobject::READ_IF_PRESENT,
                     IOobject::AUTO_WRITE
                 ),
                 U_.mesh(),
                 dimensioned<scalar>("fluid", dimensionSet(0, 0, 0, 0, 0), 0.0)
               );
  }

  volScalarField& voidFrac = const_cast<volScalarField&>(
                            (U_.mesh()).lookupObject<volScalarField>("voidFrac")
                                                  );


   if(voidFrac.size() > 0)
	{
		Info << "Calculating nu !!!!! " << endl;
//		Info << "HERE IT GOES NOWWWWWWWW " << calcNu(voidFrac) << endl;
		volScalarField nuTemp = calcNu(voidFrac);
		nuTemp.dimensions().reset(nu_.dimensions());
   		//nu_ = calcNu(voidFrac);
		nu_ = nuTemp;
	}
   else
	{
		Info << "SO SAAAAAAAD :C " << endl;
		this->nu_ = viscosityProperties_.lookup("nu") >> nu0_;
	}
}


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

bool Foam::viscosityModels::ABSNewtonian::read
(
    const dictionary& viscosityProperties
)
{
    viscosityModel::read(viscosityProperties);

    viscosityProperties_.lookup("nu") >> nu0_;
    viscosityProperties_.lookup("nuSolid") >> nuSolid0_;
    nu_ = nu0_;

    return true;
}

 Foam::tmp<Foam::volScalarField>
 Foam::viscosityModels::ABSNewtonian::calcNu
 (
     const volScalarField& field
 ) const
 {
     Info << " I am in nu() with nuSolid0_ = " << nuSolid0_ << endl;
     volScalarField nuTemp = nu_;
     dimensionSet nuDims(nu_.dimensions());
     Info << " nuDims = " << nuDims << endl;
     const volScalarField& voidFrac = (U_.mesh()).lookupObject<volScalarField>("voidFrac");

     nuTemp = nu_ + nuSolid0_ * voidFrac;

     Info << " NO PROB BEBE " << nuTemp << endl;
     Info << " tempsize " << nuTemp.size() << endl;
     Info << " nuSize " << nu_.size() << endl;
     Info << " voidFracSize " << voidFrac.size() << endl;

     nuTemp.dimensions().reset(nuDims);

     Info << " nuTemp.dimensions() = " << nuTemp.dimensions() << endl;

     return nuTemp;
 }















// ************************************************************************* //
