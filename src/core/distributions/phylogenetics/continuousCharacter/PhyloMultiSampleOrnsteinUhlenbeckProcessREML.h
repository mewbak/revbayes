#ifndef PhyloMultiSampleOrnsteinUhlenbeckProcessREML_H
#define PhyloMultiSampleOrnsteinUhlenbeckProcessREML_H

#include "AbstractPhyloBrownianProcess.h"
#include "TreeChangeEventListener.h"

namespace RevBayesCore {
    
    /**
     * @brief Homogeneous distribution of character state evolution along a tree class (PhyloCTMC).
     *
     *
     *
     * @copyright Copyright 2009-
     * @author The RevBayes Development Core Team (Sebastian Hoehna)
     * @since 2015-01-23, version 1.0
     */
    class PhyloMultiSampleOrnsteinUhlenbeckProcessREML : public AbstractPhyloContinuousCharacterProcess, public TreeChangeEventListener {
        
    public:
        // Note, we need the size of the alignment in the constructor to correctly simulate an initial state
        PhyloMultiSampleOrnsteinUhlenbeckProcessREML(const TypedDagNode<Tree> *t, const TypedDagNode< RbVector< double > > *v, const std::vector<Taxon> &ta, size_t ns );
        virtual                                                            ~PhyloMultiSampleOrnsteinUhlenbeckProcessREML(void);                                                              //!< Virtual destructor
        
        // public member functions
        // pure virtual
        virtual PhyloMultiSampleOrnsteinUhlenbeckProcessREML*               clone(void) const;                                                                      //!< Create an independent clone
        void                                                                setAlpha(const TypedDagNode< double >* a);
        void                                                                setAlpha(const TypedDagNode< RbVector< double > >* a);
        void                                                                setRootState(const TypedDagNode< double >* s);
        void                                                                setSigma(const TypedDagNode< double >* s);
        void                                                                setSigma(const TypedDagNode< RbVector< double > >* s);
        void                                                                setTheta(const TypedDagNode< double >* t);
        void                                                                setTheta(const TypedDagNode< RbVector< double > >* t);
        
        // non-virtual
        void                                                                fireTreeChangeEvent(const TopologyNode &n, const unsigned& m=0);                                             //!< The tree has changed and we want to know which part.
        double                                                              computeLnProbability(void);
        
    protected:
        
        // virtual methods that may be overwritten, but then the derived class should call this methods
        double                                                              computeMeanForSpecies(const std::string &n, size_t i);
        double                                                              getNumberOfSamplesForSpecies(const std::string &n);
        double                                                              getWithinSpeciesVariance(const std::string &n);
        virtual void                                                        keepSpecialization(DagNode* affecter);
        void                                                                recursiveComputeLnProbability( const TopologyNode &node, size_t node_index );
        void                                                                recursivelyFlagNodeDirty(const TopologyNode& n);
        void                                                                resetValue( void );
        virtual void                                                        restoreSpecialization(DagNode *restorer);
        void                                                                simulateRecursively(const TopologyNode& node, std::vector< ContinuousTaxonData > &t);
        std::vector<double>                                                 simulateRootCharacters(size_t n);
        void                                                                simulateTipSamples(const std::vector< ContinuousTaxonData > &td);
        double                                                              sumRootLikelihood(void);
        virtual void                                                        touchSpecialization(DagNode *toucher, bool touchAll);
        
        // Parameter management functions.
        virtual void                                                        swapParameterInternal(const DagNode *oldP, const DagNode *newP);                         //!< Swap a parameter
        
        // the likelihoods
        std::vector<std::vector<std::vector<double> > >                     partial_likelihoods;
        std::vector<std::vector<std::vector<double> > >                     contrasts;
        std::vector<std::vector<double> >                                   contrast_uncertainty;
        std::vector<std::vector<std::vector<double> > >                     normalizing_constants;
        std::vector<size_t>                                                 active_likelihood;
        
        // convenience variables available for derived classes too
        std::vector<bool>                                                   changed_nodes;
        std::vector<bool>                                                   dirty_nodes;
        
    private:
        double                                                              computeRootState(void) const;
        double                                                              computeBranchAlpha(size_t idx) const;
        double                                                              computeBranchSigma(size_t idx) const;
        double                                                              computeBranchTheta(size_t idx) const;
        
        const TypedDagNode< double >*                                       root_state;
        const TypedDagNode< double >*                                       homogeneous_alpha;
        const TypedDagNode< double >*                                       homogeneous_sigma;
        const TypedDagNode< double >*                                       homogeneous_theta;
        const TypedDagNode< RbVector< double > >*                           heterogeneous_alpha;
        const TypedDagNode< RbVector< double > >*                           heterogeneous_sigma;
        const TypedDagNode< RbVector< double > >*                           heterogeneous_theta;
        const TypedDagNode< RbVector< double > >*                           within_species_variances;
        
        size_t                                                              num_species;
        size_t                                                              num_individuals;
        std::vector<size_t>                                                 num_individuals_per_species;
        std::vector<Taxon>                                                  taxa;
        std::vector<size_t>                                                 site_indices;

    };
    
}


#endif


