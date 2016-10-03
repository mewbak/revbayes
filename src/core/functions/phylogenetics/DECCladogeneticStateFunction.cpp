//
//  DECCladogeneticStateFunction.cpp
//  revbayes-proj
//
//  Created by Michael Landis on 1/19/15.
//  Copyright (c) 2015 Michael Landis. All rights reserved.
//

//#define DEBUG_DEC

#include "DECCladogeneticStateFunction.h"
#include "BiogeographicCladoEvent.h"
#include "RbException.h"

#include <math.h>

using namespace RevBayesCore;

DECCladogeneticStateFunction::DECCladogeneticStateFunction(const TypedDagNode< RbVector<double> > *ep, const TypedDagNode< RbVector<double> > *er, unsigned nc, unsigned ns, bool epawa, bool wa, bool os):
    TypedFunction<MatrixReal>( new MatrixReal( pow(ns,nc), pow(ns,nc*2), 0.0) ),
    eventProbs( ep ),
    eventRates( er ),
    numCharacters(nc),
    num_states(2),
    numIntStates(pow(num_states,nc)),
    numEventTypes( (unsigned)ep->getValue().size() + 1 ),
    eventProbsAsWeightedAverages(epawa),
    wideAllopatry(wa),
    orderStatesByNum(os)
{
    // add the lambda parameter as a parent
    addParameter( eventProbs );
    addParameter( eventRates );
    
    buildBits();
    buildEventMap();
    
    update();
}

DECCladogeneticStateFunction::~DECCladogeneticStateFunction( void ) {
    // We don't delete the parameters, because they might be used somewhere else too. The model needs to do that!
}

std::vector<unsigned> DECCladogeneticStateFunction::bitAllopatryComplement( const std::vector<unsigned>& mask, const std::vector<unsigned>& base )
{
    std::vector<unsigned> ret = mask;
    for (size_t i = 0; i < base.size(); i++)
    {
        if (base[i] == 1)
            ret[i] = 0;
    }
    return ret;
}

void DECCladogeneticStateFunction::bitCombinations(std::vector<std::vector<unsigned> >& comb, std::vector<unsigned> array, int i, std::vector<unsigned> accum)
{
    if (i == array.size()) // end recursion
    {
        unsigned n = sumBits(accum);

        if ( n == 0 || n == sumBits(array) )
            ;  // ignore all-0, all-1 vectors
        else
            comb.push_back(accum);
    }
    else
    {
        unsigned b = array[i];
        std::vector<unsigned> tmp0(accum);
        std::vector<unsigned> tmp1(accum);
        tmp0.push_back(0);
        bitCombinations(comb,array,i+1,tmp0);
        if (b == 1)
        {
            tmp1.push_back(1);
            bitCombinations(comb,array,i+1,tmp1);
        }
    }
}

unsigned DECCladogeneticStateFunction::sumBits(const std::vector<unsigned>& b)
{
    unsigned n = 0;
    for (int i = 0; i < b.size(); i++)
        n += b[i];
    return n;
}

unsigned DECCladogeneticStateFunction::bitsToState( const std::vector<unsigned>& b )
{
    return bitsToStatesByNumOn[b];
}

std::string DECCladogeneticStateFunction::bitsToString( const std::vector<unsigned>& b )
{
    std::stringstream ss;
    for (size_t i = 0; i < b.size(); i++)
    {
        ss << b[i];
    }
    return ss.str();
}

void DECCladogeneticStateFunction::buildBits( void )
{
//    bits.resize(numIntStates);
//    for (size_t i = 0; i < numIntStates; i++) {
//        std::vector<unsigned> b(numCharacters, 0);
//        size_t v = i;
////        for (int j = numCharacters - 1; j >= 0; j--)
//        for (int j = 0; j < numCharacters; j++)
//        {
//            b[j] = v % num_states;
//            v /= num_states;
//            if (v == 0)
//                break;
//        }
//        bits[i] = b;
//    }
//    
    
    
    bitsByNumOn.resize(numCharacters+1);
    statesToBitsByNumOn.resize(numIntStates);
    bits = std::vector<std::vector<unsigned> >(numIntStates, std::vector<unsigned>(numCharacters, 0));
    bitsByNumOn[0].push_back(bits[0]);
    for (size_t i = 1; i < numIntStates; i++)
    {
        size_t m = i;
        for (size_t j = 0; j < numCharacters; j++)
        {
            bits[i][j] = m % 2;
            m /= 2;
            if (m == 0)
                break;
        }
        size_t j = sumBits(bits[i]);
        bitsByNumOn[j].push_back(bits[i]);
        
    }
    for (size_t i = 0; i < numIntStates; i++)
    {
        inverseBits[ bits[i] ] = (unsigned)i;
    }
    
    // assign state to each bit vector, sorted by numOn
    size_t k = 0;
    for (size_t i = 0; i < bitsByNumOn.size(); i++)
    {
        for (size_t j = 0; j < bitsByNumOn[i].size(); j++)
        {
            statesToBitsByNumOn[k++] = bitsByNumOn[i][j];
        }
    }
    
    for (size_t i = 0; i < statesToBitsByNumOn.size(); i++)
    {
        bitsToStatesByNumOn[ statesToBitsByNumOn[i] ] = (unsigned)i;
    }
    

}

void DECCladogeneticStateFunction::buildEventMap( void ) {
    
    eventMapCounts.resize(numIntStates, std::vector<unsigned>(numEventTypes, 0));
    
    
    if (orderStatesByNum==false) {
        statesToBitsByNumOn = bits;
        bitsToStatesByNumOn = inverseBits;
    }
    
    // get L,R states per A state
    std::vector<unsigned> idx(3);
    for (unsigned i = 0; i < numIntStates; i++) {
        
        idx[0] = i;
   
#ifdef DEBUG_DEC
        std::cout << "State " << i << "\n";
        std::cout << "Bits  " << bitsToString(statesToBitsByNumOn[i]) << "\n";
#endif
        
        // get on bits for A
        const std::vector<unsigned>& ba = statesToBitsByNumOn[i];
        std::vector<unsigned> on;
        for (unsigned j = 0; j < ba.size(); j++)
        {
            if (ba[j] == 1)
                on.push_back(j);
        }
        
        std::vector<unsigned> bl(numCharacters, 0);
        std::vector<unsigned> br(numCharacters, 0);
        
        // narrow sympatry
        if (sumBits(ba) == 1)
        {
      
#ifdef DEBUG_DEC
            
#endif
            idx[1] = i;
            idx[2] = i;
            eventMapTypes[ idx ] = BiogeographicCladoEvent::SYMPATRY_NARROW;
            eventMapCounts[ i ][  BiogeographicCladoEvent::SYMPATRY_NARROW ] += 1;
            eventMapProbs[ idx ] = 0.0;
 
#ifdef DEBUG_DEC
            std::cout << "Narrow sympatry\n";
            std::cout << "A " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
            std::cout << "L " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
            std::cout << "R " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n\n";
#endif

        }
        
        // subset/widespread sympatry
        else if (sumBits(ba) > 1)
        {
            idx[1] = i;
            idx[2] = i;
            eventMapTypes[ idx ] = BiogeographicCladoEvent::SYMPATRY_WIDESPREAD;
            eventMapCounts[ i ][  BiogeographicCladoEvent::SYMPATRY_WIDESPREAD ] += 1;
            eventMapProbs[ idx ] = 0.0;
            
#ifdef DEBUG_DEC
            std::cout << "Widespread sympatry\n";
            std::cout << "A " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
            std::cout << "L " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
            std::cout << "R " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n\n";
#endif
            
            
#ifdef DEBUG_DEC
            std::cout << "Subset sympatry (L-trunk, R-bud)\n";
#endif
            // get set of possible sympatric events for L-trunk, R-bud
            for (size_t j = 0; j < on.size(); j++)
            {
                br = std::vector<unsigned>(numCharacters, 0);
                br[ on[j] ] = 1;
//                unsigned sr = bitsToState(br);
                unsigned sr = bitsToStatesByNumOn[br];
                idx[1] = i;
                idx[2] = sr;
                eventMapTypes[ idx ] = BiogeographicCladoEvent::SYMPATRY_SUBSET;
                eventMapCounts[ i ][  BiogeographicCladoEvent::SYMPATRY_SUBSET ] += 1;
                eventMapProbs[ idx ] = 0.0;
                
#ifdef DEBUG_DEC
                std::cout << "A " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
                std::cout << "L " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n";
                std::cout << "R " << bitsToState(br) << " " << bitsToString(br) << "\n\n";
#endif
                
                br[ on[j] ] = 0;
            }
        
#ifdef DEBUG_DEC
            std::cout << "Subset sympatry (L-bud, R-trunk)\n";
#endif
            
            // get set of possible sympatric events for R-trunk, L-bud
            for (size_t j = 0; j < on.size(); j++)
            {
                bl = std::vector<unsigned>(numCharacters, 0);
    
                bl[ on[j] ] = 1;
//                unsigned sl = bitsToState(bl);
                unsigned sl = bitsToStatesByNumOn[bl];
                idx[1] = sl;
                idx[2] = i;
                eventMapTypes[ idx ] =  BiogeographicCladoEvent::SYMPATRY_SUBSET;
                eventMapCounts[ i ][  BiogeographicCladoEvent::SYMPATRY_SUBSET ] += 1;
                eventMapProbs[ idx ] = 0.0;
             
#ifdef DEBUG_DEC
                std::cout << "A " << bitsToState(statesToBitsByNumOn[i]) << " "<< bitsToString(statesToBitsByNumOn[i]) << "\n";
                std::cout << "L " << bitsToState(bl) << " "<< bitsToString(bl) << "\n";
                std::cout << "R " << bitsToState(statesToBitsByNumOn[i]) << " " << bitsToString(statesToBitsByNumOn[i]) << "\n\n";
#endif
                
                bl[ on[j] ] = 0;
    
            }
            
            // get set of possible allopatry events
            bl = ba;
            std::vector<std::vector<unsigned> > bc;
            bitCombinations(bc, ba, 0, std::vector<unsigned>());

#ifdef DEBUG_DEC
            std::cout << "Allopatry combinations\n";
            std::cout << "A " << bitsToState(ba) << " " << bitsToString(ba) << "\n";
#endif
            
            for (size_t j = 0; j < bc.size(); j++)
            {
                
                bl = bc[j];
                br = bitAllopatryComplement(ba, bl);
                
                if (sumBits(bl)==1 || sumBits(br)==1 || wideAllopatry)
                {
 
#ifdef DEBUG_DEC
                    std::cout << "L " << bitsToState(bl) << " " << bitsToString(bl) << "\n";
                    std::cout << "R " << bitsToState(br) << " " << bitsToString(br) << "\n";
#endif
                    
//                    unsigned sl = bitsToState(bl);
//                    unsigned sr = bitsToState(br);
                    unsigned sl = bitsToStatesByNumOn[bl];
                    unsigned sr = bitsToStatesByNumOn[br];
                    idx[1] = sl;
                    idx[2] = sr;
                    
                    eventMapTypes[ idx ] = BiogeographicCladoEvent::ALLOPATRY;
                    eventMapCounts[ i ][  BiogeographicCladoEvent::ALLOPATRY ] += 1;
                    eventMapProbs[ idx ] = 0.0;

#ifdef DEBUG_DEC
                    std::cout << "\n";
#endif
                }
            }
        }
#ifdef DEBUG_DEC
        std::cout << "\n\n";
#endif
    }
#ifdef DEBUG_DEC
    std::cout << "------\n";
#endif
}

DECCladogeneticStateFunction* DECCladogeneticStateFunction::clone( void ) const
{
    return new DECCladogeneticStateFunction( *this );
}

const std::map< std::vector<unsigned>, double >&  DECCladogeneticStateFunction::getEventMap(void) const
{
    return eventMapProbs;
}

void DECCladogeneticStateFunction::update( void )
{
    
    // get the information from the arguments for reading the file
    const std::vector<double>& ep = eventProbs->getValue();
//    const std::vector<double>& er = eventRates->getValue();
    
    // normalize tx probs
    std::vector<double> z( numIntStates, 0.0 );
    for (size_t i = 0; i < numIntStates; i++)
    {
        for (size_t j = 1; j < numEventTypes; j++)
        {
//            std::cout << i << " " << j << " " << eventMapCounts[i][j] << " " << ep[j-1] << "\n";
            z[i] += eventMapCounts[i][j] * ep[j-1];
        }
//        std::cout << "weight-sum[" << bitsToString(bits[i]) << "] = " << z[i] << "\n";
    }
    
    std::map<std::vector<unsigned>, unsigned>::iterator it;
    for (it = eventMapTypes.begin(); it != eventMapTypes.end(); it++)
    {
        const std::vector<unsigned>& idx = it->first;
        double v = 1.0;
        if (it->second != BiogeographicCladoEvent::SYMPATRY_NARROW) {
            if (eventProbsAsWeightedAverages) {
                v = ep[ it->second - 1 ] / z[ idx[0] ];
//                std::cout << bitsToString(bits[idx[0]]) << "->" << bitsToString(bits[idx[1]]) << "," << bitsToString(bits[idx[2]]) << "\t" << v << " = " << ep[it->second - 1] << " / " << z[ idx[0] ] << "\n";
            }
            else {
                v = ep[ it->second - 1 ] / eventMapCounts[ idx[0] ][ it->second ];
//                            std::cout << idx[0] << "->" << idx[1] << "," << idx[2] << "\t" << ep[it->second - 1] << " " << eventMapCounts[ idx[0] ][ it->second ] << "\n";
                
            }
            
        }

        (*value)[ idx[0] ][ numIntStates * idx[1] + idx[2] ] = v;
        eventMapProbs[ idx ] = v;
        
    }
    
    return;
}



void DECCladogeneticStateFunction::swapParameterInternal(const DagNode *oldP, const DagNode *newP)
{
    
    if (oldP == eventProbs)
    {
        eventProbs = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    else if (oldP == eventRates)
    {
        eventRates = static_cast<const TypedDagNode< RbVector<double> >* >( newP );
    }
    
}


