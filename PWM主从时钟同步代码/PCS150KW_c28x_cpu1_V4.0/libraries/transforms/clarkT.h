//#############################################################################
//
//  FILE:   clarkT.h
//
//  TITLE:  ABC to alpha beta Transform Module
//
//#############################################################################

#ifndef CLARKT_H
#define CLARKT_H

#ifdef __cplusplus
extern "C" {
#endif

//
// Included Files
//
#include <stdint.h>
#ifndef __TMS320C28XX_CLA__
#include <math.h>
#else
#include <CLAmath.h>
#endif

//#############################################################################
//
// Macro Definitions
//
//#############################################################################
#ifndef C2000_IEEE754_TYPES
#define C2000_IEEE754_TYPES
#ifdef __TI_EABI__
typedef float         float32_t;
typedef double        float64_t;
#else // TI COFF
typedef float         float32_t;
typedef long double   float64_t;
#endif // __TI_EABI__
#endif // C2000_IEEE754_TYPES


//
// Typedefs
//

typedef struct{
    float32_t alpha;   //!< Output: Alpha component (abc-> alpha beta)
    float32_t beta;    //!< Output: Beta component (abc-> alpha beta)
    float32_t zero;    //!< Output: Zero component (abc-> alpha beta)
} ClarkT;


static inline void clarkT_reset(ClarkT *v)
{
    v->alpha = 0;
    v->beta = 0;
    v->zero = 0;
}

//! \brief             Runs clarkT routine
//! \param *v          The ClarkT structure pointer
//! \param a           Phase a value
//! \param b           Phase b value
//! \param c           Phase c value
//!
static inline void clarkT_run(ClarkT *v,float32_t a, float32_t b, float32_t c)
{
    v->alpha = (0.66666666677f) * (a - 0.5f * (b + c));
    v->beta  = (0.57735026913f) * (b - c);
    v->zero  = (0.57735026913f) * (a + b + c);
}

#ifdef __cplusplus
}
#endif // extern "C"

#endif // end

//
// End of File
//
