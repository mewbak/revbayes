
#include "ArgumentRule.h"
#include "ArgumentRules.h"
#include "Ellipsis.h"
#include "FileMonitor.h"
#include "ModelVector.h"
#include "Mntr_File.h"
#include "Natural.h"
#include "RbException.h"
#include "RevObject.h"
#include "RlString.h"
#include "TypeSpec.h"

#include <algorithm>
#include <string>

using namespace RevLanguage;

Mntr_File::Mntr_File(void) : Monitor() {
    
}


/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'b'.
 *
 * \return A new copy of the process.
 */
Mntr_File* Mntr_File::clone(void) const
{
    
	return new Mntr_File(*this);
}


void Mntr_File::constructInternalObject( void )
{
    // we free the memory first
    delete value;
    
    // now allocate a new sliding move
    const std::string& fn = static_cast<const RlString &>( filename->getRevObject() ).getValue();
    const std::string& sep = static_cast<const RlString &>( separator->getRevObject() ).getValue();
    int g = (int)static_cast<const Natural &>( printgen->getRevObject() ).getValue();
    
    // sort, remove duplicates, the create monitor vector
    vars.erase( unique( vars.begin(), vars.end() ), vars.end() );
    sort( vars.begin(), vars.end(), compareVarNames );
    std::vector<RevBayesCore::DagNode *> n;
    for (std::vector<RevPtr<const RevVariable> >::iterator i = vars.begin(); i != vars.end(); ++i)
    {
        RevBayesCore::DagNode* node = (*i)->getRevObject().getDagNode();
        n.push_back( node );
    }
    bool pp = static_cast<const RlBoolean &>( posterior->getRevObject() ).getValue();
    bool l = static_cast<const RlBoolean &>( likelihood->getRevObject() ).getValue();
    bool pr = static_cast<const RlBoolean &>( prior->getRevObject() ).getValue();
    bool app = static_cast<const RlBoolean &>( append->getRevObject() ).getValue();
    bool wv = static_cast<const RlBoolean &>( version->getRevObject() ).getValue();
    
    value = new RevBayesCore::FileMonitor(n, (unsigned long)g, fn, sep, pp, l, pr, app, wv);
}

/** Get Rev type of object */
const std::string& Mntr_File::getClassType(void)
{
    
    static std::string rev_type = "Mntr_File";
    
	return rev_type; 
}

/** Get class type spec describing type of object */
const TypeSpec& Mntr_File::getClassTypeSpec(void)
{
    
    static TypeSpec rev_type_spec = TypeSpec( getClassType(), new TypeSpec( Monitor::getClassTypeSpec() ) );
    
	return rev_type_spec; 
}


/**
 * Get the Rev name for the constructor function.
 *
 * \return Rev name of constructor function.
 */
std::string Mntr_File::getMonitorName( void ) const
{
    // create a constructor function name variable that is the same for all instance of this class
    std::string c_name = "File";
    
    return c_name;
}


/** Return member rules (no members) */
const MemberRules& Mntr_File::getParameterRules(void) const
{
    
    static MemberRules filemonitorMemberRules;
    static bool rules_set = false;
    
    if ( !rules_set )
    {
        
        filemonitorMemberRules.push_back( new Ellipsis( "Variables to monitor", RevObject::getClassTypeSpec() ) );
        filemonitorMemberRules.push_back( new ArgumentRule("filename"  , RlString::getClassTypeSpec() , "The name of the file.", ArgumentRule::BY_VALUE, ArgumentRule::ANY ) );
        filemonitorMemberRules.push_back( new ArgumentRule("printgen"  , Natural::getClassTypeSpec()  , "How often should we print.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new Natural(1) ) );
        filemonitorMemberRules.push_back( new ArgumentRule("separator" , RlString::getClassTypeSpec() , "The separator/delimiter between values.", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlString("\t") ) );
        filemonitorMemberRules.push_back( new ArgumentRule("posterior" , RlBoolean::getClassTypeSpec(), "Should we print the posterior probability as well?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(true) ) );
        filemonitorMemberRules.push_back( new ArgumentRule("likelihood", RlBoolean::getClassTypeSpec(), "Should we print the likelihood as well?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(true) ) );
        filemonitorMemberRules.push_back( new ArgumentRule("prior"     , RlBoolean::getClassTypeSpec(), "Should we print the prior probability as well?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(true) ) );
        filemonitorMemberRules.push_back( new ArgumentRule("append"    , RlBoolean::getClassTypeSpec(), "Should we append or overwrite if the file exists?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );
        filemonitorMemberRules.push_back( new ArgumentRule("version", RlBoolean::getClassTypeSpec(), "Should we record the software version?", ArgumentRule::BY_VALUE, ArgumentRule::ANY, new RlBoolean(false) ) );


        rules_set = true;
    }
    
    return filemonitorMemberRules;
}

/** Get type spec */
const TypeSpec& Mntr_File::getTypeSpec( void ) const
{
    
    static TypeSpec type_spec = getClassTypeSpec();
    
    return type_spec;
}


/** Get type spec */
void Mntr_File::printValue(std::ostream &o) const {
    
    o << "Mntr_File";
}


/** Set a member variable */
void Mntr_File::setConstParameter(const std::string& name, const RevPtr<const RevVariable> &var) {
    
    if ( name == "" )
    {
        vars.push_back( var );
    }
    else if ( name == "filename" )
    {
        filename = var;
    }
    else if ( name == "separator" )
    {
        separator = var;
    }
    else if ( name == "printgen" )
    {
        printgen = var;
    }
    else if ( name == "prior" )
    {
        prior = var;
    }
    else if ( name == "posterior" )
    {
        posterior = var;
    }
    else if ( name == "likelihood" )
    {
        likelihood = var;
    }
    else if (name == "append")
    {
        append = var;
    }
    else if (name == "version")
    {
        version = var;
    }
    else
    {
        RevObject::setConstParameter(name, var);
    }
}
