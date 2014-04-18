//
//  BiogeographicTreeHistoryCtmc.h
//  rb_mlandis
//
//  Created by Michael Landis on 3/29/14.
//  Copyright (c) 2014 Michael Landis. All rights reserved.
//

#ifndef __rb_mlandis__BiogeographicTreeHistoryCtmc__
#define __rb_mlandis__BiogeographicTreeHistoryCtmc__

#include "AbstractTreeHistoryCtmc.h"
#include "DistributionExponential.h"
#include "RateMatrix.h"
#include "RbConstants.h"
#include "RbVector.h"
#include "StandardState.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"
#include "TreeChangeEventListener.h"
#include "TypedDistribution.h"

namespace RevBayesCore {
    
    template<class charType, class treeType>
    class BiogeographicTreeHistoryCtmc : public AbstractTreeHistoryCtmc<charType, treeType> {
        
    public:
        BiogeographicTreeHistoryCtmc(const TypedDagNode< treeType > *t, size_t nChars, size_t nSites);
        BiogeographicTreeHistoryCtmc(const BiogeographicTreeHistoryCtmc &n);                                                                         //!< Copy constructor
        virtual                                            ~BiogeographicTreeHistoryCtmc(void);                                                //!< Virtual destructor
        
        // public member functions
        
        BiogeographicTreeHistoryCtmc*                       clone(void) const;                                                           //!< Create an independent clone
        void                                                initializeValue(void);
        void                                                redrawValue(void);
        double                                              samplePathStart(const TopologyNode& node, const std::set<size_t>& indexSet);
        double                                              samplePathEnd(const TopologyNode& node, const std::set<size_t>& indexSet);
        double                                              samplePathHistory(const TopologyNode& node, const std::set<size_t>& indexSet);
        
        void                                                setClockRate(const TypedDagNode< double > *r);
        void                                                setClockRate(const TypedDagNode< std::vector< double > > *r);
        void                                                setRateMatrix(const TypedDagNode< RateMatrix > *rm);
        void                                                setRateMatrix(const TypedDagNode< RbVector< RateMatrix > > *rm);
        void                                                setRateMap(const TypedDagNode< RateMap > *rm);
        void                                                setRateMap(const TypedDagNode< RbVector< RateMap > > *rm);
        void                                                setRootFrequencies(const TypedDagNode< std::vector< double > > *f);
        void                                                setSiteRates(const TypedDagNode< std::vector< double > > *r);
        void                                                setDistancePower(const TypedDagNode<double>* dp);
        void                                                swapParameter(const DagNode *oldP, const DagNode *newP);                     //!< Implementation of swaping parameters
        
    protected:
        
        virtual void                                        computeRootLikelihood(size_t root, size_t l, size_t r);
        virtual void                                        computeInternalNodeLikelihood(const TopologyNode &n, size_t nIdx, size_t l, size_t r);
        virtual void                                        computeTipLikelihood(const TopologyNode &node, size_t nIdx);
        virtual const std::vector<double>&                  getRootFrequencies(void);
        // (not needed)        void                         keepSpecialization(DagNode* affecter);
        // (not needed)        void                         restoreSpecialization(DagNode *restorer);
        virtual void                                        touchSpecialization(DagNode *toucher);
        virtual void                                        updateTransitionProbabilities(size_t nodeIdx, double brlen);
        
        
    private:
        

        
        // helper function
        size_t                                              numOn(const std::vector<CharacterEvent*>& s);
        
        // members
        const TypedDagNode< double >*                       homogeneousClockRate;
        const TypedDagNode< std::vector< double > >*        heterogeneousClockRates;
        const TypedDagNode< RateMatrix >*                   homogeneousRateMatrix;
        const TypedDagNode< RbVector< RateMatrix > >*       heterogeneousRateMatrices;
        const TypedDagNode< std::vector< double > >*        rootFrequencies;
        const TypedDagNode< std::vector< double > >*        siteRates;
        const TypedDagNode< RateMap >*                      homogeneousRateMap;
        const TypedDagNode< RbVector< RateMap > >*          heterogeneousRateMaps;
        const TypedDagNode< double >*                       distancePower;
        
        // flags specifying which model variants we use
        bool                                                branchHeterogeneousClockRates;
        bool                                                branchHeterogeneousSubstitutionMatrices;
        bool                                                rateVariationAcrossSites;
        bool                                                cladogenicEvents;
        bool                                                imperfectTipData;
    };
    
}


#include "ConstantNode.h"
#include "DiscreteCharacterState.h"
#include "RateMatrix_JC.h"
#include "RandomNumberFactory.h"
#include "TopologyNode.h"
#include "TransitionProbabilityMatrix.h"

#include <cmath>
#include <cstring>

template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::BiogeographicTreeHistoryCtmc(const TypedDagNode<treeType> *t, size_t nChars, size_t nSites) : AbstractTreeHistoryCtmc<charType, treeType>(  t, nChars, nSites ) {
    
    // initialize with default parameters
    homogeneousClockRate        = new ConstantNode<double>("clockRate", new double(1.0) );
    heterogeneousClockRates     = NULL;
    homogeneousRateMatrix       = new ConstantNode<RateMatrix>("rateMatrix", new RateMatrix_JC( nChars ) );
    heterogeneousRateMatrices   = NULL;
    rootFrequencies             = NULL;
    siteRates                   = NULL;
    homogeneousRateMap          = NULL; // Define a good standard JC RateMap
    heterogeneousRateMaps       = NULL;
    distancePower               = new ConstantNode<double>("distancePower", new double(0.0));
    
    
    // flags specifying which model variants we use
    branchHeterogeneousClockRates               = false;
    branchHeterogeneousSubstitutionMatrices     = false;
    rateVariationAcrossSites                    = false;
    cladogenicEvents                            = false;
    imperfectTipData                            = false;
    
    // add the parameters to the parents list
    this->addParameter( homogeneousClockRate );
    this->addParameter( homogeneousRateMatrix );
    this->addParameter( distancePower );
    if (false)
        this->addParameter( homogeneousRateMap );
    
    // Uncomment this to draw the initial state
    // this->redrawValue();
    
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::BiogeographicTreeHistoryCtmc(const BiogeographicTreeHistoryCtmc &d) : AbstractTreeHistoryCtmc<charType, treeType>( d ) {
    // parameters are automatically copied
    // initialize with default parameters
    homogeneousClockRate        = d.homogeneousClockRate;
    heterogeneousClockRates     = d.heterogeneousClockRates;
    homogeneousRateMatrix       = d.homogeneousRateMatrix;
    heterogeneousRateMatrices   = d.heterogeneousRateMatrices;
    rootFrequencies             = d.rootFrequencies;
    siteRates                   = d.siteRates;
    distancePower               = d.distancePower;
    
    // flags specifying which model variants we use
    branchHeterogeneousClockRates               = d.branchHeterogeneousClockRates;
    branchHeterogeneousSubstitutionMatrices     = d.branchHeterogeneousSubstitutionMatrices;
    rateVariationAcrossSites                    = d.rateVariationAcrossSites;
    cladogenicEvents                            = d.cladogenicEvents;
    imperfectTipData                            = d.imperfectTipData;
    
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::~BiogeographicTreeHistoryCtmc( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
    
}


template<class charType, class treeType>
RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>* RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::clone( void ) const {
    
    return new BiogeographicTreeHistoryCtmc<charType, treeType>( *this );
}



template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeRootLikelihood( size_t root, size_t left, size_t right)
{
    this->lnProb = 0.0;
    
    for (size_t i = 0; i < this->historyLikelihoods.size(); i++)
        this->lnProb += this->historyLikelihoods[i];
    
//    // reset the likelihood
//    this->lnProb = 0.0;
//    
//    // get the root frequencies
//    const std::vector<double> &f                    = getRootFrequencies();
//    std::vector<double>::const_iterator f_end       = f.end();
//    std::vector<double>::const_iterator f_begin     = f.begin();
//    
//    // get the pointers to the partial likelihoods of the left and right subtree
//    const double* p_left   = this->partialLikelihoods + this->activeLikelihood[left]*this->activeLikelihoodOffset + left*this->nodeOffset;
//    const double* p_right  = this->partialLikelihoods + this->activeLikelihood[right]*this->activeLikelihoodOffset + right*this->nodeOffset;
//    
//    // create a vector for the per mixture likelihoods
//    // we need this vector to sum over the different mixture likelihoods
//    std::vector<double> per_mixture_Likelihoods = std::vector<double>(this->numPatterns,0.0);
//    
//    // get pointers the likelihood for both subtrees
//    const double*   p_mixture_left     = p_left;
//    const double*   p_mixture_right    = p_right;
//    // iterate over all mixture categories
//    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
//    {
//        
//        // get pointers to the likelihood for this mixture category
//        const double*   p_site_mixture_left     = p_mixture_left;
//        const double*   p_site_mixture_right    = p_mixture_right;
//        // iterate over all sites
//        for (size_t site = 0; site < this->numPatterns; ++site)
//        {
//            // temporary variable storing the likelihood
//            double tmp = 0.0;
//            // get the pointer to the stationary frequencies
//            std::vector<double>::const_iterator f_j             = f_begin;
//            // get the pointers to the likelihoods for this site and mixture category
//            const double* p_site_left_j   = p_site_mixture_left;
//            const double* p_site_right_j  = p_site_mixture_right;
//            // iterate over all starting states
//            for (; f_j != f_end; ++f_j)
//            {
//                // add the probability of starting from this state
//                tmp += *p_site_left_j * *p_site_right_j * *f_j;
//                
//                // increment pointers
//                ++p_site_left_j; ++p_site_right_j;
//            }
//            // add the likelihood for this mixture category
//            per_mixture_Likelihoods[site] += tmp;
//            
//            // increment the pointers to the next site
//            p_site_mixture_left+=this->siteOffset; p_site_mixture_right+=this->siteOffset;
//            
//        } // end-for over all sites (=patterns)
//        
//        // increment the pointers to the next mixture category
//        p_mixture_left+=this->mixtureOffset; p_mixture_right+=this->mixtureOffset;
//        
//    } // end-for over all mixtures (=rate categories)
//    
//    // sum the log-likelihoods for all sites together
//    std::vector< size_t >::const_iterator patterns = this->patternCounts.begin();
//    for (size_t site = 0; site < this->numPatterns; ++site, ++patterns)
//    {
//        this->lnProb += log( per_mixture_Likelihoods[site] ) * *patterns;
//    }
//    // normalize the log-probability
//    this->lnProb -= log( this->numSiteRates ) * this->numSites;
//    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeInternalNodeLikelihood(const TopologyNode &node, size_t nodeIndex, size_t left, size_t right)
{
    double lnL = 0.0;
    BranchHistory& bh = this->histories[nodeIndex];
    std::vector<CharacterEvent*> currState = bh.getParentCharacters();
    unsigned int n = numOn(currState);
    
    if (node.isRoot())
    {
        this->historyLikelihoods[nodeIndex] = RbConstants::Double::neginf;
    }
    else if (n == 0)
    {
        // reject extinction cfgs
        if (n == 0)
            this->historyLikelihoods[nodeIndex] = RbConstants::Double::neginf;
    }
    else
    {
        
        std::multiset<CharacterEvent*,CharacterEventCompare> history = bh.getHistory();
        std::multiset<CharacterEvent*,CharacterEventCompare>::iterator it_h;
        
        const treeType& tree = this->tau->getValue();
        double bt = tree.getBranchLength(nodeIndex);
        double br = 1.0;
        if (branchHeterogeneousClockRates)
            br = heterogeneousClockRates->getValue()[nodeIndex];
        else
            br = homogeneousClockRate->getValue();
        double bs = br * bt;
        
        const RateMap* rm;
        if (branchHeterogeneousSubstitutionMatrices)
            rm = &heterogeneousRateMaps->getValue()[nodeIndex];
        else
            rm = &homogeneousRateMap->getValue();
        
        // stepwise events
        double t = 0.0;
        double dt = 0.0;
        for (it_h = history.begin(); it_h != history.end(); it_h++)
        {
            // next event time
            double idx = (*it_h)->getIndex();
            dt = (*it_h)->getTime() - t;
            
            // rescale time
            dt *= bs;
            
            // reject extinction cfgs
            if ((*it_h)->getState() == 0)
                n--;
            else
                n++;
            
            if (n == 0)
            {
                this->historyLikelihoods[nodeIndex] = RbConstants::Double::neginf;
                break;
            }
            
//            double tr = transitionRate(currState, *it_h);
//            double sr = sumOfRates(currState);
            double tr = rm->getRate(node, currState, *it_h);
            double sr = rm->getSumOfRates(node, currState);
            
            // lnL for stepwise events for p(x->y)
            lnL += log(tr) - sr * dt;
            
            // update state
            currState[idx] = *it_h;
            t += dt;
            //std::cout << t << " " << dt << " " << tr << " " << sr << " " << lnL << "; " << bs << " = " << bt << " * " << br << "; " << dt/bs << "; " << (*it_h)->getState() << " " << numOn(currState) << "\n";
        }
        
        // lnL for final non-event
//        double sr = sumOfRates(currState);
        double sr = 2.0;
        lnL += -sr * (1.0 * bs - t);
        
        //std::cout << "lnL " << lnL << "\n";
        
        this->historyLikelihoods[nodeIndex] = lnL;
        
    }
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::computeTipLikelihood(const TopologyNode &node, size_t nodeIndex)
{
    
    // add support for ambig. characters later...
    this->historyLikelihoods[nodeIndex] = 0.0;
    
//    double* p_node = this->partialLikelihoods + this->activeLikelihood[nodeIndex]*this->activeLikelihoodOffset + nodeIndex*this->nodeOffset;
//    
//    const std::vector<bool> &gap_node = this->gapMatrix[nodeIndex];
//    const std::vector<unsigned long> &char_node = this->charMatrix[nodeIndex];
//    
//    // compute the transition probabilities
//    updateTransitionProbabilities( nodeIndex, node.getBranchLength() );
//    
//    double*   p_mixture      = p_node;
//    
//    // iterate over all mixture categories
//    for (size_t mixture = 0; mixture < this->numSiteRates; ++mixture)
//    {
//        // the transition probability matrix for this mixture category
//        const double*                       tp_begin    = this->transitionProbMatrices[mixture].theMatrix;
//        
//        // get the pointer to the likelihoods for this site and mixture category
//        double*     p_site_mixture      = p_mixture;
//        
//        // iterate over all sites
//        for (size_t site = 0; site != this->numPatterns; ++site)
//        {
//            
//            // is this site a gap?
//            if ( gap_node[site] )
//            {
//                // since this is a gap we need to assume that the actual state could have been any state
//                
//                // iterate over all initial states for the transitions
//                for (size_t c1 = 0; c1 < this->numChars; ++c1)
//                {
//                    
//                    // store the likelihood
//                    p_site_mixture[c1] = 1.0;
//                    
//                }
//            }
//            else // we have observed a character
//            {
//                
//                // get the original character
//                unsigned long org_val = char_node[site];
//                
//                // iterate over all possible initial states
//                for (size_t c1 = 0; c1 < this->numChars; ++c1)
//                {
//                    
//                    if ( this->usingAmbiguousCharacters )
//                    {
//                        // compute the likelihood that we had a transition from state c1 to the observed state org_val
//                        // note, the observed state could be ambiguous!
//                        unsigned long val = org_val;
//                        
//                        // get the pointer to the transition probabilities for the terminal states
//                        const double* d  = tp_begin+(this->numChars*c1);
//                        
//                        double tmp = 0.0;
//                        
//                        while ( val != 0 ) // there are still observed states left
//                        {
//                            // check whether we observed this state
//                            if ( (val & 1) == 1 )
//                            {
//                                // add the probability
//                                tmp += *d;
//                            }
//                            
//                            // remove this state from the observed states
//                            val >>= 1;
//                            
//                            // increment the pointer to the next transition probability
//                            ++d;
//                        } // end-while over all observed states for this character
//                        
//                        // store the likelihood
//                        p_site_mixture[c1] = tmp;
//                        
//                    }
//                    else // no ambiguous characters in use
//                    {
//                        
//                        // store the likelihood
//                        p_site_mixture[c1] = tp_begin[c1*this->numChars+org_val];
//                        
//                    }
//                    
//                } // end-for over all possible initial character for the branch
//                
//            } // end-if a gap state
//            
//            // increment the pointers to next site
//            p_site_mixture+=this->siteOffset;
//            
//        } // end-for over all sites/patterns in the sequence
//        
//        // increment the pointers to next mixture category
//        p_mixture+=this->mixtureOffset;
//        
//    } // end-for over all mixture categories
    
}



template<class charType, class treeType>
const std::vector<double>& RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::getRootFrequencies( void ) {
    
    if ( branchHeterogeneousSubstitutionMatrices || rootFrequencies != NULL )
    {
        return rootFrequencies->getValue();
    }
    else
    {
        return homogeneousRateMatrix->getValue().getStationaryFrequencies();
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::initializeValue( void )
{
    if (this->dagNode->isClamped())
    {
        std::vector<TopologyNode*> nodes = AbstractTreeHistoryCtmc<charType,treeType>::tau->getValue().getNodes();
        for (size_t i = 0; i < nodes.size(); i++)
        {
            if (nodes[i]->isTip())
            {
                DiscreteTaxonData<StandardState>& d = static_cast< DiscreteTaxonData<StandardState>& >( this->value->getTaxonData(i) );
                std::vector<CharacterEvent*> tipState;
                
                for (size_t j = 0; j < d.size(); j++)
                {
                    CharacterEvent* evt = new CharacterEvent(j, d[j].getStateIndex(), 1.0);
                    tipState.push_back( evt );
                }
                
                this->histories[i].setChildCharacters(tipState);
            }
        }
    }
}

template<class charType, class treeType>
size_t RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::numOn(const std::vector<CharacterEvent*>& s)
{
    size_t n = 0;
    for (size_t i = 0; i < s.size(); i++)
        if (s[i]->getState() == 1)
            n++;
    return n;
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::redrawValue( void )
{
    if (this->tipsInitialized == false)
        initializeValue();
    
    std::set<size_t> indexSet;
    for (size_t i = 0; i < this->numSites; i++)
        indexSet.insert(i);
    
    std::vector<TopologyNode*> nodes = AbstractTreeHistoryCtmc<charType,treeType>::tau->getValue().getNodes();
    for (size_t i = 0; i < nodes.size(); i++)
    {
        samplePathEnd(*nodes[i], indexSet);
        for (size_t j = 0; j < nodes[i]->getNumberOfChildren(); j++)
        {
            TopologyNode& child = nodes[i]->getChild(j);
            samplePathStart(child, indexSet);
        }
        
        if (nodes[i]->isRoot())
            samplePathStart(*nodes[i], indexSet);
    }
    
    // sample paths
    for (size_t i = 0; i < nodes.size(); i++)
        samplePathHistory(*nodes[i], indexSet);
    
    for (size_t i = 0; i < nodes.size(); i++)
    {
        this->histories[i].print();
    }
    
}

template<class charType, class treeType>
double RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::samplePathStart(const TopologyNode& node, const std::set<size_t>& indexSet)
{
    double lnP = 0.0;
    if (cladogenicEvents)
    {
        ; // undefined for now
    }
    else // iid inheritance
    {
        if (node.isRoot() == false)
        {
            this->histories[ node.getIndex() ].setParentCharacters( this->histories[ node.getParent().getIndex() ].getChildCharacters() );
        }
        else
        {
            // get model settings
            size_t nodeIdx = node.getIndex();
            
            const RateMatrix* rm;
            if (branchHeterogeneousSubstitutionMatrices)
                rm = &heterogeneousRateMatrices->getValue()[nodeIdx];
            else
                rm = &homogeneousRateMatrix->getValue();
            double bs = node.getAge() * 2;
            
            // compute transition probabilities
            double expPart = exp( -((*rm)[1][0] + (*rm)[0][1]) * bs);
            double p = (*rm)[1][0] / ((*rm)[1][0] + (*rm)[0][1]);
            double q = 1.0 - p;
            double tp[2][2] = { { p + q * expPart, q - q * expPart }, { p - p * expPart, q + p * expPart } };
            
            std::vector<CharacterEvent*> parentState = this->histories[nodeIdx].getParentCharacters();
            std::vector<CharacterEvent*> childState = this->histories[nodeIdx].getChildCharacters();
            for (std::set<size_t>::iterator it = indexSet.begin(); it != indexSet.end(); it++)
            {
                unsigned int decS = childState[*it]->getState();
                
                double u = GLOBAL_RNG->uniform01();
                double g0 = tp[0][decS];
                double g1 = tp[1][decS];
                
                unsigned int s = 0;
                if (u < g1 / (g0 + g1))
                    s = 1;
                
                parentState[*it] = new CharacterEvent(*it, s, 1.0);
            }
            
            // forbid extinction
            if (numOn(parentState) == 0)
                samplePathStart(node, indexSet);
            else
                this->histories[nodeIdx].setParentCharacters(parentState);

        }
    }
    
    return lnP;
}

template<class charType, class treeType>
double RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::samplePathEnd(const TopologyNode& node, const std::set<size_t>& indexSet)
{
    double lnP = 0.0;
    
    if (node.isTip())
    {
        if (imperfectTipData)
        {
            ;
        }
        else
        {
            ;
        }
    }
    else
    {
        // get model settings
        size_t nodeIdx = node.getIndex();
        size_t leftIdx = node.getChild(0).getIndex();
        size_t rightIdx = node.getChild(1).getIndex();
        
        //std::vector<CharacterEvent*> states = this->histories[nodeIdx].getChildCharacters();
        const treeType& tree = this->tau->getValue();
        double bt[3] = { tree.getBranchLength(nodeIdx), tree.getBranchLength( leftIdx ), tree.getBranchLength( rightIdx ) };
        if (node.isRoot())
            bt[0] = node.getAge() * 2;

        double br[3] = { 1.0, 1.0, 1.0 };
        if (branchHeterogeneousClockRates)
        {
            br[0] = heterogeneousClockRates->getValue()[nodeIdx];
            br[1] = heterogeneousClockRates->getValue()[leftIdx];
            br[2] = heterogeneousClockRates->getValue()[rightIdx];
        }
        else
            br[0] = br[1] = br[2] = homogeneousClockRate->getValue();
            
        const RateMatrix* rm0;
        const RateMatrix* rm1;
        const RateMatrix* rm2;
        if (branchHeterogeneousSubstitutionMatrices)
        {
            rm0 = &heterogeneousRateMatrices->getValue()[nodeIdx];
            rm1 = &heterogeneousRateMatrices->getValue()[leftIdx];
            rm2 = &heterogeneousRateMatrices->getValue()[rightIdx];
        }
        else
            rm0 = rm1 = rm2 = &homogeneousRateMatrix->getValue();
        
        double bs0 = br[0] * bt[0];
        double bs1 = br[1] * bt[1];
        double bs2 = br[2] * bt[2];
        
        // compute transition probabilities
        //double r[2] = { (*rm)[1][0], (*rm)[0][1] };
//        double expPart0 = exp( - (r[0] + r[1]) * bs0);
//        double expPart1 = exp( - (r[0] + r[1]) * bs1);
//        double expPart2 = exp( - (r[0] + r[1]) * bs2);
//        double pi0 = r[0] / (r[0] + r[1]);
//        double pi1 = 1.0 - pi0;

//        double tp0[2][2] = { { pi0 + pi1 * expPart0, pi1 - pi1 * expPart0 }, { pi0 - pi0 * expPart0, pi1 + pi0 * expPart0 } };
//        double tp1[2][2] = { { pi0 + pi1 * expPart1, pi1 - pi1 * expPart1 }, { pi0 - pi0 * expPart1, pi1 + pi0 * expPart1 } };
//        double tp2[2][2] = { { pi0 + pi1 * expPart2, pi1 - pi1 * expPart2 }, { pi0 - pi0 * expPart2, pi1 + pi0 * expPart2 } };

        
        double expPart0 = exp( -((*rm0)[1][0] + (*rm0)[0][1]) * bs0);
        double expPart1 = exp( -((*rm1)[1][0] + (*rm1)[0][1]) * bs1);
        double expPart2 = exp( -((*rm2)[1][0] + (*rm2)[0][1]) * bs2);
        double p0 = (*rm0)[1][0] / ((*rm0)[1][0] + (*rm0)[0][1]);
        double q0 = 1.0 - p0;
        double p1 = (*rm1)[1][0] / ((*rm1)[1][0] + (*rm1)[0][1]);
        double q1 = 1.0 - p1;
        double p2 = (*rm2)[1][0] / ((*rm2)[1][0] + (*rm2)[0][1]);
        double q2 = 1.0 - p2;

        double tp0[2][2] = { { p0 + q0 * expPart0, q0 - q0 * expPart0 }, { p0 - p0 * expPart0, q0 + p0 * expPart0 } };
        double tp1[2][2] = { { p1 + q1 * expPart1, q1 - q1 * expPart1 }, { p1 - p1 * expPart1, q1 + p1 * expPart1 } };
        double tp2[2][2] = { { p2 + q2 * expPart2, q2 - q2 * expPart2 }, { p2 - p2 * expPart2, q2 + p2 * expPart2 } };
        
        //    std::cout << tp0[0][0] << " " << tp0[0][1] << "\n" << tp0[1][0] << " " << tp0[1][1] << "\n";
        //    std::cout << tp1[0][0] << " " << tp1[0][1] << "\n" << tp1[1][0] << " " << tp1[1][1] << "\n";
        //    std::cout << tp2[0][0] << " " << tp2[0][1] << "\n" << tp2[1][0] << " " << tp2[1][1] << "\n";
        
        std::vector<CharacterEvent*> parentState = this->histories[nodeIdx].getParentCharacters();
        std::vector<CharacterEvent*> childState = this->histories[nodeIdx].getChildCharacters();
        const std::vector<CharacterEvent*> leftState = this->histories[ leftIdx ].getChildCharacters();
        const std::vector<CharacterEvent*> rightState = this->histories[ rightIdx ].getChildCharacters();
        for (std::set<size_t>::iterator it = indexSet.begin(); it != indexSet.end(); it++)
        {
            unsigned int ancS = parentState[*it]->getState();
            unsigned int desS1 = leftState[*it]->getState();
            unsigned int desS2 = rightState[*it]->getState();
            
            double u = GLOBAL_RNG->uniform01();
            double g0 = tp0[ancS][0] * tp1[0][desS1] * tp2[0][desS2];
            double g1 = tp0[ancS][1] * tp1[1][desS1] * tp2[1][desS2];
            
            unsigned int s = 0;
            if (u < g1 / (g0 + g1))
                s = 1;
            
            //std::cout << "\t" << *it << " : " << ancS << " -> " << s << " -> (" << desS1 << "," << desS2 << ") wp " << (s == 0 ? g0/(g0+g1) : g1/(g0+g1)) << "\n";
            
            childState[*it] = new CharacterEvent(*it, s, 1.0);
        }
    
        // forbid extinction
        if (numOn(childState) == 0)
            samplePathEnd(node, indexSet);
        else
            this->histories[nodeIdx].setChildCharacters(childState);
        
    }
    return lnP;
}

template<class charType, class treeType>
double RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::samplePathHistory(const TopologyNode& node, const std::set<size_t>& indexSet)
{

    // get model parameters
    size_t nodeIdx = node.getIndex();
    const treeType& tree = this->tau->getValue();
    double bt = tree.getBranchLength(nodeIdx);
    if (node.isRoot())
        bt = node.getAge() * 2;
    
    double br = 1.0;
    if (branchHeterogeneousClockRates)
        br = heterogeneousClockRates->getValue()[nodeIdx];
    else
        br = homogeneousClockRate->getValue();
    
    const RateMatrix* rm;
    if (branchHeterogeneousSubstitutionMatrices)
        rm = &heterogeneousRateMatrices->getValue()[nodeIdx];
    else
        rm = &homogeneousRateMatrix->getValue();
    
    double bs = br * bt;
    double r[2] = { (*rm)[1][0], (*rm)[0][1] };
    
    // begin update
    BranchHistory& h = this->histories[ node.getIndex() ];
    h.clearEvents( indexSet );
    
    // reject sample path history
    std::vector<CharacterEvent*> parentVector = h.getParentCharacters();
    std::vector<CharacterEvent*> childVector = h.getChildCharacters();
    std::multiset<CharacterEvent*,CharacterEventCompare> history;
    
    for (std::set<size_t>::iterator it = indexSet.begin(); it != indexSet.end(); it++)
    {
        size_t i = *it;
        std::set<CharacterEvent*> tmpHistory;
        
        unsigned int currState = parentVector[i]->getState();
        unsigned int endState = childVector[i]->getState();
        do
        {
            tmpHistory.clear();
            currState = parentVector[i]->getState();
            double t = 0.0;
            do
            {
                unsigned int nextState = (currState == 1 ? 0 : 1);
                t += RbStatistics::Exponential::rv( r[nextState] * bs, *GLOBAL_RNG );
                //std::cout << i << " " << t << " " << currState << " " << r[currState] * bs << "\n";
                if (t < 1.0)
                {
                    currState = nextState;
                    CharacterEvent* evt = new CharacterEvent(i , nextState, t);
                    tmpHistory.insert(evt);
                }
                else
                {
                    ;//std::cout << "------\n";
                }
            }
            while(t < 1.0);
        }
        while (currState != endState);
        
        for (std::set<CharacterEvent*>::iterator it = tmpHistory.begin(); it != tmpHistory.end(); it++)
            history.insert(*it);
    }
    
    /*
     if (historyContainsExtinction(parentVector, history) == true)
     {
     std::cout << "contains extinction\n";
     samplePath(indexSet);
     return;
     }
     */
    //else
    h.updateHistory(history,indexSet);
    
    //value->print();
    //
    //std::cout << value->getHistory().size() << "\n";
    //std::cout << bt << "\n";
    //std::cout << bt * (double)value->getHistory().size() << "\n";
    
    return 0.0;
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setClockRate(const TypedDagNode< double > *r) {
    
    // remove the old parameter first
    if ( homogeneousClockRate != NULL )
    {
        this->removeParameter( homogeneousClockRate );
        homogeneousClockRate = NULL;
    }
    else // heterogeneousClockRate != NULL
    {
        this->removeParameter( heterogeneousClockRates );
        heterogeneousClockRates = NULL;
    }
    
    // set the value
    branchHeterogeneousClockRates = false;
    homogeneousClockRate = r;
    
    // add the parameter
    this->addParameter( homogeneousClockRate );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}



template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setClockRate(const TypedDagNode< std::vector< double > > *r) {
    
    // remove the old parameter first
    if ( homogeneousClockRate != NULL )
    {
        this->removeParameter( homogeneousClockRate );
        homogeneousClockRate = NULL;
    }
    else // heterogeneousClockRate != NULL
    {
        this->removeParameter( heterogeneousClockRates );
        heterogeneousClockRates = NULL;
    }
    
    // set the value
    branchHeterogeneousClockRates = true;
    heterogeneousClockRates = r;
    
    // add the parameter
    this->addParameter( heterogeneousClockRates );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setDistancePower(const TypedDagNode< double > *dp) {
    
    // remove the old parameter first
    if ( distancePower != NULL )
    {
        this->removeParameter( distancePower );
        distancePower = NULL;
    }

    // set the value
    distancePower = dp;
    
    // add the parameter
    this->addParameter( distancePower );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setRateMatrix(const TypedDagNode< RateMatrix > *rm) {
    
    // remove the old parameter first
    if ( homogeneousRateMatrix != NULL )
    {
        this->removeParameter( homogeneousRateMatrix );
        homogeneousRateMatrix = NULL;
    }
    else // heterogeneousRateMatrix != NULL
    {
        this->removeParameter( heterogeneousRateMatrices );
        heterogeneousRateMatrices = NULL;
    }
    
    // set the value
    branchHeterogeneousSubstitutionMatrices = false;
    homogeneousRateMatrix = rm;
    
    // add the parameter
    this->addParameter( homogeneousRateMatrix );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setRateMatrix(const TypedDagNode< RbVector< RateMatrix > > *rm) {
    
    // remove the old parameter first
    if ( homogeneousRateMatrix != NULL )
    {
        this->removeParameter( homogeneousRateMatrix );
        homogeneousRateMatrix = NULL;
    }
    else // heterogeneousRateMatrix != NULL
    {
        this->removeParameter( heterogeneousRateMatrices );
        heterogeneousRateMatrices = NULL;
    }
    
    // set the value
    branchHeterogeneousSubstitutionMatrices = true;
    heterogeneousRateMatrices = rm;
    
    // add the parameter
    this->addParameter( heterogeneousRateMatrices );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setRateMap(const TypedDagNode< RateMap > *rm) {
    
    // remove the old parameter first
    if ( homogeneousRateMap != NULL )
    {
        this->removeParameter( homogeneousRateMap );
        homogeneousRateMap = NULL;
    }
    else // heterogeneousRateMatrix != NULL
    {
        this->removeParameter( heterogeneousRateMaps );
        heterogeneousRateMaps = NULL;
    }
    
    // set the value
    branchHeterogeneousSubstitutionMatrices = false;
    homogeneousRateMap = rm;
    
    // add the parameter
    this->addParameter( homogeneousRateMap );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setRateMap(const TypedDagNode< RbVector< RateMap > > *rm) {
    
    // remove the old parameter first
    if ( homogeneousRateMap != NULL )
    {
        this->removeParameter( homogeneousRateMatrix );
        homogeneousRateMap = NULL;
    }
    else // heterogeneousRateMatrix != NULL
    {
        this->removeParameter( heterogeneousRateMaps );
        heterogeneousRateMaps = NULL;
    }
    
    // set the value
    branchHeterogeneousSubstitutionMatrices = true;
    heterogeneousRateMaps = rm;
    
    // add the parameter
    this->addParameter( heterogeneousRateMaps );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
    
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setRootFrequencies(const TypedDagNode< std::vector< double > > *f) {
    
    // remove the old parameter first
    if ( rootFrequencies != NULL )
    {
        this->removeParameter( rootFrequencies );
        rootFrequencies = NULL;
    }
    
    if ( f != NULL )
    {
        // set the value
        //        branchHeterogeneousSubstitutionMatrices = true;
        rootFrequencies = f;
        
        // add the parameter
        this->addParameter( rootFrequencies );
    }
    else
    {
        branchHeterogeneousSubstitutionMatrices = false;
    }
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
}


template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::setSiteRates(const TypedDagNode< std::vector< double > > *r) {
    
    // remove the old parameter first
    if ( siteRates != NULL )
    {
        this->removeParameter( siteRates );
        siteRates = NULL;
    }
    
    if ( r != NULL )
    {
        // set the value
        rateVariationAcrossSites = true;
        siteRates = r;
        this->numSiteRates = r->getValue().size();
        this->resizeLikelihoodVectors();
    }
    else
    {
        // set the value
        rateVariationAcrossSites = false;
        siteRates = NULL;
        this->numSiteRates = 1;
        this->resizeLikelihoodVectors();
        
    }
    
    // add the parameter
    this->addParameter( siteRates );
    
    // redraw the current value
    if ( this->dagNode != NULL && !this->dagNode->isClamped() )
    {
        this->redrawValue();
    }
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::swapParameter(const DagNode *oldP, const DagNode *newP) {
    
    if (oldP == homogeneousClockRate)
    {
        homogeneousClockRate = static_cast<const TypedDagNode< double >* >( newP );
    }
    else if (oldP == heterogeneousClockRates)
    {
        heterogeneousClockRates = static_cast<const TypedDagNode< std::vector< double > >* >( newP );
    }
    else if (oldP == homogeneousRateMatrix)
    {
        homogeneousRateMatrix = static_cast<const TypedDagNode< RateMatrix >* >( newP );
    }
    else if (oldP == heterogeneousRateMatrices)
    {
        heterogeneousRateMatrices = static_cast<const TypedDagNode< RbVector< RateMatrix > >* >( newP );
    }
    else if (oldP == rootFrequencies)
    {
        rootFrequencies = static_cast<const TypedDagNode< std::vector< double > >* >( newP );
    }
    else if (oldP == siteRates)
    {
        siteRates = static_cast<const TypedDagNode< std::vector< double > >* >( newP );
    }
    else if (oldP == distancePower)
    {
        distancePower = static_cast<const TypedDagNode<double>* >( newP );
    }
    else
    {
        AbstractTreeHistoryCtmc<charType, treeType>::swapParameter(oldP,newP);
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::touchSpecialization( DagNode* affecter ) {
    
    // if the topology wasn't the culprit for the touch, then we just flag everything as dirty
    if ( affecter == heterogeneousClockRates )
    {
        const std::set<size_t> &indices = heterogeneousClockRates->getTouchedElementIndices();
        
        // maybe all of them have been touched or the flags haven't been set properly
        if ( indices.size() == 0 )
        {
            // just delegate the call
            AbstractTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
        }
        else
        {
            const std::vector<TopologyNode *> &nodes = this->tau->getValue().getNodes();
            // flag recomputation only for the nodes
            for (std::set<size_t>::iterator it = indices.begin(); it != indices.end(); ++it)
            {
                this->recursivelyFlagNodeDirty( *nodes[*it] );
            }
        }
    }
    else if ( affecter == heterogeneousRateMatrices )
    {
        
        const std::set<size_t> &indices = heterogeneousRateMatrices->getTouchedElementIndices();
        
        // maybe all of them have been touched or the flags haven't been set properly
        if ( indices.size() == 0 )
        {
            // just delegate the call
            AbstractTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
        }
        else
        {
            const std::vector<TopologyNode *> &nodes = this->tau->getValue().getNodes();
            // flag recomputation only for the nodes
            for (std::set<size_t>::iterator it = indices.begin(); it != indices.end(); ++it)
            {
                this->recursivelyFlagNodeDirty( *nodes[*it] );
            }
        }
    }
    else if ( affecter == rootFrequencies || affecter == rootFrequencies )
    {
        
        const TopologyNode &root = this->tau->getValue().getRoot();
        this->recursivelyFlagNodeDirty( root );
    }
    else
    {
        AbstractTreeHistoryCtmc<charType, treeType>::touchSpecialization( affecter );
    }
    
}

template<class charType, class treeType>
void RevBayesCore::BiogeographicTreeHistoryCtmc<charType, treeType>::updateTransitionProbabilities(size_t nodeIdx, double brlen) {
    
    // first, get the rate matrix for this branch
    const RateMatrix *rm;
    if ( this->branchHeterogeneousSubstitutionMatrices == true )
    {
        rm = &this->heterogeneousRateMatrices->getValue()[nodeIdx];
    }
    else
    {
        rm = &this->homogeneousRateMatrix->getValue();
    }
    
    // second, get the clock rate for the branch
    double branchTime;
    if ( this->branchHeterogeneousClockRates == true )
    {
        branchTime = this->heterogeneousClockRates->getValue()[nodeIdx] * brlen;
    }
    else
    {
        branchTime = this->homogeneousClockRate->getValue() * brlen;
    }
    
    // and finally compute the per site rate transition probability matrix
    if ( this->rateVariationAcrossSites == true )
    {
        const std::vector<double> &r = this->siteRates->getValue();
        for (size_t i = 0; i < this->numSiteRates; ++i)
        {
            rm->calculateTransitionProbabilities( branchTime * r[i], this->transitionProbMatrices[i] );
        }
    }
    else
    {
        rm->calculateTransitionProbabilities( branchTime, this->transitionProbMatrices[0] );
    }
    
}

#endif /* defined(__rb_mlandis__BiogeographicTreeHistoryCtmc__) */
