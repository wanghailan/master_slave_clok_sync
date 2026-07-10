//#############################################################################
//
//  FILE:   parkT.h
//
//  TITLE:  park Transform Module
//
//#############################################################################
#ifndef PARKT_H
#define PARKT_H

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

//
// Typedefs
//

typedef struct{
    float32_t d;       //!< Output: D axis component (alpha beta -> d,q,z)
    float32_t q;       //!< Output: Q axis component (alpha beta -> d,q,z)
    float32_t z;       //!< Output: Z axis component (alpha beta -> d,q,z)
}ParkT;

//! \brief       Resets internal data to zero
//! \param *v    The ABC_DQ0_POS structure pointer
//!
static inline void ParkT_reset(ParkT *v)
{
    v->d = 0;
    v->q = 0;
    v->z = 0;
}

//! \brief             Run ParkT routine
//! \param *v          The ParkT structure pointer
//! \param sine_val    sine value of the grid angle
//! \param cosine_val  cosine value of the grid angle
//!
static inline void ParkT_Pos_run(ParkT *v,
                                   float32_t alpha, float32_t beta, float32_t zero,
                                   float32_t sine_val, float32_t cosine_val)
{
    v->d     =  alpha * cosine_val + beta * sine_val;//no delay
    v->q     = -alpha * sine_val   + beta * cosine_val;
    v->z     =  zero;
//    v->d     =  alpha * sine_val   - beta * cosine_val;//add pi/2 delay
//    v->q     =  alpha * cosine_val + beta * sine_val;
}

static inline void ParkT_Neg_run(ParkT *v,
                                   float32_t alpha, float32_t beta, float32_t zero,
                                   float32_t sine_val, float32_t cosine_val)
{
    v->d     =  alpha * cosine_val - beta * sine_val;//no delay
    v->q     =  alpha * sine_val   + beta * cosine_val;
    v->z     =  zero;
//    v->d     =  alpha * sine_val    + beta * cosine_val;//add pi/2 delay
//    v->q     = -alpha * cosine_val  + beta * sine_val;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif // extern "C"

#endif

//
// End of File
//
