/**CFile****************************************************************

  FileName    [abcSymm.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Network and node package.]

  Synopsis    [Computation of two-variable symmetries.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: abcSymm.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "base/abc/abc.h"
#include "opt/sim/sim.h"
#include "opt/dau/dau.h"
#include "misc/util/utilTruth.h"

#ifdef ABC_USE_CUDD
#include "bdd/extrab/extraBdd.h"
#endif

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////
 
#ifdef ABC_USE_CUDD

static void Abc_NtkSymmetriesUsingBdds( Abc_Ntk_t * pNtk, int fNaive, int fReorder, int fVerbose );
static void Abc_NtkSymmetriesUsingSandS( Abc_Ntk_t * pNtk, int fVerbose );
static void Ntk_NetworkSymmsBdd( DdManager * dd, Abc_Ntk_t * pNtk, int fNaive, int fVerbose );
static void Ntk_NetworkSymmsPrint( Abc_Ntk_t * pNtk, Extra_SymmInfo_t * pSymms );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [The top level procedure to compute symmetries.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_NtkSymmetries( Abc_Ntk_t * pNtk, int fUseBdds, int fNaive, int fReorder, int fVerbose )
{
    if ( fUseBdds || fNaive )
        Abc_NtkSymmetriesUsingBdds( pNtk, fNaive, fReorder, fVerbose );
    else
        Abc_NtkSymmetriesUsingSandS( pNtk, fVerbose );
}

/**Function*************************************************************

  Synopsis    [Symmetry computation using simulation and SAT.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_NtkSymmetriesUsingSandS( Abc_Ntk_t * pNtk, int fVerbose )
{
//    extern int Sim_ComputeTwoVarSymms( Abc_Ntk_t * pNtk, int fVerbose );
    int nSymms = Sim_ComputeTwoVarSymms( pNtk, fVerbose );
    printf( "The total number of symmetries is %d.\n", nSymms );
}

/**Function*************************************************************

  Synopsis    [Symmetry computation using BDDs (both naive and smart).]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_NtkSymmetriesUsingBdds( Abc_Ntk_t * pNtk, int fNaive, int fReorder, int fVerbose )
{
    DdManager * dd;
    abctime clk, clkBdd, clkSym;
    int fGarbCollect = 1;

    // compute the global functions
clk = Abc_Clock();
    dd = (DdManager *)Abc_NtkBuildGlobalBdds( pNtk, 10000000, 1, fReorder, 0, fVerbose );
    printf( "Shared BDD size = %d nodes.\n", Abc_NtkSizeOfGlobalBdds(pNtk) ); 
    Cudd_AutodynDisable( dd );
    if ( !fGarbCollect )
        Cudd_DisableGarbageCollection( dd );
    Cudd_zddVarsFromBddVars( dd, 2 );
clkBdd = Abc_Clock() - clk;
    // create the collapsed network
clk = Abc_Clock();
    Ntk_NetworkSymmsBdd( dd, pNtk, fNaive, fVerbose );
clkSym = Abc_Clock() - clk;
    // undo the global functions
    Abc_NtkFreeGlobalBdds( pNtk, 1 );
printf( "Statistics of BDD-based symmetry detection:\n" );
printf( "Algorithm = %s. Reordering = %s. Garbage collection = %s.\n", 
       fNaive? "naive" : "fast", fReorder? "yes" : "no", fGarbCollect? "yes" : "no" );
ABC_PRT( "Constructing BDDs", clkBdd );
ABC_PRT( "Computing symms  ", clkSym );
ABC_PRT( "TOTAL            ", clkBdd + clkSym );
}

/**Function*************************************************************

  Synopsis    [Symmetry computation using BDDs (both naive and smart).]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ntk_NetworkSymmsBdd( DdManager * dd, Abc_Ntk_t * pNtk, int fNaive, int fVerbose )
{
    Extra_SymmInfo_t * pSymms;
    Abc_Obj_t * pNode;
    DdNode * bFunc;
    int nSymms = 0;
    int nSupps = 0;
    int i;

    // compute symmetry info for each PO
    Abc_NtkForEachCo( pNtk, pNode, i )
    {
//      bFunc = pNtk->vFuncsGlob->pArray[i];
        bFunc = (DdNode *)Abc_ObjGlobalBdd( pNode );
        nSupps += Cudd_SupportSize( dd, bFunc );
        if ( Cudd_IsConstant(bFunc) )
            continue;
        if ( fNaive )
            pSymms = Extra_SymmPairsComputeNaive( dd, bFunc );
        else
            pSymms = Extra_SymmPairsCompute( dd, bFunc );
        nSymms += pSymms->nSymms;
        if ( fVerbose )
        {
            printf( "Output %6s (%d): ", Abc_ObjName(pNode), pSymms->nSymms );
            Ntk_NetworkSymmsPrint( pNtk, pSymms );
        }
//Extra_SymmPairsPrint( pSymms );
        Extra_SymmPairsDissolve( pSymms );
    }
    printf( "Total number of vars in functional supports = %8d.\n", nSupps );
    printf( "Total number of two-variable symmetries     = %8d.\n", nSymms );
}

/**Function*************************************************************

  Synopsis    [Printing symmetry groups from the symmetry data structure.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ntk_NetworkSymmsPrint( Abc_Ntk_t * pNtk, Extra_SymmInfo_t * pSymms )
{
    char ** pInputNames;
    int * pVarTaken;
    int i, k, nVars, nSize, fStart;

    // get variable names
    nVars = Abc_NtkCiNum(pNtk);
    pInputNames = Abc_NtkCollectCioNames( pNtk, 0 );

    // alloc the array of marks
    pVarTaken = ABC_ALLOC( int, nVars );
    memset( pVarTaken, 0, sizeof(int) * nVars );

    // print the groups
    fStart = 1;
    nSize = pSymms->nVars;
    for ( i = 0; i < nSize; i++ )
    {
        // skip the variable already considered
        if ( pVarTaken[i] )
            continue;
        // find all the vars symmetric with this one
        for ( k = 0; k < nSize; k++ )
        {
            if ( k == i )
                continue;
            if ( pSymms->pSymms[i][k] == 0 )
                continue;
            // vars i and k are symmetric
            assert( pVarTaken[k] == 0 );
            // there is a new symmetry pair 
            if ( fStart == 1 )
            {  // start a new symmetry class
                fStart = 0;
                printf( "  { %s", pInputNames[ pSymms->pVars[i] ] );
                // mark the var as taken
                pVarTaken[i] = 1;
            }
            printf( " %s", pInputNames[ pSymms->pVars[k] ] );
            // mark the var as taken
            pVarTaken[k] = 1;
        }
        if ( fStart == 0 )
        {
            printf( " }" );
            fStart = 1; 
        }   
    }   
    printf( "\n" );

    ABC_FREE( pInputNames );
    ABC_FREE( pVarTaken );
}

#else

void Abc_NtkSymmetries( Abc_Ntk_t * pNtk, int fUseBdds, int fNaive, int fReorder, int fVerbose ) {}

#endif

/**Function*************************************************************

  Synopsis    [Try different permutations.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ntk_SymTryRandomFlips( word * pFun, word * pNpn, int nVars )
{
    int Rand[16] = { 17290, 20203, 19027, 12035, 14687, 10920, 10413, 261, 2072, 16899, 4480, 6192, 3978, 8343, 745, 1370 };
    int i, nWords = Abc_TtWordNum(nVars);
    word * pFunT = ABC_CALLOC( word, nWords );
    Abc_TtCopy( pFunT, pFun, nWords, 0 );
    for ( i = 0; i < 16; i++ )
        Abc_TtFlip( pFunT, nWords, Rand[i] % (nVars-1) );
    assert( Abc_TtCompareRev(pNpn, pFunT, nWords) != 1 );
    ABC_FREE( pFunT );
}

/**Function*************************************************************

  Synopsis    [Find canonical form of symmetric function.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ntk_SymFunDeriveNpn( word * pFun, int nVars, int * pComp )
{
    int i, nWords = Abc_TtWordNum(nVars);
    word * pFunB = ABC_CALLOC( word, nWords );
    Abc_TtCopy( pFunB, pFun, nWords, 1 );
    if ( Abc_TtCompareRev(pFunB, pFun, nWords) == 1 )
        Abc_TtCopy( pFunB, pFun, nWords, 0 );
    for ( i = 0; i < (1 << nVars); i++ )
    {
        Abc_TtFlip( pFun, nWords, pComp[i] );
        if ( Abc_TtCompareRev(pFunB, pFun, nWords) == 1 )
            Abc_TtCopy( pFunB, pFun, nWords, 0 );
        Abc_TtNot( pFun, nWords );
        if ( Abc_TtCompareRev(pFunB, pFun, nWords) == 1 )
            Abc_TtCopy( pFunB, pFun, nWords, 0 );
    }
    //Ntk_SymTryRandomFlips( pFun, pFunB, nVars );
    Abc_TtCopy( pFun, pFunB, nWords, 0 );
    ABC_FREE( pFunB );
}

/**Function*************************************************************

  Synopsis    [Generating NPN classes of all symmetric function of N variables.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Ntk_SymFunGenerate( int nVars )
{
    int k, m, Class, fVerbose = 0;
    int * pComp = Extra_GreyCodeSchedule( nVars );
    Vec_Mem_t * vTtMem = Vec_MemAlloc( Abc_Truth6WordNum(nVars), 12 );
    Vec_MemHashAlloc( vTtMem, 10000 );
    assert( !(nVars < 1 || nVars > 16) );
    printf( "Generating truth tables of all symmetric functions of %d variables.\n", nVars );
    for ( m = 0; m < (1 << (nVars+1)); m++ )
    {
        word * pFun;
        char Ones[100] = {0};
        for ( k = 0; k <= nVars; k++ )
            Ones[k] = '0' + ((m >> k) & 1);
        if ( fVerbose )
            printf( "%s : ", Ones );
        pFun = Abc_TtSymFunGenerate( Ones, nVars );
        if ( nVars < 6 )
            pFun[0] = Abc_Tt6Stretch( pFun[0], nVars );
        if ( fVerbose )
            Extra_PrintHex( stdout, (unsigned *)pFun, nVars );
        Ntk_SymFunDeriveNpn( pFun, nVars, pComp );
        Class = Vec_MemHashInsert( vTtMem, pFun );
        if ( fVerbose )
        {
            printf( " : NPN " );
            Extra_PrintHex( stdout, (unsigned *)pFun, nVars );
            printf( "  Class %3d", Class );
            printf( "\n" );
        }
        ABC_FREE( pFun );
    }
    printf( "The number of different NPN classes is %d.\n", Vec_MemEntryNum(vTtMem) );
    Vec_MemHashFree( vTtMem );
    Vec_MemFreeP( &vTtMem );
    ABC_FREE( pComp );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

