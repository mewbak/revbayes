 ///
//  TreeHistory.h
//  rb_mlandis
//
//  Created by Michael Landis on 3/28/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#ifndef __rb_mlandis__AbstractTreeHistoryCtmc__
#define __rb_mlandis__AbstractTreeHistoryCtmc__

#include "AbstractCharacterData.h"
#include "BranchHistory.h"
#include "DiscreteTaxonData.h"
#include "DiscreteCharacterData.h"
#include "DiscreteCharacterState.h"
#include "DnaState.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RateMatrix.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"
#include "Tree.h"
#include "TreeChangeEventListener.h"
#include "TypedDistribution.h"

#include <cmath>

namespace RevBayesCore {

    template<class charType, class treeType>
    class AbstractTreeHistoryCtmc : public TypedDistribution< AbstractCharacterData >, public TreeChangeEventListener {
        
    public:
        // Note, we need the size of the alignment in the constructor to correctly simulate an initial state
        AbstractTreeHistoryCtmc(const TypedDagNode<treeType> *t, size_t nChars, size_t nSites, bool useAmbigChar=false);
        AbstractTreeHistoryCtmc(const AbstractTreeHistoryCtmc &n);                                                                           //!< Copy constructor
        virtual                                                            ~AbstractTreeHistoryCtmc(void);                                   //!< Virtual destructor
        
        // public member functions
        // pure virtual
        virtual AbstractTreeHistoryCtmc*                                    clone(void) const = 0;                                           //!< Create an independent clone
        virtual void                                                        redrawValue(void) = 0;
        virtual void                                                        initializeValue(void) = 0;
        virtual double                                                      samplePathStart(const TopologyNode& node, const std::set<size_t>& indexSet) = 0;
        virtual double                                                      samplePathEnd(const TopologyNode& node, const std::set<size_t>& indexSet) = 0;
        virtual double                                                      samplePathHistory(const TopologyNode& node, const std::set<size_t>& indexSet) = 0;
        
        // virtual (you need to overwrite this method if you have additional parameters)
        virtual void                                                        swapParameter(const DagNode *oldP, const DagNode *newP);         //!< Implementation of swaping paramoms
        
        // non-virtual
        double                                                              computeLnProbability(void);
        void                                                                fireTreeChangeEvent(const TopologyNode &n);                      //!< The tree has changed and we want to know which part.
        BranchHistory&                                                      getHistory(size_t idx);
        const BranchHistory&                                                getHistory(size_t idx) const;
        std::vector<BranchHistory*>                                         getHistories(void);
        const std::vector<BranchHistory*>&                                  getHistories(void) const;
        void                                                                reInitialized(void);
        void                                                                setHistory(const BranchHistory& bh, size_t idx);
        void                                                                setHistories(const std::vector<BranchHistory>& bh);
        void                                                                setValue(AbstractCharacterData *v);                              //!< Set the current value, e.g. attach an observation (clamp)
        virtual void                                                        simulate(void);

        
    protected:
        // helper method for this and derived classes
        void                                                                recursivelyFlagNodeDirty(const TopologyNode& n);
        void                                                                resizeLikelihoodVectors(void);
        
        // virtual methods that may be overwritten, but then the derived class should call this methods
        virtual void                                                        keepSpecialization(DagNode* affecter);
        virtual void                                                        restoreSpecialization(DagNode *restorer);
        virtual void                                                        touchSpecialization(DagNode *toucher);
        
        // pure virtual methods
        virtual void                                                        computeRootLikelihood(const TopologyNode &n, size_t root) = 0;
        virtual void                                                        computeInternalNodeLikelihood(const TopologyNode &n, size_t nIdx) = 0;
        virtual void                                                        computeTipLikelihood(const TopologyNode &node, size_t nIdx) = 0;
        virtual void                                                        updateTransitionProbabilities(size_t nodeIdx, double brlen) = 0;
        virtual const std::vector<double>&                                  getRootFrequencies(void) = 0;
        
        // members
        double                                                              lnProb;
        const size_t                                                        numChars;
        size_t                                                              numSites;
        size_t                                                              numSiteRates;
        const TypedDagNode<treeType>*                                       tau;
        std::vector<TransitionProbabilityMatrix>                            transitionProbMatrices;
        
        // the likelihoods
        double*                                                             partialLikelihoods;
        std::vector<size_t>                                                 activeLikelihood;
        std::vector<size_t>                                                 activeHistory;
        std::vector<double>                                                 historyLikelihoods;
        
        // the data
        std::vector<std::vector<unsigned long> >                            charMatrix;
        std::vector<std::vector<bool> >                                     gapMatrix;
        std::vector<size_t>                                                 patternCounts;
        size_t                                                              numPatterns;
        bool                                                                compressed;
        std::vector<BranchHistory*>                                          histories;
        
        // convenience variables available for derived classes too
        std::vector<bool>                                                   changedNodes;
        std::vector<bool>                                                   dirtyNodes;
        
        // offsets for nodes
        size_t                                                              activeLikelihoodOffset;
        size_t                                                              nodeOffset;
        size_t                                                              mixtureOffset;
        size_t                                                              siteOffset;
        
        // flags
        bool                                                                usingAmbiguousCharacters;
        bool                                                                treatUnknownAsGap;
        bool                                                                treatAmbiguousAsGaps;
        bool                                                                tipsInitialized;
        
    private:
        // private methods
        // void                                                                compress(void);
        void                                                                fillLikelihoodVector(const TopologyNode &n, size_t nIdx);
        void                                                                initializeHistoriesVector(void);
//        virtual void                                                        simulate(const TopologyNode& node, std::vector< DiscreteTaxonData< charType > > &t);
        virtual void                                                        simulate(const TopologyNode& node, DiscreteCharacterData< charType > &taxa);
        

        
        
    };

}

template<class charType, class treeType>
RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::AbstractTreeHistoryCtmc(const TypedDagNode<treeType> *t, size_t nChars, size_t nSites, bool useAmbigChar) : TypedDistribution< AbstractCharacterData >(  new DiscreteCharacterData<charType>() ),
numChars( nChars ),
numSites( nSites ),
numSiteRates( 1 ),
tau( t ),
transitionProbMatrices( std::vector<TransitionProbabilityMatrix>(numSiteRates, TransitionProbabilityMatrix(numChars) ) ),
partialLikelihoods( new double[2*tau->getValue().getNumberOfNodes()*numSiteRates*numSites*numChars] ),
activeLikelihood( std::vector<size_t>(tau->getValue().getNumberOfNodes(), 0) ),
activeHistory(),
historyLikelihoods(),
charMatrix(),
gapMatrix(),
patternCounts(),
numPatterns( numSites ),
compressed( false ),
histories(),
changedNodes( std::vector<bool>(tau->getValue().getNumberOfNodes(),false) ),
dirtyNodes( std::vector<bool>(tau->getValue().getNumberOfNodes(), true) ),
usingAmbiguousCharacters( useAmbigChar ),
treatUnknownAsGap( true ),
treatAmbiguousAsGaps( true ),
tipsInitialized( false )
{
    
    // add the paramoms to the parents list
    this->addParameter( tau );
    tau->getValue().getTreeChangeEventHandler().addListener( this );
    
    // initialize histories
    initializeHistoriesVector();
    
    activeLikelihoodOffset      =  tau->getValue().getNumberOfNodes()*numSiteRates*numPatterns*numChars;
    nodeOffset                  =  numSiteRates*numPatterns*numChars;
    mixtureOffset               =  numPatterns*numChars;
    siteOffset                  =  numChars;
    
}


template<class charType, class treeType>
RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::AbstractTreeHistoryCtmc(const AbstractTreeHistoryCtmc &n) : TypedDistribution< AbstractCharacterData >( n ),
numChars( n.numChars ),
numSites( n.numSites ),
numSiteRates( n.numSiteRates ),
tau( n.tau ),
transitionProbMatrices( n.transitionProbMatrices ),
partialLikelihoods( new double[2*tau->getValue().getNumberOfNodes()*numSiteRates*numSites*numChars] ),
activeLikelihood( n.activeLikelihood ),
activeHistory( n.activeHistory ),
historyLikelihoods( n.historyLikelihoods ),
charMatrix( n.charMatrix ),
gapMatrix( n.gapMatrix ),
patternCounts( n.patternCounts ),
numPatterns( n.numPatterns ),
compressed( n.compressed ),
histories( n.histories ),
changedNodes( n.changedNodes ),
dirtyNodes( n.dirtyNodes ),
usingAmbiguousCharacters( n.usingAmbiguousCharacters ),
treatUnknownAsGap( n.treatUnknownAsGap ),
treatAmbiguousAsGaps( n.treatAmbiguousAsGaps ),
tipsInitialized( n.tipsInitialized )
{
    // parameters are automatically copied
    
    tau->getValue().getTreeChangeEventHandler().addListener( this );
    
    // copy the partial likelihoods
    memcpy(partialLikelihoods, n.partialLikelihoods, 2*tau->getValue().getNumberOfNodes()*numSiteRates*numPatterns*numChars*sizeof(double));
    
    activeLikelihoodOffset      =  tau->getValue().getNumberOfNodes()*numSiteRates*numPatterns*numChars;
    nodeOffset                  =  numSiteRates*numPatterns*numChars;
    mixtureOffset               =  numPatterns*numChars;
    siteOffset                  =  numChars;
    
}


template<class charType, class treeType>
RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::~AbstractTreeHistoryCtmc( void ) {
    // We don't delete the paramoms, because they might be used somewhere else too. The model needs to do that!
    
    // remove myself from the tree listeners
    if ( tau != NULL )
    {
        // TODO: this needs to be implemented (Sebastian)
        tau->getValue().getTreeChangeEventHandler().removeListener( this );
    }
    
    // free the partial likelihoods
    delete [] partialLikelihoods;
}


template<class charType, class treeType>
RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>* RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::clone( void ) const
{
    
    return new AbstractTreeHistoryCtmc<charType, treeType>( *this );
}

template<class charType, class treeType>
double RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::computeLnProbability( void )
{
    this->lnProb = 0.0;
    
    const std::vector<TopologyNode*>& nodes = tau->getValue().getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        dirtyNodes[i] = true;
        fillLikelihoodVector(*nodes[i], nodes[i]->getIndex());
        this->lnProb += historyLikelihoods[i];
    }
    
    //std::cout << "computeLnProbability " << this->lnProb << "\n";
    
    return this->lnProb;
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::fillLikelihoodVector(const TopologyNode &node, size_t nodeIndex)
{
    if (dirtyNodes[nodeIndex] == false)
        return;
    
    // mark as computed
    dirtyNodes[nodeIndex] = false;
    
//    if ( node.isTip() )
//        computeTipLikelihood(node, nodeIndex);
//    else
//    if (node.isRoot())
//        computeRootLikelihood(node, nodeIndex);
//    else
        computeInternalNodeLikelihood(node,nodeIndex);
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::fireTreeChangeEvent( const RevBayesCore::TopologyNode &n ) {
    
    // call a recursive flagging of all node above (closer to the root) and including this node
    // recursivelyFlagNodeDirty( n );
    dirtyNodes[n.getIndex()] = true;
    
}


template<class charType, class treeType>
const RevBayesCore::BranchHistory&  RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::getHistory(size_t idx) const
{
    return histories[idx];
}

template<class charType, class treeType>
RevBayesCore::BranchHistory&  RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::getHistory(size_t idx)
{
    return *histories[idx];
}

template<class charType, class treeType>
const std::vector<RevBayesCore::BranchHistory*>& RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::getHistories(void) const
{
    return histories;
}

template<class charType, class treeType>
std::vector<RevBayesCore::BranchHistory*> RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::getHistories(void)
{
    return histories;
}



template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::initializeHistoriesVector( void ) {
    
    std::vector<TopologyNode*> nodes = tau->getValue().getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        histories.push_back(new BranchHistory(numSites, numChars, i));
    }
    
    historyLikelihoods.resize(nodes.size(), 0.0);
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::keepSpecialization( DagNode* affecter ) {
    
    // reset all flags
    for (std::vector<bool>::iterator it = this->dirtyNodes.begin(); it != this->dirtyNodes.end(); ++it)
    {
        (*it) = false;
    }
    
    for (std::vector<bool>::iterator it = this->changedNodes.begin(); it != this->changedNodes.end(); ++it)
    {
        (*it) = false;
    }
    
}



template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::recursivelyFlagNodeDirty( const RevBayesCore::TopologyNode &n ) {
    
    // we need to flag this node and all ancestral nodes for recomputation
    size_t index = n.getIndex();
    
    // if this node is already dirty, the also all the ancestral nodes must have been flagged as dirty
    if ( !dirtyNodes[index] )
    {
        // the root doesn't have an ancestor
        if ( !n.isRoot() )
        {
            recursivelyFlagNodeDirty( n.getParent() );
        }
        
        // set the flag
        dirtyNodes[index] = true;
        
        // if we previously haven't touched this node, then we need to change the active likelihood pointer
        if ( !changedNodes[index] )
        {
            activeLikelihood[index] ^= 1;
            changedNodes[index] = true;
        }
        
    }
    
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::reInitialized( void ) {
    
    // we need to recompress because the tree may have changed
    // compress();
}



template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::resizeLikelihoodVectors( void ) {
    
    // we resize the partial likelihood vectors to the new dimensions
    delete [] partialLikelihoods;
    partialLikelihoods = new double[2*tau->getValue().getNumberOfNodes()*numSiteRates*numPatterns*numChars];
    
    transitionProbMatrices = std::vector<TransitionProbabilityMatrix>(numSiteRates, TransitionProbabilityMatrix(numChars) );
    
    // set the offsets for easier iteration through the likelihood vector
    activeLikelihoodOffset      =  tau->getValue().getNumberOfNodes()*numSiteRates*numPatterns*numChars;
    nodeOffset                  =  numSiteRates*numPatterns*numChars;
    mixtureOffset               =  numPatterns*numChars;
    siteOffset                  =  numChars;
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::restoreSpecialization( DagNode* affecter ) {
    
    // reset the flags
    for (std::vector<bool>::iterator it = dirtyNodes.begin(); it != dirtyNodes.end(); ++it)
    {
        (*it) = false;
    }
    
    // restore the active likelihoods vector
    for (size_t index = 0; index < changedNodes.size(); ++index)
    {
        // we have to restore, that means if we have changed the active likelihood vector
        // then we need to revert this change
        if ( changedNodes[index] )
        {
            activeLikelihood[index] = (activeLikelihood[index] == 0 ? 1 : 0);
        }
        
        // set all flags to false
        changedNodes[index] = false;
    }
    
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::setHistory(const BranchHistory& bh, size_t idx)
{
    histories[idx] = new BranchHistory(bh);
}

template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::setHistories(const std::vector<BranchHistory>& bh)
{
    for (size_t i = 0; i < bh.size(); i++)
        histories[i] = bh[i];

}

template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::setValue(AbstractCharacterData *v) {
    
    // delegate to the parent class
    TypedDistribution< AbstractCharacterData >::setValue(v);
    
    // this->compress();
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::simulate(void)
{
    ;
}

template<class charType, class treeType>
//void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::simulate( const TopologyNode &node, std::vector< DiscreteTaxonData< charType > > &taxa)
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::simulate( const TopologyNode &node, DiscreteCharacterData< charType > &taxa)
{
//    
//    // get the children of the node
//    const std::vector<TopologyNode*>& children = node.getChildren();
//    
//    // get the sequence of this node
//    size_t nodeIndex = node.getIndex();
//    const DiscreteTaxonData< charType > &parent = taxa[ nodeIndex ];
//    
//    // simulate the sequence for each child
//    RandomNumberGenerator* rng = GLOBAL_RNG;
//    for (std::vector< TopologyNode* >::const_iterator it = children.begin(); it != children.end(); ++it)
//    {
//        const TopologyNode &child = *(*it);
//        
//        // update the transition probability matrix
//        updateTransitionProbabilities( child.getIndex(), child.getBranchLength() );
//        
//        DiscreteTaxonData< charType > &taxon = taxa[ child.getIndex() ];
//        for ( size_t i = 0; i < numSites; ++i )
//        {
//            // get the ancestral character for this site
//            unsigned long parentState = parent.getCharacter( i ).getState();
//            size_t p = 0;
//            while ( parentState != 1 )
//            {
//                // shift to the next state
//                parentState >>= 1;
//                // increase the index
//                ++p;
//            }
//            
//            double *freqs = transitionProbMatrices[ perSiteRates[i] ][ p ];
//            
//            // create the character
//            charType c;
//            c.setToFirstState();
//            // draw the state
//            double u = rng->uniform01();
//            while ( true )
//            {
//                u -= *freqs;
//                
//                if ( u > 0.0 )
//                {
//                    ++c;
//                    ++freqs;
//                }
//                else
//                {
//                    break;
//                }
//            }
//            
//            // add the character to the sequence
//            taxon.addCharacter( c );
//        }
//        
//        if ( child.isTip() )
//        {
//            taxon.setTaxonName( child.getName() );
//        }
//        else
//        {
//            // recursively simulate the sequences
//            simulate( child, taxa, perSiteRates );
//        }
//        
//    }
    
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::swapParameter(const DagNode *oldP, const DagNode *newP) {
    
    // we only have the topology here as the parameter
    if (oldP == tau)
    {
        tau->getValue().getTreeChangeEventHandler().removeListener( this );
        tau = static_cast<const TypedDagNode<treeType>* >( newP );
        tau->getValue().getTreeChangeEventHandler().addListener( this );
    }
    
}


template<class charType, class treeType>
void RevBayesCore::AbstractTreeHistoryCtmc<charType, treeType>::touchSpecialization( DagNode* affecter ) {
    
    // if the topology wasn't the culprit for the touch, then we just flag everything as dirty
    if ( affecter != tau )
    {
        for (std::vector<bool>::iterator it = dirtyNodes.begin(); it != dirtyNodes.end(); ++it)
        {
            (*it) = true;
        }
        
        // flip the active likelihood pointers
        for (size_t index = 0; index < changedNodes.size(); ++index)
        {
            if ( !changedNodes[index] )
            {
                activeLikelihood[index] = (activeLikelihood[index] == 0 ? 1 : 0);
                changedNodes[index] = true;
            }
        }
    }
    
}

#endif /* defined(__rb_mlandis__TreeHistory__) */
