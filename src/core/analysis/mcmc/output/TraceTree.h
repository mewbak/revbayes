#ifndef TraceTree_H
#define TraceTree_H

#include "Trace.h"
#include "Tree.h"

namespace RevBayesCore {

    struct AnnotationReport
    {

        AnnotationReport() :
            ages(true),
            cc_ages(false),
            ccp(true),
            tree_ages(false),
            hpd(0.95),
            map_parameters(false),
            mean(true),
            posterior(true),
            sa(true) {}

        bool ages;
        bool cc_ages;
        bool ccp;
        bool tree_ages;
        double hpd;
        bool map_parameters;
        bool mean;
        bool posterior;
        bool sa;
    };

    class TraceTree : public Trace<Tree> {

        /*
         * This struct represents a value/count pair that is sorted by count
         */
        template <class T>
        struct Sample : public std::pair<T, long>
        {
            Sample(T t, long l) : std::pair<T, long>(t,l) {}

            inline bool operator<(const Sample<T>& rhs) const
            {
                if (this->second == rhs.second)
                    return this->first < rhs.first;
                else
                    return this->second < rhs.second;
            }
        };

        /*
         * This struct represents a tree bipartition (split) that can be rooted or unrooted
         */
        struct Split : public std::pair<RbBitSet, std::set<Taxon> >
        {
            Split( RbBitSet b, std::set<Taxon> m, bool r) : std::pair<RbBitSet, std::set<Taxon> >( !r && b[0] ? ~b : b, m) {}

            inline bool operator()(const Sample<Split>& s)
            {
                return (*this) == s.first;
            }
        };

    public:
        
        TraceTree( bool c = true );
        virtual ~TraceTree(){}
        
        TraceTree*                                 clone(void) const;
        void                                       annotateTree(Tree &inputTree, AnnotationReport report, bool verbose );
        double                                     cladeProbability(const Clade &c, bool verbose);
        double                                     computeEntropy( double credible_interval_size, int num_taxa, bool verbose );
        std::vector<double>                        computePairwiseRFDistance( double credible_interval_size, bool verbose );
        std::vector<double>                        computeTreeLengths(void);
        int                                        getBurnin(void) const;
        std::vector<Tree>                          getUniqueTrees(double ci=0.95, bool verbose=true);
        int                                        getTopologyFrequency(const Tree &t, bool verbose);
        bool                                       isClock(void);
        bool                                       isCoveredInInterval(const std::string &v, double size, bool verbose);
        Tree*                                      mapTree(AnnotationReport report, bool verbose);
        Tree*                                      mccTree(AnnotationReport report, bool verbose);
        Tree*                                      mrTree(AnnotationReport report, double cutoff, bool verbose);
        void                                       printTreeSummary(std::ostream& o, double ci=0.95, bool verbose=true);
        void                                       printCladeSummary(std::ostream& o, double minP=0.05, bool verbose=true);

    private:

        Split                                      collectTreeSample(const TopologyNode&, RbBitSet&, std::string, std::map<Split, long>&);
        void                                       enforceNonnegativeBranchLengths(TopologyNode& tree) const;
        long                                       splitFrequency(const Split &n) const;
        TopologyNode*                              findParentNode(TopologyNode&, const Split &, std::vector<TopologyNode*>&, RbBitSet& ) const;
        void                                       mapContinuous(Tree &inputTree, const std::string &n, size_t paramIndex, double hpd = 0.95, bool np=true ) const;
        void                                       mapDiscrete(Tree &inputTree, const std::string &n, size_t paramIndex, size_t num = 3, bool np=true ) const;
        void                                       mapParameters(Tree &inputTree) const;
        void                                       summarize(bool verbose);

        bool                                       clock;
        bool                                       rooted;

        std::set<Sample<Split> >                   clade_samples;
        std::map<Taxon, long >                     sampled_ancestor_counts;
        std::set<Sample<std::string> >             tree_samples;

        std::map<Split, std::vector<double> >                           clade_ages;
        std::map<Split, std::map<Split, std::vector<double> > >         conditional_clade_ages;
        std::map<std::string, std::map<Split, std::vector<double> > >   tree_clade_ages;
    };


} //end namespace RevBayesCore


#endif
