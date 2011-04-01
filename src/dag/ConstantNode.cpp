/**
 * @file
 * This file contains the implementation of ConstantNode, which is derived
 * from ConstantNode. ConstantNode is used for DAG nodes holding constant
 * values, and generally for variables used in RevBayes.
 *
 * @brief Implementation of ConstantNode
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-11-19 17:29:33 +0100 (Tor, 19 Nov 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-16, version 1.0
 * @extends ConstantNode
 *
 * $Id: ConstantNode.h 69 2009-11-19 16:29:33Z ronquist $
 */


#include "ConstantNode.h"
#include "RbException.h"
#include "RbNames.h"
#include "StochasticNode.h"
#include "VectorString.h"
#include "Workspace.h"

#include <cassert>
#include <iostream>
#include <string>
#include <set>


/** Constructor from value */
ConstantNode::ConstantNode(RbObject* val) : DAGNode(val->getType()) {

    value = val;
    if ( val->getDim() > 0 )
        throw RbException( "ConstantNode cannot hold container objects. Use a ContainerNode instead." );
}


/** Constructor from value class */
ConstantNode::ConstantNode(const std::string& valType) : DAGNode(valType) {

    if ( Workspace::userWorkspace().isXOfTypeY(valType, Container_name) )
        throw RbException( "ConstantNode cannot hold container objects. Use a ContainerNode instead." );
}


/** Copy constructor */
ConstantNode::ConstantNode(const ConstantNode& x) : DAGNode(x) {

    value = x.value->clone();
}

/** Destructor */
ConstantNode::~ConstantNode(void) {

    if (numRefs() != 0)
        throw RbException ("Cannot delete node with references"); 
    delete value;
}

/** Assignment operator */
ConstantNode& ConstantNode::operator=(const ConstantNode& x) {

    if (this != &x) {

        if (valueType != x.valueType)
            throw RbException("Type mismatch");

        delete value;
        value = x.value->clone();
    }

    return (*this);
}

/** Clone this object */
ConstantNode* ConstantNode::clone(void) const {

    return new ConstantNode(*this);
}

/** Cloning the entire graph only involves children for a constant node */
ConstantNode* ConstantNode::cloneDAG(std::map<DAGNode*, DAGNode*>& newNodes) const {

    if (newNodes.find((DAGNode*)(this)) != newNodes.end())
        return (ConstantNode*)(newNodes[(DAGNode*)(this)]);

    /* Make pristine copy */
    ConstantNode* copy = clone();
    newNodes[(DAGNode*)(this)] = copy;

    /* Make sure the children clone themselves */
    for(std::set<VariableNode*>::const_iterator i=children.begin(); i!=children.end(); i++) {
        (*i)->cloneDAG(newNodes);
    }
 
    return copy;
}

/** Get class vector describing type of DAG node */
const VectorString& ConstantNode::getDAGClass() const {

    static VectorString rbClass = VectorString(ConstantNode_name) + DAGNode::getDAGClass();
    return rbClass;
}


/**
 * @brief Is the node a constant expression?
 *
 * We can only guarantee that the node is a constant expression if the
 * node cannot be mutated; this is the case if the constant node does
 * not belong to a variable slot.
 *
 */
bool ConstantNode::isConstExpr(void) const {

    return false;
}


/** Is it possible to mutate node to newNode? */
bool ConstantNode::isMutableTo(const DAGNode* newNode) const {

    return false;
}


/** Mutate to newNode */
void ConstantNode::mutateTo(DAGNode* newNode) {
    
    throw RbException("Not implemented yet");
}


/** Print value for user */
void ConstantNode::printValue(std::ostream& o) const {

    value->printValue(o);
}


/** Print struct for user */
void ConstantNode::printStruct(std::ostream &o) const {

    o << "Wrapper:" << std::endl;
    o << "_class   = " << getDAGClass() << std::endl;
    o << "_value   = " << value << std::endl;
    o << "_parents = NULL" << std::endl;
    o << "_children" << std::endl;
    printChildren(o);
    o << std::endl;
    o << std::endl;
}


/** Complete info on object */
std::string ConstantNode::richInfo(void) const {

    std::ostringstream o;
    o << "ConstantNode: value = ";
    value->printValue(o);

    return o.str();
}


/** Touch affected: only needed if a set function is called */
void ConstantNode::touchAffected(void) {

    for (std::set<VariableNode*>::const_iterator i=children.begin(); i!=children.end(); i++)
        (*i)->touchAffected();
}

