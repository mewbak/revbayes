/**
 * @file
 * This file contains the abstract class for distributions implementing the distribution interface with the common functions.
 *
 * @brief Implementation of the abstract class Distribution
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date$
 * @author The REvBayes development core team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-09-08, version 1.0
 * @interface Distribution
 * @extends RbObject
 *
 * $Id$
 */

#include "Distribution.h"
#include "RbDouble.h"
#include "RbNames.h"



const StringVector Distribution::rbClass = StringVector(RbNames::Distribution::name) + RbObject::rbClass;


std::set<DAGNode*> Distribution::getParents() const {
    return parents;
}
