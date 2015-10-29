#ifndef Move_MixtureAllocation_H
#define Move_MixtureAllocation_H

#include "RlMove.h"
#include "TypedDagNode.h"

#include <ostream>
#include <string>

namespace RevLanguage {
    
    /**
     * @brief Rev Wrapper of a mixture re-allocation move.
     *
     * This class is the RevLanguage wrapper of MixtureAllocationMove.
     *
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @copyright GPL version 3
     * @since 2014-08-18, version 1.0
     */
    template <class rlValueType>
    class Move_MixtureAllocation : public Move {
        
    public:
        
        Move_MixtureAllocation(void);                                                                                                               //!< Default constructor
        
        // Basic utility functions
        virtual Move_MixtureAllocation*             clone(void) const;                                                                              //!< Clone the object
        void                                        constructInternalObject(void);                                                                  //!< We construct the a new internal move.
        static const std::string&                   getClassType(void);                                                                             //!< Get Rev type
        static const TypeSpec&                      getClassTypeSpec(void);                                                                         //!< Get class type spec
        std::string                                 getConstructorFunctionName(void) const;                                                         //!< Get the name used for the constructor function in Rev.
        const MemberRules&                          getParameterRules(void) const;                                                                  //!< Get member rules (const)
        virtual const TypeSpec&                     getTypeSpec(void) const;                                                                        //!< Get language type of the object
        virtual void                                printValue(std::ostream& o) const;                                                              //!< Print value (for user)
        
    protected:
        
        void                                        setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var);               //!< Set member variable
        
        RevPtr<const RevVariable>                   x;                                                                                              //!< The variable holding the real valued vector.
        RevPtr<const RevVariable>                   delta;                                                                                          //!< The width for proposing new allocations (default 0, uniform random sampling)
        
    };
    
}


#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "MetropolisHastingsMove.h"
#include "ModelVector.h"
#include "MixtureAllocationProposal.h"
#include "RbException.h"
#include "TypedDagNode.h"
#include "TypeSpec.h"


using namespace RevLanguage;


template <class rlValueType>
RevLanguage::Move_MixtureAllocation<rlValueType>::Move_MixtureAllocation() : Move()
{
    
}


/** Clone object */
template <class rlValueType>
RevLanguage::Move_MixtureAllocation<rlValueType>* Move_MixtureAllocation<rlValueType>::clone(void) const
{
    
	return new Move_MixtureAllocation<rlValueType>(*this);
}


template <class rlValueType>
void RevLanguage::Move_MixtureAllocation<rlValueType>::constructInternalObject( void )
{
    // we free the memory first
    delete value;
    
    // now allocate a new vector-scale move
    size_t d = size_t( static_cast<const Natural &>( delta->getRevObject() ).getValue() );
    double w = static_cast<const RealPos &>( weight->getRevObject() ).getValue();
    RevBayesCore::TypedDagNode<typename rlValueType::valueType>* tmp = static_cast<const rlValueType &>( x->getRevObject() ).getDagNode();
    RevBayesCore::StochasticNode< typename rlValueType::valueType > *sn = static_cast<RevBayesCore::StochasticNode< typename rlValueType::valueType > *>( tmp );
    
    RevBayesCore::Proposal *p = new RevBayesCore::MixtureAllocationProposal<typename rlValueType::valueType>( sn, d );
    value = new RevBayesCore::MetropolisHastingsMove(p, w, false);

}


/** Get Rev type of object */
template <class rlValueType>
const std::string& RevLanguage::Move_MixtureAllocation<rlValueType>::getClassType(void)
{
    
//    static std::string revType = "Move_MixtureAllocation<" + rlValueType::getClassType() + ">";
    static std::string revType = "Move_MixtureAllocation__" + rlValueType::getClassType();
    
	return revType;
}

/** Get class type spec describing type of object */
template <class rlValueType>
const RevLanguage::TypeSpec& RevLanguage::Move_MixtureAllocation<rlValueType>::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( Move::getClassTypeSpec() ) );
    
	return revTypeSpec;
}


/**
 * Get the Rev name for the constructor function.
 *
 * \return Rev name of constructor function.
 */
template <class rlValueType>
std::string RevLanguage::Move_MixtureAllocation<rlValueType>::getConstructorFunctionName( void ) const
{
    // create a constructor function name variable that is the same for all instance of this class
    std::string c_name = "mvMixtureAllocation";
    
    return c_name;
}


/** Return member rules (no members) */
template <class rlValueType>
const RevLanguage::MemberRules& RevLanguage::Move_MixtureAllocation<rlValueType>::getParameterRules(void) const
{
    
    static MemberRules moveMemberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet )
    {
        moveMemberRules.push_back( new ArgumentRule( "x", rlValueType::getClassTypeSpec(), "The variable on which this move operates.", ArgumentRule::BY_REFERENCE, ArgumentRule::STOCHASTIC ) );
        moveMemberRules.push_back( new ArgumentRule( "delta", Natural::getClassTypeSpec(), "The window of how many categories to propose left and right.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new Natural(0)));
        
        /* Inherit weight from Move, put it after variable */
        const MemberRules& inheritedRules = Move::getParameterRules();
        moveMemberRules.insert( moveMemberRules.end(), inheritedRules.begin(), inheritedRules.end() );
        
        rulesSet = true;
    }
    
    return moveMemberRules;
}

/** Get type spec */
template <class rlValueType>
const RevLanguage::TypeSpec& RevLanguage::Move_MixtureAllocation<rlValueType>::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Get type spec */
template <class rlValueType>
void RevLanguage::Move_MixtureAllocation<rlValueType>::printValue(std::ostream &o) const
{
    
    o << "Move_MixtureAllocation(";
    if (x != NULL)
    {
        o << x->getName();
    }
    else
    {
        o << "?";
    }
    o << ")";
}


/** Set a member variable */
template <class rlValueType>
void RevLanguage::Move_MixtureAllocation<rlValueType>::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var) {
    
    if ( name == "x" )
    {
        x = var;
    }
    else if ( name == "delta" )
    {
        delta = var;
    }
    else
    {
        Move::setConstParameter(name, var);
    }
}


#endif
