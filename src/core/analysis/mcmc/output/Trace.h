#ifndef Trace_H
#define Trace_H

#include "RbVector.h"
#include "Simplex.h"
#include "Tree.h"

#include <string>
#include <vector>

namespace RevBayesCore {

    class AbstractTrace : public Cloneable {

    public:

        virtual                         ~AbstractTrace(void) {}

        virtual void                    addValueFromString(const std::string &s) = 0;
        virtual bool                    isCoveredInInterval(const std::string &v, double i, bool verbose) = 0;

    };
    
    template <class valueType>
    class Trace : public AbstractTrace {
        
    public:
        
        Trace(void);
        
        virtual                         ~Trace(void) {}

        bool                            operator==(const Trace &t) const                { return this == &t; }
        bool                            operator!=(const Trace &t) const                { return !this->operator==( t ); }
        bool                            operator<(const Trace &t) const                 { return this < &t; }

        // pure virtual methods
        virtual void                    addValueFromString(const std::string &s);
        virtual Trace*                  clone(void) const;                              //!< Clone object

        const valueType&                objectAt(size_t index, bool post = false) const { return post ? values.at(index + burnin) : values.at(index); }
        long                            size(bool post = false) const                   { return post ? values.size() - burnin : values.size(); }

        virtual void                    addObject(valueType d);
        virtual void                    addObject(valueType* d);
        virtual bool                    isCoveredInInterval(const std::string &v, double i, bool verbose);
        bool                            isDirty(void) const                             { return dirty; };
        void                            removeLastObject();
        void                            removeObjectAtIndex(int index);
        
        // getters and setters
        size_t                          getBurnin() const                               { return burnin; }
        const std::vector<valueType>&   getValues() const                               { return values; }

        virtual void                    setBurnin(long b);
        void                            setValues(std::vector<valueType> v)             { values = v; }
        

        // getters and setters
        std::string                     getFileName() const                             { return fileName; }
        const std::string&              getParameterName() const                        { return parmName; }
        
        void                            setFileName(std::string fn)                     { fileName = fn; }
        void                            setParameterName(std::string pm)                { parmName = pm; }
        
    protected:
        
        size_t                          burnin;
        std::string                     fileName;
        std::string                     parmName;
        std::vector<valueType>          values;                                     //!< the values of this trace

        mutable bool                    dirty;

    };


    // Global functions using the class
    template <class valueType>
    std::ostream&                       operator<<(std::ostream& o, const Trace<valueType>& x) {
        o << "Trace(";
        o << x.getParameterName();
        o << ")";

        return o;
    }                                //!< Overloaded output operator
    

    /**
     * Typedefs
     */
    typedef Trace<RevBayesCore::Simplex> TraceSimplex;
    typedef Trace<RevBayesCore::RbVector<double> > TraceNumericVector;
    typedef Trace<std::string> AncestralStateTrace;
    typedef Trace<std::string> ModelTrace;


    /**
     * Template specializations
     */
    template <>
    bool Trace<double>::isCoveredInInterval(const std::string &v, double alpha, bool verbose);

    template <>
    bool Trace<RbVector<double > >::isCoveredInInterval(const std::string &v, double i, bool verbose);

    template <>
    bool Trace<Simplex>::isCoveredInInterval(const std::string &v, double i, bool verbose);

}

/**
 * Default constructor.
 */
template <class valueType>
RevBayesCore::Trace<valueType>::Trace() :
    burnin( 0 ),
    fileName( "" ),
    parmName( "" ),
    dirty( true )
{
}


template <class valueType>
void RevBayesCore::Trace<valueType>::addObject(valueType t)
{
    values.push_back(t);
    dirty = true;
}


template <class valueType>
void RevBayesCore::Trace<valueType>::addObject(valueType* t)
{
    addObject(*t);
    delete t;
}


template <class valueType>
void RevBayesCore::Trace<valueType>::addValueFromString(const std::string &s)
{

    valueType t;
    Serializer<valueType, IsDerivedFrom<valueType, Serializable>::Is >::ressurectFromString( &t, s );

    addObject( t );

}


/** Clone function */
template <class valueType>
RevBayesCore::Trace<valueType>* RevBayesCore::Trace<valueType>::clone() const
{

    return new Trace(*this);
}


template <class valueType>
bool RevBayesCore::Trace<valueType>::isCoveredInInterval(const std::string &v, double alpha, bool verbose)
{
    throw RbException("Cannot compute interval coverage for '" + parmName + "' because there are not trace objects implemented for this value type.");
}


template <class valueType>
void RevBayesCore::Trace<valueType>::removeObjectAtIndex (int index)
{
    // remove the element
    values.erase(values.begin() + index);
    dirty = true;
}


template <class valueType>
void RevBayesCore::Trace<valueType>::removeLastObject()
{
    // remove object from list
    values.pop_back();
    dirty = true;
}


template <class valueType>
void RevBayesCore::Trace<valueType>::setBurnin(long b)
{
    size_t old = burnin;
    if (b == -1)
    {
        burnin = size() / 4;
    }
    else
    {
        burnin = size_t(b);
    }

    dirty = ( dirty || (old != burnin) );
}

#endif
