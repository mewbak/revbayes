/**
 * @file
 * This file contains the implementation of RbFunction_sqrt, the
 * sqrt() function.
 *
 * @brief Implementation of RbFunction_sqrt
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author Fredrik Ronquist and the REvBayes core team
 * @license GPL version 3
 * @version 1.0
 * @since Version 1.0, 2009-08-26
 *
 * $Id$
 */

#include "RbFunction_ln.h"
#include "RbDouble.h"
#include "RbObject.h"
#include "DAGNode.h"
#include "RbException.h"
#include <cmath>

const StringVector RbFunction_ln::rbClass = StringVector("ln") + RbFunction::rbClass;

/** Define the argument rules */

/** Add to symbol table */
//static bool fxn_ln = SymbolTable::globalTable().add("ln", new RbFunction_ln());


/** Default constructor, allocate workspace */
RbFunction_ln::RbFunction_ln(void)
    : RbFunction(), value(new RbDouble(0)) {

	argRules.push_back( ArgumentRule("x", "double") );
	returnType = "double";
} 

/** Copy constructor */
RbFunction_ln::RbFunction_ln(const RbFunction_ln& s)
    : RbFunction(s), value(new RbDouble(0)) {
    
	argRules.push_back( ArgumentRule("x", "double") );
	returnType = "double";
}

/** Destructor, delete workspace */
RbFunction_ln::~RbFunction_ln() {

    delete value;
}

/**
 * @brief clone function
 *
 * This function creates a deep copy of this object.
 *
 * @see RbObject.clone()
 * @returns           return a copy of this object
 *
 */
RbObject* RbFunction_ln::clone(void) const {

    RbObject *x = new RbFunction_ln( *this );
    return x;
}

RbObject& RbFunction_ln::operator=(const RbObject& obj) {

    try {
        // Use built-in fast down-casting first
        const RbFunction_ln& x = dynamic_cast<const RbFunction_ln&> (obj);

        RbFunction_ln& y = (*this);
        y = x;
        return y;
    } catch (std::bad_cast & bce) {
        try {
            // Try converting the value to an argumentRule
            const RbFunction_ln& x = dynamic_cast<const RbFunction_ln&> (*(obj.convertTo("ln")));

            RbFunction_ln& y = (*this);
            y = x;
            return y;
        } catch (std::bad_cast & bce) {
            RbException e("Not supported assignment of " + obj.getClass()[0] + " to ln");
            throw e;
        }
    }

    // dummy return
    return (*this);
}

RbFunction_ln& RbFunction_ln::operator=(const RbFunction_ln& obj) {
    argRules = obj.argRules;
    returnType = obj.returnType;
    (*value) = (*obj.value);
    return (*this);
}

/**
 * @brief print function
 *
 * This function prints this object.
 *
 * @see RbObject.print()
 * @param c           the stream where to print to
 *
 */
void RbFunction_ln::print(std::ostream &c) const {

    c << "RbFunction_ln" << std::endl;
}

void RbFunction_ln::printValue(std::ostream &o) const {

    o << value << std::endl;
}

/**
 * @brief dump function
 *
 * This function dumps this object.
 *
 * @see RbObject.dump()
 * @param c           the stream where to dump to
 *
 */
void RbFunction_ln::dump(std::ostream& c){
    //TODO implement

    std::string message = "Dump function of RbFunction_ln not fully implemented!";
    RbException e;
    e.setMessage(message);
    throw e;
}

/**
 * @brief resurrect function
 *
 * This function resurrects this object.
 *
 * @see RbObject.resurrect()
 * @param x           the object from which to resurrect
 *
 */
void RbFunction_ln::resurrect(const RbDumpState& x){
    //TODO implement
    std::string message = "Resurrect function of RbFunction_ln not fully implemented!";
    RbException e;
    e.setMessage(message);
    throw e;
}

std::string RbFunction_ln::toString(void) const {

	char temp[30];
	sprintf(temp, "%1.6lf", value->getValue());
	std::string tempStr = temp;
    return "Value = " + tempStr;
}


/**
 * @brief overloaded == operators
 *
 * This function compares this object
 *
 * @param o           the object to compare to
 *
 */
bool RbFunction_ln::equals(const RbObject* o) const {

    return false;
}


/** Get number of argument rules */
const int RbFunction_ln::getNumberOfRules(void) const {
    return 1;
}

/** Execute function */
RbObject* RbFunction_ln::executeOperation(const std::vector<DAGNode*>& arguments) {

    /* Get actual argument */
    RbDouble *arg = (RbDouble*) arguments[0]->getValue();

    /* Compute result */
    if ( arg->getValue() < 0.0 )
        value->setValue(1E-100);
    else
        value->setValue(std::log(arg->getValue()));

    return value;
}

RbObject* RbFunction_ln::convertTo(const std::string& type) const {

    return NULL;
}

/**
 * @brief is convertible to
 *
 * This function checks if this data type can be converted into the given data type.
 *
 * @param dt         the data type we want to convert to
 * @returns          true, if it can be converted
 *
 */
bool RbFunction_ln::isConvertibleTo(const std::string& type) const {

    return false;
}
