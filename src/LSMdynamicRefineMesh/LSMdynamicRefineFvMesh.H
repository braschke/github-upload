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
    Foam::LSMdynamicRefineFvMesh

Description
    A fvMesh with built-in refinement.

    Determines which cells to refine/unrefine and does all in update().


        // How often to refine
        refineInterval  1;
        // Field to be refinement on
        field           alpha.water;
        // Refine field inbetween lower..upper
        lowerRefineLevel 0.001;
        upperRefineLevel 0.999;
        // If value < unrefineLevel (default=GREAT) unrefine
        //unrefineLevel   10;
        // Have slower than 2:1 refinement
        nBufferLayers   1;
        // Refine cells only up to maxRefinement levels
        maxRefinement   2;
        // Stop refinement if maxCells reached
        maxCells        200000;
        // Flux field and corresponding velocity field. Fluxes on changed
        // faces get recalculated by interpolating the velocity. Use 'none'
        // on surfaceScalarFields that do not need to be reinterpolated, use
        // NaN to detect use of mapped variable
        correctFluxes
        (
            (phi none)  //NaN)   //none)
            (nHatf none)   //none)
            (rho*phi none)   //none)
            (ghf none)  //NaN)   //none)
        );
        // Write the refinement level as a volScalarField
        dumpLevel       true;


SourceFiles
    LSMdynamicRefineFvMesh.C

\*---------------------------------------------------------------------------*/

#ifndef LSMdynamicRefineFvMesh_H
#define LSMdynamicRefineFvMesh_H

#include "dynamicFvMesh.H"
#include "hexRef8.H"
#include "PackedBoolList.H"
#include "Switch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Class LSMdynamicRefineFvMesh Declaration
\*---------------------------------------------------------------------------*/

class LSMdynamicRefineFvMesh
:
    public dynamicFvMesh
{
protected:

        //- Mesh cutting engine
        hexRef8 meshCutter_;

        //- Dump cellLevel for postprocessing
        Switch dumpLevel_;

        //- Fluxes to map
        HashTable<word> correctFluxes_;

        //- Number of refinement/unrefinement steps done so far.
        label nRefinementIterations_;

        //- Protected cells (usually since not hexes)
        PackedBoolList protectedCell_;

        //- max time step until autorefinement of region 'alwaysRefine' will happen
        label maxTimeAutoRefine_;

    // Private Member Functions

        //- Count set/unset elements in packedlist.
        static label count(const PackedBoolList&, const unsigned int);

        //- Calculate cells that cannot be refined since would trigger
        //  refinement of protectedCell_ (since 2:1 refinement cascade)
        void calculateProtectedCells(PackedBoolList& unrefineableCell) const;

        //- Read the projection parameters from dictionary
        void readDict();


        //- Refine cells. Update mesh and fields.
        autoPtr<mapPolyMesh> refine(const labelList&);

        //- Unrefine cells. Gets passed in centre points of cells to combine.
        autoPtr<mapPolyMesh> unrefine(const labelList&);


        // Selection of cells to un/refine

            //- Calculates approximate value for refinement level so
            //  we don't go above maxCell
            scalar getRefineLevel
            (
                const label maxCells,
                const label maxRefinement,
                const scalar refineLevel,
                const scalarField&
            ) const;

            //- Get per cell max of connected point
            scalarField maxPointField(const scalarField&) const;

            //- Get point max of connected cell
            scalarField maxCellField(const List<scalarField*> &vFlds) const;

            scalarField cellToPoint(const scalarField& vFld) const;

            scalarField error
            (
                const scalarField& fld,
                const scalar minLevel,
                const scalar maxLevel
            ) const;

            //- Select candidate cells for refinement
            virtual void selectRefineCandidates
            (
                const scalar lowerRefineLevel,
                const scalar upperRefineLevel,
                const label  nBufferLayers,
                const List<scalarField*> &vFlds,
                PackedBoolList& candidateCell
            ) const;

            //- Subset candidate cells for refinement
            virtual labelList selectRefineCells
            (
                const label maxCells,
                const label maxRefinement,
                const PackedBoolList& candidateCell
            ) const;

            //- Select points that can be unrefined.
            virtual labelList selectUnrefinePoints
            (
                const scalar unrefineLevel,
                const PackedBoolList& markedCell,
                const scalarField& pFld
            ) const;

            //- Extend markedCell with cell-face-cell.
            void extendMarkedCells(PackedBoolList& markedCell) const;

            //- Check all cells have 8 anchor points
            void checkEightAnchorPoints
            (
                PackedBoolList& protectedCell,
                label& nProtected
            ) const;

private:

        //- Disallow default bitwise copy construct
        LSMdynamicRefineFvMesh(const LSMdynamicRefineFvMesh&);

        //- Disallow default bitwise assignment
        void operator=(const LSMdynamicRefineFvMesh&);

public:

    //- Runtime type information
    TypeName("LSMdynamicRefineFvMesh");


    // Constructors

        //- Construct from IOobject
        explicit LSMdynamicRefineFvMesh(const IOobject& io);


    //- Destructor
    virtual ~LSMdynamicRefineFvMesh();


    // Member Functions

        //- Direct access to the refinement engine
        const hexRef8& meshCutter() const
        {
            return meshCutter_;
        }

        //- Cells which should not be refined/unrefined
        const PackedBoolList& protectedCell() const
        {
            return protectedCell_;
        }

        //- Cells which should not be refined/unrefined
        PackedBoolList& protectedCell()
        {
            return protectedCell_;
        }

        //- Update the mesh for both mesh motion and topology change
        virtual bool update();


    // Writing

        //- Write using given format, version and compression
//Vora OF-3
/*        virtual bool writeObject
        (
            IOstream::streamFormat fmt,
            IOstream::versionNumber ver,
            IOstream::compressionType cmp
        ) const;
*/

//Vora
//In OF-5.x: the new structure of writeObject() requires fourth entity as "const bool valid".
//see the "dynamicRefineFvMesh.C" 

        virtual bool writeObject
        (
            IOstream::streamFormat fmt,
            IOstream::versionNumber ver,
            IOstream::compressionType cmp,
	    const bool	valid
        ) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
