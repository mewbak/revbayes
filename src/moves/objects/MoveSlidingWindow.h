/*
 * MoveScale.h
 *
 *  Created on: 25 nov 2009
 *      Author: Sebastian
 */

#ifndef MoveSlidingWindow_H
#define MoveSlidingWindow_H

#include <iostream>
#include <string>
#include <vector>

#include "RbMove.h"

class RbDouble;
class RbObject;

class MoveSlidingWindow : public RbMove {
    public:

        static const StringVector   rbClass;            //!< Static class attribute

        MoveSlidingWindow(DAGNode* n, RbDouble* lower, RbDouble* upper, RbDouble* tn, RandomNumberGenerator* r);
        MoveSlidingWindow(const MoveSlidingWindow& m);
        virtual ~MoveSlidingWindow();

        // Basic utility functions
        RbObject*                  clone() const;                                  //!< Clone object
        bool                       equals(const RbObject* obj) const;              //!< Equals comparison
        const StringVector&        getClass() const { return rbClass; }            //!< Get class
        void                       print(std::ostream& o) const;                   //!< Print complete object info
        void                       printValue(std::ostream& o) const;              //!< Print value (for user)
        std::string                toString(void) const;                           //!< General info on object
        RbMove&                    operator=(const RbMove& o);
        RbObject&                  operator=(const RbObject& o);
        MoveSlidingWindow&         operator=(const MoveSlidingWindow& o);



    protected:
        double         perform(void);
        void           accept(void);                // for statistic purposes
        void           reject(void);
        
        RbDouble*				lower;
        RbDouble*				upper;
        RbDouble*				tuningParm;
};

#endif /* MOVESCALE_H_ */
