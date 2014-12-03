/**
 * @file
 * This file contains the declaration of the RevLanguage mk1 function, which
 * is used to created a deterministic variable associated with the mk1 function.
 *
 * @brief Declaration and implementation of Chromosomes_pomo
 *
 * (c) Copyright 2014- under GPL version 3
 * @author The RevBayes Development Core Team
 * @license GPL version 3
 * @version 1.0
 *
 */


#ifndef Func_mk1_H
#define Func_mk1_H

#include "RlFunction.h"

#include <map>
#include <string>

namespace RevLanguage {
    
    class Func_mk1 :  public Function {
        
    public:
        Func_mk1( void );
        
        // Basic utility functions
        Func_mk1*                                       clone(void) const;                                                              //!< Clone the object
        static const std::string&                       getClassType(void);                                                             //!< Get Rev type
        static const TypeSpec&                          getClassTypeSpec(void);                                                         //!< Get class type spec
        const TypeSpec&                                 getTypeSpec(void) const;                                                        //!< Get the type spec of the instance
        
        // Function functions you have to override
        RevPtr<Variable>                                execute(void);                                                                  //!< Execute function
        const ArgumentRules&                            getArgumentRules(void) const;                                                   //!< Get argument rules
        const TypeSpec&                                 getReturnType(void) const;                                                      //!< Get type of return value
        
    };
    
}

#endif