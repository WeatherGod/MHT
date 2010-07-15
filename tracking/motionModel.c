/*********************************************************************
 * FILE: motionModel.c                                               *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *   23 Oct 93 -- (slh) created                                      *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Functions related to CORNER_TRACK                               *
 *   ___________________________                                     *
 *							             *
 *     findTrack:  find/create a CORNER_TRACK with the given id      *
 *     saveFalarm :  save the false alarms in the global list        *
 *     verify     :  verify that a given state & report go with      *
 *                   a CORNER_TRACK with the given id                *
 *                                                                   *
 *   Functions related to CORNER_TRACK_MHT                           *
 *   _____________________________________                           *
 *								     *
 *                                                                   *
 *   measure:                                                        *
 *                Get measurements for all CORNER_TRACKs.            *
 *                                                                   *
 *   Member functions for CONSTVEL_MDL                               *
 *                        CONSTVEL_STATE                             *
 *                        CONSTPOS_REPORT                            *
 *                                                                   *
 * ----------------------------------------------------------------- *
 *                                                                   *
 *             Copyright (c) 1993, NEC Research Institute            *
 *                       All Rights Reserved.                        *
 *                                                                   *
 *   Permission to use, copy, and modify this software and its       *
 *   documentation is hereby granted only under the following terms  *
 *   and conditions.  Both the above copyright notice and this       *
 *   permission notice must appear in all copies of the software,    *
 *   derivative works or modified versions, and any portions         *
 *   thereof, and both notices must appear in supporting             *
 *   documentation.                                                  *
 *                                                                   *
 *   Correspondence should be directed to NEC at:                    *
 *                                                                   *
 *                     Ingemar J. Cox                                *
 *                                                                   *
 *                     NEC Research Institute                        *
 *                     4 Independence Way                            *
 *                     Princeton                                     *
 *                     NJ 08540                                      *
 *                                                                   *
 *                     phone:  609 951 2722                          *
 *                     fax:  609 951 2482                            *
 *                     email:  ingemar@research.nj.nec.com (Inet)    *
 *                                                                   *
 *********************************************************************/

#include "motionModel.h"
#include "param.h"
#define NO_DEBUG1
#define NO_DEBUG2
#define NO_DEBUG3
//#define SUM_SQUAREDIFF
#define CORR_COEFF

double EPSILON = 0.00000000000001;

extern CORNERLIST *g_currentCornerList;
extern int g_isFirstScan;
extern int g_time;
extern iDLIST_OF<FALARM> *g_falarms_ptr;
extern iDLIST_OF<CORNER_TRACK> *g_cornerTracks_ptr;

using std::cout;
using std::endl;
/*------------------------------------------------------*
 * findTrack():  look for the track with given id in the
 * global cornerTrackList and return a ptr to it.  If
 * not found create one
 *------------------------------------------------------*/

CORNER_TRACK *findTrack( int id )
{
BGN
  PTR_INTO_iDLIST_OF< CORNER_TRACK > p;
  CORNER_TRACK *newTrack;

  LOOP_DLIST( p, *g_cornerTracks_ptr)
  {
    if( (*p).id == id )
      return p.get();
  }

  newTrack = new CORNER_TRACK( id, getTrackColor(id) );
  g_cornerTracks_ptr->append( newTrack );

  return newTrack;
}

/*-------------------------------------------------------------------*
 * saveFalarm(report): saves the given report in the FALARM list
 *                    *g_falarms_ptr
 *-------------------------------------------------------------------*/


void saveFalarm( CONSTPOS_REPORT *report )
{
  g_falarms_ptr->append( new FALARM( report ) );
}


/*-------------------------------------------------------------------*
 * verify(trackId, report, state) : find the CORNER_TRACK with the given 
 *                    id, create a new CORNER_TRACK_ELEMENT with the 
 *                    given report & state and append it to the
 *                    list of CORNER_TRACK_ELEMENTs of that CORNER_TRACK
 *-------------------------------------------------------------------*/

void verify( int trackId, double r_x, double r_y, double s_x, double s_y, 
                    double logLikelihood,
                    int modelType, int frame)
{
BGN
  CORNER_TRACK *track;
  
//  printf("Verifying trackId=%d r_x=%lf r_y=%lf s_x=%lf s_y=%lf frame=%d\n",
//			trackId,r_x,r_y,s_x,s_y,frame);
  track = findTrack( trackId );
  track->list.append( new CORNER_TRACK_ELEMENT( s_x,s_y,r_x,r_y,logLikelihood,modelType,g_time,frame));
}

/**-------------------------------------------------------------------
 * void CORNER_TRACK_MHT::measure()
 * Take the corners of the current frame and install
 * them as reports
 *-------------------------------------------------------------------*/

void CORNER_TRACK_MHT::measure()
{
BGN
  PTR_INTO_ptrDLIST_OF< T_HYPO > tHypoPtr;
  PTR_INTO_iDLIST_OF< CORNER > cornerPtr;

  LOOP_DLIST( cornerPtr,g_currentCornerList->list)
  {
     installReport(new CONSTPOS_REPORT(m_falarmLogLikelihood,
                   (*cornerPtr).x, (*cornerPtr).y,
                   (*cornerPtr).i1, (*cornerPtr).i2,
                   (*cornerPtr).i3, (*cornerPtr).i4,
                   (*cornerPtr).i5, (*cornerPtr).i6,
                   (*cornerPtr).i7, (*cornerPtr).i8,
                   (*cornerPtr).i9, (*cornerPtr).i10,
                   (*cornerPtr).i11, (*cornerPtr).i12,
                   (*cornerPtr).i13, (*cornerPtr).i14,
                   (*cornerPtr).i15, (*cornerPtr).i16,
                   (*cornerPtr).i17, (*cornerPtr).i18,
                   (*cornerPtr).i19, (*cornerPtr).i20,
                   (*cornerPtr).i21, (*cornerPtr).i22,
                   (*cornerPtr).i23, (*cornerPtr).i24,
                   (*cornerPtr).i25,
                   (*cornerPtr).frameNo)
                  );
   }

}

/*-------------------------------------------------------------------*
 | LOG_NORMFACTOR -- constant part of likelihood calculation
 *-------------------------------------------------------------------*/

static const double LOG_NORMFACTOR =
                    /* log( 2PI^(measureVars/2) ) = */ 1.5963597;


/*-------------------------------------------------------------------*
 | CORNER_TRACK_STATE::setup() -- compute parts of Kalman filter
 |                           calculation that are independent of
 |                           reports
 *-------------------------------------------------------------------*/

void CONSTVEL_STATE::setup( double processVariance, const MATRIX &R )
{
  BGN

  /* don't do this more than once */
  if( m_hasBeenSetup )
    return;

    m_ds = 1;

  /* compute the state transition matrix and process covariance matrix
     based on the above time step */

  double ds2 = m_ds * m_ds;
  double ds3 = ds2 * m_ds;

  static MATRIX F( 4, 4 ); F.set(     1.,  m_ds,    0.,    0.,
                                      0.,    1.,    0.,    0.,
                                      0.,    0.,    1.,  m_ds,
                                      0.,    0.,    0.,    1.    );


  static MATRIX Q( 4, 4 ); Q.set(  ds3/3, ds2/2,    0.,    0.,
                                   ds2/2,  m_ds,    0.,    0.,
                                      0.,    0., ds3/3, ds2/2,
                                      0.,    0., ds2/2,  m_ds  );
                           Q = Q * processVariance;

  static MATRIX H(2,4); H.set(1., 0., 0., 0.,
                              0., 0., 1., 0.);


  /* fill in the rest of the variables */

  MATRIX P1 = F * m_P * F.trans() + Q; // state prediction covariance

  MATRIX S = H * P1 * H.trans() + R;  // innovation covariance

  m_logLikelihoodCoef = -(LOG_NORMFACTOR + log( S.det() ) / 2);

  m_Sinv = new MATRIX( S.inv() );
//  printf("Sinv:\n"); m_Sinv->print();
  
  m_W = new MATRIX( P1 * H.trans() * *m_Sinv );

  MATRIX tmp(4,4);
  tmp =  *m_W * S * m_W->trans();

  MATRIX tmp1(4,4);
  tmp1 = P1-tmp;

  m_nextP = new MATRIX( tmp1 );
  m_x1 = new MATRIX( F * m_x );

  m_hasBeenSetup = 1;

#ifdef DEBUG1
  printf("\nF:\n");F.print();
  printf("\nm_P:\n");m_P.print();
  printf("\nQ=\n");Q.print();
  printf("\nState Pred Cov(P1=F*m_P*F.trans +Q):\n");P1.print();
  printf("\nInnov Cov(S=H*P1*H.trans):\n");S.print();
  printf("\nS_inv:\n");m_Sinv->print();
   printf("\nPrevious State:\n");m_x.print();
   printf("LOG_NORMFACTOR =%lf log( S.det() ) / 2)=%lf\n",LOG_NORMFACTOR, log( S.det() ) / 2);
   printf(" m_logLikelihoodCoef= %lf\n", m_logLikelihoodCoef);
#endif

}

/*--------------------------------------------*
 * CONSTVEL_MDL::getStateX(MDL_STATE *s)
 *--------------------------------------------*/

double CONSTVEL_MDL::getStateX(MDL_STATE *s)
    {
      return ((CONSTVEL_STATE*)s)->getX();
     }


/*--------------------------------------------*
 * CONSTVEL_MDL::getStateY(MDL_STATE *s)
 *--------------------------------------------*/

double CONSTVEL_MDL::getStateY(MDL_STATE *s)
    {
      return ((CONSTVEL_STATE*)s)->getY();
     }


double CONSTVEL_MDL::getEndLogLikelihood( MDL_STATE *s )
      {
        CONSTVEL_STATE *cs = (CONSTVEL_STATE*)s;
        int m = cs->m_numSkipped;
        double endProb = 1.0 - exp( -m / m_lambda_x);
        endProb += (endProb == 0.0) ? EPSILON : 0.0;
        m_endLogLikelihood = log( endProb);
        return m_endLogLikelihood;
}
double CONSTVEL_MDL::getContinueLogLikelihood( MDL_STATE *s )
      {
        CONSTVEL_STATE *cs = (CONSTVEL_STATE*)s;
        int m = cs->m_numSkipped;
        double endProb = 1.0 - exp( -m / m_lambda_x);
        endProb += (endProb == 0.0) ? EPSILON : 0.0;
        m_continueLogLikelihood = log(1.0-endProb);
        return m_continueLogLikelihood;
}



/*-------------------------------------------------------------------*
 | CONSTVEL_MDL::getSkipLogLikelihood() -- get the likelihood of
 |                                        skipping a(nother) report
 *-------------------------------------------------------------------*/

double CONSTVEL_MDL::getSkipLogLikelihood( MDL_STATE *mdlState )
{
  CONSTVEL_STATE *state = (CONSTVEL_STATE *)mdlState;

  return  m_skipLogLikelihood;

//  return (state->getNumSkipped() + 1) * m_skipLogLikelihood;
}


/*------------------------------------------------------------------*
 * MDL_STATE* CONSTVEL_MDL::getNewState(...)
 *------------------------------------------------------------------*/

MDL_STATE* CONSTVEL_MDL::getNewState( int stateNum,
                                    MDL_STATE *mdlState,
                                    MDL_REPORT *mdlReport )
    {
BGN
      CONSTVEL_STATE *state = (CONSTVEL_STATE *) mdlState;
      CONSTPOS_REPORT *report = (CONSTPOS_REPORT *) mdlReport;
      double dx,dy;

      switch(stateNum)
      {
         case 0:   // Continue constVel State 
          {
            if( state != 0 && report !=0 && state->getDX() == 0 
                           && state->getDY() == 0 )
              {
                dx = report->getX() - state->getX();
                dy = report->getY() - state->getY();

                state->setDX(dx);
                state->setDY(dy);
             }

           CONSTVEL_STATE *newState;
           newState=getNextState(state,report);
           return (MDL_STATE*) newState;
           }
          default: 
            THROW_ERR("Too many calls to CONSTVEL_MDL::getNewState()");
       }

}



/*-------------------------------------------------------------------*
 | CONSTVEL_MDL::getNextState() -- get the next state estimate, given
 |                                a previous state estimate and a
 |                                reported measurement
 *-------------------------------------------------------------------*/

CONSTVEL_STATE* CONSTVEL_MDL::getNextState( CONSTVEL_STATE *state,
                                          CONSTPOS_REPORT *report )
{
  CONSTVEL_STATE *nextState;          // new state
  static MATRIX v( 2,1 );              // innovation
  double distance;                   // mahalanobis distance
  
  static MATRIX H(2,4); H.set(1., 0., 0., 0.,
                              0., 0., 1., 0.);


  if( state == 0 )
  {
    /* starting a new track */

    double x=report->getX(); 
    double y=report->getY();
 
#ifdef DEBUG1
    printf("\nStart a new State/Contour with Cov:"); m_startP.print(2);
    printf("\nSTARTING NEW STATE WITH %lf %lf\n",x,y);
#endif

    nextState = new CONSTVEL_STATE( this,
                                   x,
                                   0.,
                                   y,
                                   0.,
                                   report->m_int[0],
                                   report->m_int[1],
                                   report->m_int[2],
                                   report->m_int[3],
                                   report->m_int[4],
                                   report->m_int[5],
                                   report->m_int[6],
                                   report->m_int[7],
                                   report->m_int[8],
                                   report->m_int[9],
                                   report->m_int[10],
                                   report->m_int[11],
                                   report->m_int[12],
                                   report->m_int[13],
                                   report->m_int[14],
                                   report->m_int[15],
                                   report->m_int[16],
                                   report->m_int[17],
                                   report->m_int[18],
                                   report->m_int[19],
                                   report->m_int[20],
                                   report->m_int[21],
                                   report->m_int[22],
                                   report->m_int[23],
                                   report->m_int[24],
                                   m_startP,
                                   m_startLogLikelihood,
                                   0 );
  }
  else if( report == 0 )
  {
    /* continuing an existing CORNER_TRACK, skipping a measurement */

    state->setup( m_processVariance, m_R );

#ifdef DEBUG1
    printf("Skipping meas(report=0); continued state= %lf %lf %lf %lf\n",
                   state->getX1(), state->getDX1(),
                   state->getY1(), state->getDY1() );
#endif

    nextState = new CONSTVEL_STATE( this,
                                   state->getX1(),
                                   state->getDX1(),
			           state->getY1(),
			           state->getDY1(),
                                   state->m_prevInt[0],
                                   state->m_prevInt[1],
                                   state->m_prevInt[2],
                                   state->m_prevInt[3],
                                   state->m_prevInt[4],
                                   state->m_prevInt[5],
                                   state->m_prevInt[6],
                                   state->m_prevInt[7],
                                   state->m_prevInt[8],
                                   state->m_prevInt[9],
                                   state->m_prevInt[10],
                                   state->m_prevInt[11],
                                   state->m_prevInt[12],
                                   state->m_prevInt[13],
                                   state->m_prevInt[14],
                                   state->m_prevInt[15],
                                   state->m_prevInt[16],
                                   state->m_prevInt[17],
                                   state->m_prevInt[18],
                                   state->m_prevInt[19],
                                   state->m_prevInt[20],
                                   state->m_prevInt[21],
                                   state->m_prevInt[22],
                                   state->m_prevInt[23],
                                   state->m_prevInt[24],
                                   state->getNextP(),
                                   0.,
                                   state->getNumSkipped() + 1 );
 }
  else
  {
    /* continuing an existing CORNER_TRACK, with a measurement */

    state->setup( m_processVariance, m_R );
    v = report->getZ() - H * state->getPrediction();
    distance = (v.trans() * state->getSinv() * v)();
#ifdef DEBUG1
    printf("\nPredicted State:\n");(state->getPrediction()).print(2);
    printf("\nValidating meas:\n");(report->getZ()).print(3);
    printf("\nInnovation:\n "); v.print(4);
    printf("\nSinv:\n");(state->getSinv()).print(5);
    printf("\nMahalinobus dist(innovTrans * s_inv * innov)=%lf maxDist=%lf\n",
                     distance,m_maxDistance);
#endif
    if( distance > m_maxDistance ) {
      nextState = 0;
    }
    else {
      int intValidated;
      double intDistance =  getCorr(state, report);
#ifdef SUM_SQUAREDIFF
      intValidated = (intDistance < m_intensityThreshold); 
#endif
#ifdef CORR_COEFF
      intValidated = (intDistance > m_intensityThreshold);
#endif

      if (!intValidated) {
#ifdef DEBUG2
       printf("\nPredicted State:\n");(state->getPrediction()).print(2);
       printf("\nValidating meas:\n");(report->getZ()).print(3);
       printf("\nInnovation:\n "); v.print(4);
       printf("\nSinv:\n");(state->getSinv()).print(5);
       printf("\nMahalinobus dist(innovTrans * s_inv * innov)=%lf maxDist=%lf\n",
                     distance,m_maxDistance);
       printf("intDist=%lf\n",intDistance);
       printf("Prev Int = %hd %hd %hd %hd %hd %hd %hd %hd\n",state->m_prevInt[0],
state->m_prevInt[1],state->m_prevInt[2],state->m_prevInt[3],state->m_prevInt[4],
state->m_prevInt[5],state->m_prevInt[6],state->m_prevInt[7]);
       printf("Current Int = %hd %hd %hd %hd %hd %hd %hd %hd\n",report->m_int[0],
report->m_int[1],report->m_int[2],report->m_int[3],report->m_int[4],
report->m_int[5],report->m_int[6],report->m_int[7]);
#endif
       nextState=0;
     }
      else {
       MATRIX new_m_x =  state->getPrediction() + state->getW() * v;

#ifdef DEBUG1
      printf("   Updated State:\n");new_m_x.print();
      printf("   State Likelihood=%lf\n",state->getLogLikelihoodCoef() -
                                      distance / 2 );


#endif


       nextState = new CONSTVEL_STATE( this,
			              new_m_x(0),
               			      new_m_x(1),
			    	      new_m_x(2),
			    	      new_m_x(3),
                                      report->m_int[0],
                                      report->m_int[1],
                                      report->m_int[2],
                                      report->m_int[3],
                                      report->m_int[4],
                                      report->m_int[5],
                                      report->m_int[6],
                                      report->m_int[7],
                                      report->m_int[8],
                                      report->m_int[9],
                                      report->m_int[10],
                                      report->m_int[11],
                                      report->m_int[12],
                                      report->m_int[13],
                                      report->m_int[14],
                                      report->m_int[15],
                                      report->m_int[16],
                                      report->m_int[17],
                                      report->m_int[18],
                                      report->m_int[19],
                                      report->m_int[20],
                                      report->m_int[21],
                                      report->m_int[22],
                                      report->m_int[23],
                                      report->m_int[24],
                            	      state->getNextP(),
                            	      state->getLogLikelihoodCoef() -
                             	      distance / 2,
                            	      0 );
     }
   }
  }
  return nextState;
}



/*-------------------------------------------------------------------*
 | CONSTVEL_MDL::beginNewStates() -- Number of new states to start.
 | Here we are limiting new track growth to only the first frame.
 | If you would like to include new track initiation at every frame
 | comment the "if(!g_is........" line and modify getNextState
 | to deal with state==0 case, or the new track state, where the
 | next/new state would be initialized with the report,startP  and 
 | start Likelihood
 *-------------------------------------------------------------------*/

int CONSTVEL_MDL::beginNewStates( MDL_STATE *mdlState,
                                MDL_REPORT *mdlReport )
    {
  
      CONSTVEL_STATE *state = (CONSTVEL_STATE *)mdlState;
      CONSTPOS_REPORT *report = (CONSTPOS_REPORT *)mdlReport;

#ifdef NO_NEW_TRACKS
      if (!g_isFirstScan && state==0) return 0;
#endif
      return 1;
    }


/*-------------------------------------------------------------------*
 | CONSTVEL_MDL::CONSTVEL_MDL() -- constructor for the CONSTVEL_MDL
 *-------------------------------------------------------------------*/

CONSTVEL_MDL::CONSTVEL_MDL( double positionMeasureVarianceX,
                 double positionMeasureVarianceY,
                 double gradientMeasureVariance,
                 double intensityVariance,
                 double processVariance,
                 double startProb,
                 double lambda_x,
                 double detectProb,
                 double stateVar,
                 double intensityThreshold,
                 double maxDistance):
      CORNER_TRACK_MDL(),
      m_startLogLikelihood( log( startProb ) ),
      m_lambda_x( lambda_x),
      m_skipLogLikelihood( log( 1. - detectProb ) ),
      m_detectLogLikelihood( log( detectProb ) ),
      m_maxDistance( maxDistance ),
      m_processVariance( processVariance ),
      m_intensityVariance( intensityVariance ),
      m_intensityThreshold( intensityThreshold ),
      m_stateVariance( stateVar ),
      m_R( 2, 2 ),
      m_startP( 4, 4 )
    {
      BGN
      printf("\nSTARTING A NEW CONSTVEL_MDL\n");

      double pVx = positionMeasureVarianceX;
      double pVy = positionMeasureVarianceY;
      double gV = gradientMeasureVariance;
      MATRIX Q (4, 4 );


      m_R.set(  pVx, 0.,
                0., pVy );
     
      Q.set(  1./3, 1./2,   0.,   0.,
              1./2,   1.,   0.,   0.,
                0.,   0., 1./3, 1./2,
                0.,   0., 1./2,   1.   );

      Q = Q * m_processVariance;
 
      m_startP.set(pVx , 0., 0., 0.,
                    0., m_stateVariance, 0., 0.,
                  0., 0., pVy, 0.,
                  0., 0., 0., m_stateVariance );
      printf("\nstartP:\n");m_startP.print();
#ifdef DEBUG1
      printf("\nstartP:\n");m_startP.print();
#endif

      type = 2;
    }


/*--------------------------------------------------------*
 * getTrackColor( int trackId )
 *--------------------------------------------------------*/

int getTrackColor( int trackId )
{
  BGN

  static unsigned char color[] =
  {
     1,  2,  3,  4,  5,  6,  8,  9, 10, 11, 12, 13, 14, 15,
    67, 72, 75, 81, 85, 90, 97, 101, 153, 156, 164,
  };

  return color[ trackId % sizeof( color ) ];
}

void CORNER_TRACK_MHT::describe(int spaces)
{
  BGN

  PTR_INTO_ptrDLIST_OF< T_HYPO > tHypoPtr;
  PTR_INTO_iDLIST_OF< GROUP > groupPtr;
  PTR_INTO_iDLIST_OF< REPORT > reportPtr;
  PTR_INTO_iDLIST_OF< T_TREE > tTreePtr;
  int k;

  Indent( spaces ); cout << "MHT "; print(); cout << endl;
  spaces += 2;

  Indent( spaces );
  cout << "lastTrackUsed = " << m_lastTrackIdUsed;
  cout << ", time = " << m_currentTime;
  cout << endl;

  Indent( spaces );
  cout << "maxDepth = " << m_maxDepth;
  cout << ", logMinRatio = " << m_logMinGHypoRatio;
  cout << ", maxGHypos = " << m_maxGHypos;
  cout << endl;

  Indent( spaces ); cout << "active tHypo's:";
  k = 0;

  LOOP_DLIST( tHypoPtr, m_activeTHypoList )
  {
    if( k++ >= 3 )
    {
      cout << endl;
      Indent( spaces ); cout << "               ";
      k = 0;
    }

    cout << " "; (*tHypoPtr).print();
  }
  cout << endl;

  Indent( spaces ); cout << "===== clusters"; cout << endl;
  LOOP_DLIST( groupPtr, m_groupList )
  {
    (*groupPtr).describe( spaces + 2 );
  }

  Indent( spaces ); cout << "===== oldReports"; cout << endl;
  LOOP_DLIST( reportPtr, m_oldReportList )
  {
    CONSTPOS_REPORT *creport=(CONSTPOS_REPORT*)(reportPtr.get());
    (*creport).describe( spaces + 2 );
  }

  Indent( spaces ); cout << "===== newReports"; cout << endl;
  LOOP_DLIST( reportPtr, m_newReportList )
  {
    CONSTPOS_REPORT *creport=(CONSTPOS_REPORT*)(reportPtr.get());
    (*creport).describe( spaces + 2 );
  }

  Indent( spaces ); cout << "===== oldTrees"; cout << endl;
  LOOP_DLIST( tTreePtr, m_tTreeList )
  {
    if( tTreePtr == m_nextNewTTree )
    {
      Indent( spaces ); cout << "===== newTrees"; cout << endl;
    }

    cout << endl;
    (**(*tTreePtr).getTree()).describeTree( spaces + 2 );
  }
}










double CONSTVEL_MDL::getCorr(CONSTVEL_STATE *state, CONSTPOS_REPORT *report)
{
#ifdef DEBUG3
  printf("State: %hd %hd %hd %hd %hd\n",state->m_prevInt[0],state->m_prevInt[1],state->m_prevInt[2],state->m_prevInt[3],state->m_prevInt[4]);
  printf("Report: %hd %hd %hd %hd %hd\n",report->m_int[0],report->m_int[1],report->m_int[2],report->m_int[3],report->m_int[4]);
#endif

  int width = 5;
  int xm,ym;
  int index;
#ifdef SUM_SQUAREDIFF
  double minDist = HUGE;
  for (int p = 1 ; p<=3 ; p++) {
     ym=p;
     for (int q = 1; q<=3 ; q++) {
       xm=q;
       double dist = 0.0;
       for (int j=-1; j<=1 ; j++) {
          int y = ym + j ;
          int y1 = 2+j;  // pattern window is centered around middle
          for (int i=-1; i<=1; i++) {
             int x=xm + i ;
             int x1 = 2 + i;
             index = width * y + x;
             int index1 = width * y1 + x1; 
             dist += (double)(state->m_prevInt[index1] - report->m_int[index]) * 
                     (double)(state->m_prevInt[index1] - report->m_int[index]);
           }
        }
        dist = dist / m_intensityVariance;
        if (dist < minDist) minDist = dist;
#ifdef DEBUG3
        printf("Corr(%d %d)=%lf  ",ym,xm,dist);
#endif 
      }
  }
    
     return minDist;
#endif

#ifdef CORR_COEFF

  double stateMean, stateSigma;
  double reportMean, reportSigma;
  
  /* Precompute the previous Mean and sigma */
  xm=ym=2;
  stateMean=stateSigma=0.0;
  int j = -1;
  for (j=-1; j<=1 ; j++) {
    int y = ym + j ;
    for (int i=-1; i<=1; i++) {
       int x=xm + i ;
       index = width * y + x;
       stateMean += (double)(state->m_prevInt[index]);
       stateSigma += (double)(state->m_prevInt[index]) *( state->m_prevInt[index]);
     }
  }
  stateMean /= 9;
  stateSigma /= 9;
  stateSigma = stateSigma -stateMean*stateMean;
  stateSigma= sqrt(stateSigma);



/* 
 *Slide the pattern window i.e the 3x3 sub window of the 5x5 window of
 * the previous state centered at the middle
 */
  double maxCorr = -HUGE;
  for (int p = 1 ; p<=3 ; p++) {
     ym=p;
     for (int q = 1; q<=3 ; q++) {
       xm=q;

       /* Find the mean & sigma of the search window */
       reportMean=reportSigma=0.0;
       for (j=-1; j<=1 ; j++) {
          int y = ym + j ;
          for (int i=-1; i<=1; i++) {
             int x=xm + i ;
             index = width * y + x;
             reportMean += (double)(report->m_int[index]);
             reportSigma += (double)(report->m_int[index])*( report->m_int[index]);
           }
        }
        reportMean /= 9.;
        reportSigma /= 9.;
        reportSigma = reportSigma -reportMean*reportMean;
        reportSigma = sqrt(reportSigma);

       /* Compute the max correlation coeff of the 3x3 window */
       double corr = 0.0;
       for (int j=-1; j<=1 ; j++) {
          int y = ym + j ;
          int y1 = 2 + j;
          for (int i=-1; i<=1; i++) {
             int x=xm + i ;
             int x1 = 2 + i;
             index = width * y + x;
             int index1 = width *y1 + x1;
             corr += (double)(state->m_prevInt[index1] - stateMean) *
                     (double)(report->m_int[index] - reportMean);
           }
        }
        corr = corr / (9.0 * reportSigma * stateSigma);
        if (corr < -1.0 || corr > 1.0) {
           fprintf(stderr, "Error in corr calculation\n");
           exit(1);
	 }
        if (corr > maxCorr) maxCorr = corr;

#ifdef DEBUG3
        printf("Corr(%d %d)=%lf  ",ym,xm,corr);
#endif 

      }
  }
    
#ifdef DEBUG3
        printf("MAXCorr=%lf  ",maxCorr);
#endif 
     return maxCorr;
#endif
}



