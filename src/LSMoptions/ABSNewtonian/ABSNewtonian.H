/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 2011-2016 OpenFOAM Foundation
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
    Foam::viscosityModels::ABSNewtonian

Description
    An incompressible Newtonian viscosity model with modified solid phase viscosity.

SourceFiles
    ABSNewtonian.C

\*---------------------------------------------------------------------------*/

#ifndef ABSNewtonian_H
#define ABSNewtonian_H

#include "viscosityModel.H"
#include "dimensionedScalar.H"
#include "volFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
namespace viscosityModels
{

/*---------------------------------------------------------------------------*\
                           Class Newtonian Declaration
\*---------------------------------------------------------------------------*/

class ABSNewtonian
:
    public viscosityModel
{
    // Private data

        dimensionedScalar nu0_;
        dimensionedScalar nuSolid0_;

        volScalarField nu_;

	//const fvMesh& mesh_;

//	tmp<volScalarField> calcNu(const volScalarField&) const;
	tmp<volScalarField> calcNu() const;


public:

    //- Runtime type information
    TypeName("ABSNewtonian");


    // Constructors

        //- Construct from components
        ABSNewtonian
        (
            const word& name,
            const dictionary& viscosityProperties,
            const volVectorField& U,
            const surfaceScalarField& phi
        );


    //- Destructor
    virtual ~ABSNewtonian()
    {}


    // Member Functions

        //- Return the laminar viscosity
        virtual tmp<volScalarField> nu() const
        {
            return nu_;
        }



        //- Return the laminar viscosity for patch
        virtual tmp<scalarField> nu(const label patchi) const
        {
            return nu_.boundaryField()[patchi];
        }

        //- Correct the laminar viscosity (not appropriate, viscosity constant)
        /*virtual void correct()
        {
	Info << "Calling correct()" << endl;
	Info << " return is " << calcNu() << endl;
	Foam::tmp<Foam::volScalarField> nuTemp = calcNu();
	//nuTemp.dimensions().reset(nu_.dimensions());
	//nu_ = nuTemp;
	nu_ = nuTemp();
	}*/

 virtual void correct()
 {
     Info << "Adjusting viscosity at solid cells." << endl;
     volScalarField nuTemp = nu_;
     dimensionSet nuDims(nu_.dimensions());
     const volScalarField& voidFrac = (U_.mesh()).lookupObject<volScalarField>("voidFrac");

     nuTemp = nu0_ + nuSolid0_ * voidFrac;//nu_ -> nu0_

     nu_ = nuTemp;
 }        

        //- Read transportProperties dictionary
        virtual bool read(const dictionary& viscosityProperties);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace viscosityModels
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
