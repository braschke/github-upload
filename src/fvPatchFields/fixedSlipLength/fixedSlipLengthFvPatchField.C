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

-------------------------------------------------------------------------------

Author

    Markus Buerger
    Chair of Fluid Mechanics
    markus.buerger@uni-wuppertal.de

    $Date$

License

    This file is contaminated by GNU General Public Licence.
    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

\*---------------------------------------------------------------------------*/


#include "fixedSlipLengthFvPatchField.H"
#include "mathematicalConstants.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
fixedSlipLengthFvPatchField<Type>::fixedSlipLengthFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(p, iF),
    slipLength_(0.0)
{}


template<class Type>
fixedSlipLengthFvPatchField<Type>::fixedSlipLengthFvPatchField
(
    const fixedSlipLengthFvPatchField<Type>& ptf,
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedValueFvPatchField<Type>(ptf, p, iF, mapper),
    slipLength_(ptf.slipLength_)
{}


template<class Type>
fixedSlipLengthFvPatchField<Type>::fixedSlipLengthFvPatchField
(
    const fvPatch& p,
    const DimensionedField<Type, volMesh>& iF,
    const dictionary& dict
)
:
    fixedValueFvPatchField<Type>(p, iF),
    slipLength_(dict.lookupOrDefault<scalar>("slipLength", 0.0))
{
    if (dict.found("value"))
    {
        fixedValueFvPatchField<Type>::operator==
        (
            Field<Type>("value", dict, p.size())
        );
    }
    else
    {
        fixedValueFvPatchField<Type>::operator==(pTraits<Type>::zero);
    }
}


template<class Type>
fixedSlipLengthFvPatchField<Type>::fixedSlipLengthFvPatchField
(
    const fixedSlipLengthFvPatchField<Type>& ptf
)
:
    fixedValueFvPatchField<Type>(ptf),
    slipLength_(ptf.slipLength_)
{}


template<class Type>
fixedSlipLengthFvPatchField<Type>::fixedSlipLengthFvPatchField
(
    const fixedSlipLengthFvPatchField<Type>& ptf,
    const DimensionedField<Type, volMesh>& iF
)
:
    fixedValueFvPatchField<Type>(ptf, iF),
    slipLength_(ptf.slipLength_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


template<class Type>
void fixedSlipLengthFvPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

    // Get patch normal
    tmp<vectorField>     tn  = this->patch().nf();
    const vectorField   &n  = tn();

    // make copy of values
    vectorField          Uwall = (*this);
    // subtract normal component --> tangential part
    Uwall = Uwall - (Uwall & n)*n;
    // get cell values near wall
    tmp<vectorField>     tU    = this->patchInternalField();
//    vectorField          &U    = tU();
    vectorField          &U    = tU.ref();//Vora: OF-5.x
    // subtract normal component --> tangential part
    U = U - (U & n)*n;
    // (compute normal gradient of tangential component) times slipLength
    vectorField          tauL  = (U - Uwall)*this->patch().deltaCoeffs()
                                             * slipLength_;
    // set fvPatchField
    (*this) == tauL;


    fixedValueFvPatchField<Type>::updateCoeffs();
}


template<class Type>
void fixedSlipLengthFvPatchField<Type>::write(Ostream& os) const
{
    fixedValueFvPatchField<Type>::write(os);
//    refValue_.writeEntry("refValue", os);
      os.writeKeyword("slipLength") << slipLength_ << token::END_STATEMENT <<nl;
//    amplitude_->writeData(os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
