//
//  Move_NodeTimeScale.cpp
//  RevBayes
//
//  Created by will freyman on 8/13/15.
//  Copyright (c) 2015 will freyman. All rights reserved.
//

#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "MetropolisHastingsMove.h"
#include "Move_NodeTimeScale.h"
#include "NodeTimeScaleProposal.h"
#include "RbException.h"
#include "RealPos.h"
#include "RevObject.h"
#include "RlTimeTree.h"
#include "TypedDagNode.h"
#include "TypeSpec.h"


using namespace RevLanguage;

Move_NodeTimeScale::Move_NodeTimeScale() : Move()
{
    
}


/** Clone object */
Move_NodeTimeScale* Move_NodeTimeScale::clone(void) const
{
    
    return new Move_NodeTimeScale(*this);
}


void Move_NodeTimeScale::constructInternalObject( void )
{
    
    // we free the memory first
    delete value;
    
    // now allocate a new sliding move
    RevBayesCore::TypedDagNode<RevBayesCore::Tree> *tmp = static_cast<const TimeTree &>( tree->getRevObject() ).getDagNode();
    double w = static_cast<const RealPos &>( weight->getRevObject() ).getValue();
    double l = static_cast<const RealPos &>( lambda->getRevObject() ).getValue();
    RevBayesCore::StochasticNode<RevBayesCore::Tree> *t = static_cast<RevBayesCore::StochasticNode<RevBayesCore::Tree> *>( tmp );
    RevBayesCore::Proposal *p = new RevBayesCore::NodeTimeScaleProposal(t, l);
    value = new RevBayesCore::MetropolisHastingsMove(p, w, t);
}

/** Get Rev type of object */
const std::string& Move_NodeTimeScale::getClassType(void)
{
    
    static std::string revType = "Move_NodeTimeScale";
    
    return revType;
}

/** Get class type spec describing type of object */
const TypeSpec& Move_NodeTimeScale::getClassTypeSpec(void)
{
    
    static TypeSpec revTypeSpec = TypeSpec( getClassType(), new TypeSpec( Move::getClassTypeSpec() ) );
    
    return revTypeSpec;
}



/** Return member rules (no members) */
const MemberRules& Move_NodeTimeScale::getParameterRules(void) const
{
    
    static MemberRules memberRules;
    static bool rulesSet = false;
    
    if ( !rulesSet )
    {
        
        memberRules.push_back( new ArgumentRule( "tree", TimeTree::getClassTypeSpec(), ArgumentRule::BY_REFERENCE, ArgumentRule::STOCHASTIC ) );
        memberRules.push_back( new ArgumentRule( "lambda", RealPos::getClassTypeSpec(), ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RealPos(1.0) ) );
        /* Inherit weight from Move, put it after variable */
        const MemberRules& inheritedRules = Move::getParameterRules();
        memberRules.insert( memberRules.end(), inheritedRules.begin(), inheritedRules.end() );
        
        rulesSet = true;
    }
    
    return memberRules;
}

/** Get type spec */
const TypeSpec& Move_NodeTimeScale::getTypeSpec( void ) const
{
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}



/** Get type spec */
void Move_NodeTimeScale::printValue(std::ostream &o) const
{
    
    o << "Move_NodeTimeScale(";
    if (tree != NULL)
    {
        o << tree->getName();
    }
    else
    {
        o << "?";
    }
    o << ")";
}


/** Set a NearestNeighborInterchange variable */
void Move_NodeTimeScale::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var)
{
    
    if ( name == "tree" )
    {
        tree = var;
    }
    else if ( name == "lambda" )
    {
        lambda = var;
    }
    else
    {
        Move::setConstParameter(name, var);
    }
}