/* 
 * File:   RlMultivariatePhyloProcess.cpp
 * Author: nl
 * 
 * Created on 16 juillet 2014, 19:54
 */

#include "RlMultivariatePhyloProcess.h"

#include "Natural.h"
#include "RbUtil.h"
#include "RlMemberFunction.h"
#include "RlString.h"
#include "RealPos.h"
#include "TypeSpec.h"
#include "Vector.h"

#include <sstream>

using namespace RevLanguage;

/** Default constructor */
MultivariatePhyloProcess::MultivariatePhyloProcess(void) : ModelObject<RevBayesCore::MultivariatePhyloProcess>() {
    
}

/** Construct from bool */
MultivariatePhyloProcess::MultivariatePhyloProcess(RevBayesCore::MultivariatePhyloProcess *t) : ModelObject<RevBayesCore::MultivariatePhyloProcess>( t ) {
    
}

/** Construct from bool */
MultivariatePhyloProcess::MultivariatePhyloProcess(const RevBayesCore::MultivariatePhyloProcess &t) : ModelObject<RevBayesCore::MultivariatePhyloProcess>( new RevBayesCore::MultivariatePhyloProcess( t ) ) {
    
}

/** Construct from bool */
MultivariatePhyloProcess::MultivariatePhyloProcess(RevBayesCore::TypedDagNode<RevBayesCore::MultivariatePhyloProcess> *n) : ModelObject<RevBayesCore::MultivariatePhyloProcess>( n ) {
    
}



/** Construct from bool */
MultivariatePhyloProcess::MultivariatePhyloProcess(const MultivariatePhyloProcess &t) : ModelObject<RevBayesCore::MultivariatePhyloProcess>( t ) {
    
}


/** Clone object */
MultivariatePhyloProcess* MultivariatePhyloProcess::clone(void) const {
    
	return new MultivariatePhyloProcess(*this);
}


/* Map calls to member methods */
RevLanguage::RevObject* MultivariatePhyloProcess::executeMethod(std::string const &name, const std::vector<Argument> &args) {
    
    if (name == "rootVal") {        
        RevBayesCore::TypedDagNode< int >* k = static_cast<const Integer &>( args[0].getVariable()->getRevObject() ).getDagNode();
        double mean = this->dagNode->getValue().getRootVal(k->getValue());
        return new Real( mean );
    }
    
    return ModelObject<RevBayesCore::MultivariatePhyloProcess>::executeMethod( name, args );
}


/** Get class name of object */
const std::string& MultivariatePhyloProcess::getClassName(void) { 
    
    static std::string rbClassName = "MultivariatePhyloProcess";
    
	return rbClassName; 
}

/** Get class type spec describing type of object */
const TypeSpec& MultivariatePhyloProcess::getClassTypeSpec(void) { 
    
    static TypeSpec rbClass = TypeSpec( getClassName(), new TypeSpec( RevObject::getClassTypeSpec() ) );
    
	return rbClass; 
}


/* Get method specifications */
const RevLanguage::MethodTable& MultivariatePhyloProcess::getMethods(void) const {
    
    static MethodTable    methods                     = MethodTable();
    static bool           methodsSet                  = false;
    
    if ( methodsSet == false )
    {
        
        ArgumentRules* meanArgRules = new ArgumentRules();
        meanArgRules->push_back(new ArgumentRule("index", false, Natural::getClassTypeSpec()));
        methods.addFunction("mean", new MemberFunction<MultivariatePhyloProcess,Real>( this, meanArgRules ) );
        
        ArgumentRules* stdevArgRules = new ArgumentRules();
        stdevArgRules->push_back(new ArgumentRule("index", false, Natural::getClassTypeSpec()));
        methods.addFunction("stdev", new MemberFunction<MultivariatePhyloProcess,RealPos>(  this, stdevArgRules ) );
        
        ArgumentRules* rootArgRules = new ArgumentRules();
        rootArgRules->push_back(new ArgumentRule("index", false, Natural::getClassTypeSpec()));
        methods.addFunction("rootVal", new MemberProcedure(Real::getClassTypeSpec(), rootArgRules ) );
        
        // necessary call for proper inheritance
        methods.setParentTable( &ModelObject<RevBayesCore::MultivariatePhyloProcess>::getMethods() );
        methodsSet = true;
    }
    
    
    return methods;
}


/** Get type spec */
const TypeSpec& MultivariatePhyloProcess::getTypeSpec( void ) const {
    
    static TypeSpec typeSpec = getClassTypeSpec();
    
    return typeSpec;
}


/** Print value for user */
void MultivariatePhyloProcess::printValue(std::ostream &os) const {

    /*
    long previousPrecision = o.precision();
    std::ios_base::fmtflags previousFlags = o.flags();
    
    std::fixed( o );
    o.precision( 3 );
    o << dagNode->getValue();
    
    o.setf( previousFlags );
    o.precision( previousPrecision );

    */
    
    os << dagNode->getValue();

    /*
    RevBayesCore::MultivariatePhyloProcess x = dagNode->getValue();
    
    os << x << '\t';

    for (size_t i=0; i<x.getDim(); i++)   {
        os << x.getMean(i) << '\t';
    }
    
    for (size_t i=0; i<x.getDim(); i++)   {
        os << x.getStdev(i) << '\t';
    }    
    
    for (size_t i=0; i<x.getDim(); i++)   {
        os << x.getRootVal(i) << '\t';
    }    
    */
}


