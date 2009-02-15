/**CFile****************************************************************

  FileName    [cec.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Combinatinoal equivalence checking.]

  Synopsis    [External declarations.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: cec.h,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/
 
#ifndef __CEC_H__
#define __CEC_H__

////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

// dynamic SAT parameters
typedef struct Cec_ParSat_t_ Cec_ParSat_t;
struct Cec_ParSat_t_
{
    int              nBTLimit;      // conflict limit at a node
    int              nSatVarMax;    // the max number of SAT variables
    int              nCallsRecycle; // calls to perform before recycling SAT solver
    int              fPolarFlip;    // flops polarity of variables
    int              fFirstStop;    // stop on the first sat output
    int              fVerbose;      // verbose stats
};

// combinational SAT sweeping parameters
typedef struct Cec_ParCsw_t_ Cec_ParCsw_t;
struct Cec_ParCsw_t_
{
    int              nWords;        // the number of simulation words
    int              nRounds;       // the number of simulation rounds
    int              nItersMax;     // the maximum number of iterations of SAT sweeping
    int              nBTLimit;      // conflict limit at a node
    int              nSatVarMax;    // the max number of SAT variables
    int              nCallsRecycle; // calls to perform before recycling SAT solver
    int              nLevelMax;     // restriction on the level nodes to be swept
    int              nDepthMax;     // the depth in terms of steps of speculative reduction
    int              fRewriting;    // enables AIG rewriting
    int              fFirstStop;    // stop on the first sat output
    int              fVeryVerbose;  // verbose stats
    int              fVerbose;      // verbose stats
};

// combinational equivalence checking parameters
typedef struct Cec_ParCec_t_ Cec_ParCec_t;
struct Cec_ParCec_t_
{
    int              nIters;        // iterations of SAT solving/sweeping
    int              nBTLimitBeg;   // starting backtrack limit
    int              nBTlimitMulti; // multiple of backtrack limit
    int              fUseSmartCnf;  // use smart CNF computation
    int              fRewriting;    // enables AIG rewriting
    int              fSatSweeping;  // enables SAT sweeping
    int              fFirstStop;    // stop on the first sat output
    int              fVerbose;      // verbose stats
};

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/*=== cecCore.c ==========================================================*/
extern void          Cec_ManSatSetDefaultParams( Cec_ParSat_t * p );
extern void          Cec_ManCswSetDefaultParams( Cec_ParCsw_t * p );
extern void          Cec_ManCecSetDefaultParams( Cec_ParCec_t * p );
extern Gia_Man_t *   Cec_ManSatSweeping( Gia_Man_t * pAig, Cec_ParCsw_t * pPars );
extern Gia_Man_t *   Cec_ManSatSolving( Gia_Man_t * pAig, Cec_ParSat_t * pPars );

#ifdef __cplusplus
}
#endif

#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

