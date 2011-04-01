/**
 * @file
 * This file contains the declaration of StochasticNode, which is derived
 * from DAGNode. StochasticNode is used for DAG nodes holding stochastic
 * variables with an associated probability density function.
 *
 * @brief Declaration of StochasticNode
 *
 * (c) Copyright 2009- under GPL version 3
 * @date Last modified: $Date: 2009-12-30 00:19:25 +0100 (Ons, 30 Dec 2009) $
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 * @since 2009-08-16, version 1.0
 * @extends DAGNode
 *
 * $Id: StochasticNode.h 216 2009-12-29 23:19:25Z ronquist $
 */

#ifndef StochasticNode_H
#define StochasticNode_H

#include "VariableNode.h"

class Distribution;
class MemberNode;
class RbObject;
class VectorString;

class StochasticNode : public VariableNode {

    public:
                                StochasticNode(const std::string& type);                        //!< Construct empty stochastic node
                                StochasticNode(Distribution* dist);                             //!< Construct from distribution (raw object)
                                StochasticNode(const StochasticNode& s);                        //!< Copy constructor
							   ~StochasticNode(void);                                           //!< Destructor

        // Assignment operator
        StochasticNode&         operator=(const StochasticNode& x);                             //!< Assignment operator

        // Basic utility functions
        StochasticNode*         clone(void) const;                                              //!< Clone the stochastic node
        const VectorString&     getDAGClass(void) const;                                        //!< Get DAG node class vector
        const RbObject*         getStoredValue(void);                                           //!< Get stored value
        const RbObject*         getValue(void);                                                 //!< Get current value
        const RbObject*         getValue(void) const;                                           //!< Get const value if possible
        void                    printStruct(std::ostream& o) const;                             //!< Print struct for user
        void                    printValue(std::ostream& o) const;                              //!< Print struct for user
        std::string             richInfo(void) const;                                           //!< Complete info about object

        // Functions for setting the value (for functions, not for use by the parser, which is not allowed to set the value of a stochastic node)
        void                    setElement(const VectorInteger& index, RbObject* value);        //!< Set value element
        void                    setValue(RbObject* value);                                      //!< Set value

        // StochasticNode functions
        double                  calculateLnProbability(void);                                   //!< Calculate log conditional probability
        void                    clamp(RbObject* observedVal);                                   //!< Clamp the node with an observed value
        Distribution*           getDistribution(void) { return distribution; }                  //!< Get distribution
        double                  getLnLikelihoodRatio(void);                                     //!< Get log likelihood ratio
        double                  getLnPriorRatio(void);                                          //!< Get log prior ratio
        bool                    isClamped(void) const { return clamped; }                       //!< Is the node clamped?
        void                    unclamp(void);                                                  //!< Unclamp the node
        
        // DAG functions
        StochasticNode*         cloneDAG(std::map<DAGNode*, DAGNode*>& newNodes) const;         //!< Clone entire graph
        void                    getAffected(std::set<StochasticNode*>& affected);               //!< Mark and get affected nodes
        RbObject*               getValuePtr(std::set<StochasticNode*>& affected);               //!< Get value ptr + affected nodes
        bool                    isMutableTo(const DAGNode* newNode) const;                      //!< Is node mutable to newNode?
        bool                    isParentMutableTo(const DAGNode* oldNode, const DAGNode* newNode) const;  //!< Is parent mutable to newNode?
        void                    keep(void);                                                     //!< Keep value of this and affected nodes
        void    	            keepAffected(void);                                             //!< Keep value of affected nodes recursively
        void                    mutateTo(DAGNode* newNode);                                     //!< Mutate to new node
        void                    restore(void);                                                  //!< Restore value of this and affected nodes
        void                    restoreAffected(void);                                          //!< Restore value of  affected nodes recursively
        void                    swapParentNode(DAGNode* oldP, DAGNode* newP);                   //!< Swap a parent node
        void                    touchAffected(void) {}                                          //!< Tell affected nodes value is reset

        // Move function
        MoveSchedule*           getDefaultMoves(void);                                          //!< Get default moves

    protected:
        // Member variables
        bool                    clamped;                                                       //!< Is the node clamped with data?
        Distribution*           distribution;                                                  //!< Distribution (density functions, random draw function)
};

#endif

