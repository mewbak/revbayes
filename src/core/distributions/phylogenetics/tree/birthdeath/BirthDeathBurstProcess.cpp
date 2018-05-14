#include "BirthDeathBurstProcess.h"
#include "DistributionExponential.h"
#include "RandomNumberFactory.h"
#include "RandomNumberGenerator.h"
#include "RbConstants.h"
#include "RbMathCombinatorialFunctions.h"
#include "StochasticNode.h"

#include <cmath>


using namespace RevBayesCore;


/**
 * Constructor.
 * We delegate most parameters to the base class and initialize the members.
 *
 * \param[in]    ra             Age or the root (=time of the process).
 * \param[in]    s              Speciation rate.
 * \param[in]    e              Extinction rate.
 * \param[in]    p              Extinction sampling rate.
 * \param[in]    r              Sampling probability at present time.
 * \param[in]    cdt            Condition of the process (none/survival/#Taxa).
 * \param[in]    tn             Taxa.
 * \param[in]    c              Clades conditioned to be present.
 */
BirthDeathBurstProcess::BirthDeathBurstProcess( const TypedDagNode<double> *ra,
                                                const TypedDagNode<double> *s,
                                                const TypedDagNode<double> *e,
                                                const TypedDagNode<double> *b,
                                                const TypedDagNode<double> *bt,
                                                const TypedDagNode<double> *r,
                                                const std::string& cdt,
                                                const std::vector<Taxon> &tn) : AbstractBirthDeathProcess( ra, cdt, tn ),
    lambda( s ),
    mu( e ),
    beta( b ),
    time_burst( bt ),
    rho( r )
{
    addParameter( lambda );
    addParameter( mu );
    addParameter( beta );
    addParameter( time_burst );
    addParameter( rho );
    
    simulateTree();
    
}

/**
 * The clone function is a convenience function to create proper copies of inherited objected.
 * E.g. a.clone() will create a clone of the correct type even if 'a' is of derived type 'B'.
 *
 * \return A new copy of myself
 */
BirthDeathBurstProcess* BirthDeathBurstProcess::clone( void ) const
{
    
    return new BirthDeathBurstProcess( *this );
}


/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 */
double BirthDeathBurstProcess::computeLnProbabilityDivergenceTimes( void ) const
{
    // prepare the probability computation
    prepareProbComputation();
    
    // variable declarations and initialization
    double lnProbTimes = computeLnProbabilityTimes();
    
    return lnProbTimes;
}


/**
 * Compute the log-transformed probability of the current value under the current parameter values.
 *
 * \return    The log-probability density.
 */
double BirthDeathBurstProcess::computeLnProbabilityTimes( void ) const
{
    
    double lnProbTimes = 0.0;
    double process_time = getOriginAge();
    size_t num_initial_lineages = 2;
    
    // variable declarations and initialization
    double birth_rate = lambda->getValue();
    double burst_prob = beta->getValue();
    double burst_time = time_burst->getValue();
    double sampling_prob = rho->getValue();
    
    // get node/time variables
    size_t num_nodes = value->getNumberOfNodes();
    size_t num_lineages_burst_at_event = 0;
    size_t num_lineages_alive_at_burst = 0;
    
    // classify nodes
    int num_extant_taxa = 0;
    
    std::vector<double> internal_node_ages = std::vector<double>();
    for (size_t i = 0; i < num_nodes; i++)
    {
        const TopologyNode& n = value->getNode( i );
        double node_age = n.getAge();
        
        // store the age if this node is an internal node
        if ( n.isInternal() == true && n.isRoot() == false )
        {
            internal_node_ages.push_back( node_age );
        }
        
        // count the number of lineages alive at the burst event
        // do not count the lineages that actually bursted
        if ( n.isRoot() == false )
        {
            double parent_age = n.getParent().getAge();
            if ( fabs(node_age - burst_time) > 1E-10 &&
                 node_age < burst_time &&
                 fabs(parent_age - burst_time) > 1E-10 &&
                 parent_age > burst_time )
            {
                ++num_lineages_alive_at_burst;
            }
            
        }
        
        // count the number of lineages that bursted
        if ( lineage_bursted_at_event[i] == true )
        {
            
            if ( fabs(node_age - burst_time) < 1E-10 )
            {
                // node is sampled ancestor
                ++num_lineages_burst_at_event;
            }
            else
            {
                return RbConstants::Double::neginf;
            }
            
        }
        
        
    }
    
    // get helper variables
//    double a = birth_rate - death_rate;
//    double c1 = a;
//    double c2 = -(a - 2 * birth_rate * sampling_prob) / (birth_rate - death_rate);
    
    // add the log probability for sampling the extant taxa
    lnProbTimes += num_extant_taxa * log( 4.0 * sampling_prob );
    
    // add the log probability of the initial sequences
    lnProbTimes -= lnQ(process_time) * num_initial_lineages;
    
    // add the log probability for the internal node ages
    lnProbTimes += internal_node_ages.size() * log( birth_rate );
    for (size_t i=0; i<internal_node_ages.size(); i++)
    {
        lnProbTimes -= lnQ(internal_node_ages[i]);
    }
    
    // add the log probability for the burst event
    lnProbTimes += log(burst_prob)     * num_lineages_burst_at_event;
    lnProbTimes += log( pow(1.0-burst_prob, num_lineages_alive_at_burst) + pow(burst_prob*pZero(burst_time), num_lineages_alive_at_burst) );
    
    // condition on survival
    if ( condition == "survival")
    {
        lnProbTimes -= num_initial_lineages * log(1.0 - pZero(process_time));
    }
    // condition on nTaxa
    else if ( condition == "nTaxa" )
    {
        lnProbTimes -= lnProbNumTaxa( value->getNumberOfTips(), 0, process_time, true );
    }
    
    return lnProbTimes;
    
}


double BirthDeathBurstProcess::getBurstTime( void ) const
{
    
    return time_burst->getValue();
}


bool BirthDeathBurstProcess::isBurstSpeciation( size_t index ) const
{
    
    return lineage_bursted_at_event[index];
}


double BirthDeathBurstProcess::lnProbTreeShape(void) const
{
    // the birth death divergence times density is derived for a (ranked) unlabeled oriented tree
    // so we convert to a (ranked) labeled non-oriented tree probability by multiplying by 2^{n+m-1} / n!
    // where n is the number of extant tips, m is the number of sampled extinct tips
    
    int num_taxa = (int)value->getNumberOfTips();
    int num_extinct = (int)value->getNumberOfExtinctTips();
    int num_sa = (int)value->getNumberOfSampledAncestors();
    
    return (num_taxa - num_sa - 1) * RbConstants::LN2 - RbMath::lnFactorial(num_taxa - num_extinct);
}


/**
 * Compute the probability of survival if the process starts with one species at time start and ends at time end.
 *
 *
 * \param[in]    start      Start time of the process.
 * \param[in]    end        End/stopping time of the process.
 *
 * \return Speciation rate at time t.
 */
double BirthDeathBurstProcess::pSurvival(double start, double end) const
{
    return 1.0 - pZero(end);
}


void BirthDeathBurstProcess::setBurstSpeciation( size_t index, bool tf )
{
    
    lineage_bursted_at_event[index] = tf;
}



/**
 * Simulate new speciation times.
 */
double BirthDeathBurstProcess::simulateDivergenceTime(double origin, double present) const
{
    
    // incorrect placeholder for constant FBDP
    
    // Get the rng
    RandomNumberGenerator* rng = GLOBAL_RNG;
    
    // get the parameters
    double age = origin - present;
    double b = lambda->getValue();
    double d = mu->getValue();
    double r = rho->getValue();
    
    // get a random draw
    double u = rng->uniform01();
    
    
    // compute the time for this draw
    // see Hartmann et al. 2010 and Stadler 2011
    double t = 0.0;
    if ( b > d )
    {
        t = ( log( ( (b-d) / (1 - (u)*(1-((b-d)*exp((d-b)*age))/(r*b+(b*(1-r)-d)*exp((d-b)*age) ) ) ) - (b*(1-r)-d) ) / (r * b) ) )  /  (b-d);
    }
    else
    {
        t = ( log( ( (b-d) / (1 - (u)*(1-(b-d)/(r*b*exp((b-d)*age)+(b*(1-r)-d) ) ) ) - (b*(1-r)-d) ) / (r * b) ) )  /  (b-d);
    }
    
    return present + t;
}


double BirthDeathBurstProcess::pZero(double t) const
{
    double birth = lambda->getValue();
    double death = mu->getValue();
    double burst = beta->getValue();
    double t_b = time_burst->getValue();
    double sampling = rho->getValue();
    double E = 0;
    
    double A = birth - death;
    
    if ( t < t_b )
    {
        double B = ((1.0 - 2.0*(1.0-sampling)) * birth + death ) /  A;
        E = birth + death - A * (1.0 + B - exp(-A*t) * (1.0-B)) / (1.0+B+exp(-A*t)*(1.0-B));
        E /= (2*birth);
//        \frac{\lambda + \mu - A \frac{1+B_{t_\beta}-e^{-A(t-t_\beta)}(1-B_{t_\beta})}{1+B_{t_\beta}+e^{-A(t-t_\beta)}(1-B_{t_\beta})} }{ 2\lambda } \quad & \quad \text{otherwise}

    }
    else
    {
        double B_tmp = ((1.0 - 2.0*(1.0-sampling)) * birth + death ) /  A;
        double E_tmp = birth + death - A * (1.0 + B_tmp - exp(-A*t_b) * (1.0-B_tmp)) / (1.0+B_tmp+exp(-A*t_b)*(1.0-B_tmp));
        double B = ((1.0 - 2.0*((1.0-burst)*E_tmp+burst*E_tmp*E_tmp)) * birth + death ) /  A;
        E = birth + death - A * (1.0 + B - exp(-A*(t-t_b)) * (1.0-B)) / (1.0+B+exp(-A*(t-t_b))*(1.0-B));
        E /= (2*birth);
    }
    
    return E;
}


double BirthDeathBurstProcess::lnQ(double t) const
{
    // D(t) = \frac{ 4e^{-A(t-t_y)} }{ \big(1+B_{t_y}+e^{-A(t-t_y)}(1-B_{t_y}) \big)^2 }
    
    double birth = lambda->getValue();
    double death = mu->getValue();
    double burst = beta->getValue();
    double t_b = time_burst->getValue();
    double sampling = rho->getValue();
    
    double A = birth - death;
    double B = 0.0;
    if ( t < t_b )
    {
        B = ((1.0 - 2.0*(1.0-sampling)) * birth + death ) /  A;
    }
    else
    {
        double B_tmp = ((1.0 - 2.0*(1.0-sampling)) * birth + death ) /  A;
        double E_tmp = birth + death - A * (1.0 + B_tmp - exp(-A*t_b) * (1.0-B_tmp)) / (1.0+B_tmp+exp(-A*t_b)*(1.0-B_tmp));
        B = ((1.0 - 2.0*((1.0-burst)*E_tmp+burst*E_tmp*E_tmp)) * birth + death ) /  A;
    }
    
    double D = 4.0*exp(-A*t);
    double tmp = 1.0 + B + exp(-A*t)*(1.0-B);
    D /= (tmp*tmp);
    
    // numerically safe code
    return log( D );
}


/**
 * Swap the parameters held by this distribution.
 *
 *
 * \param[in]    oldP      Pointer to the old parameter.
 * \param[in]    newP      Pointer to the new parameter.
 */
void BirthDeathBurstProcess::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    if (oldP == lambda)
    {
        lambda = static_cast<const TypedDagNode<double>* >( newP );
    }
    
    if (oldP == mu)
    {
        mu = static_cast<const TypedDagNode<double>* >( newP );
    }
    
    if (oldP == beta)
    {
        beta = static_cast<const TypedDagNode<double>* >( newP );
    }
    
    if (oldP == time_burst)
    {
        time_burst = static_cast<const TypedDagNode<double>* >( newP );
    }
    
    if (oldP == rho)
    {
        rho = static_cast<const TypedDagNode<double>* >( newP );
    }
    
    // delegate the super-class
    AbstractBirthDeathProcess::swapParameterInternal(oldP, newP);
    
}


