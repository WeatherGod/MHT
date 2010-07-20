/*********************************************************************
 *                                                                   *
 * FILE: motionModel.h                                               *
 *                                                                   *
 *                                                                   *
 * HISTORY:                                                          *
 *                                                                   *
 * CONTENTS:                                                         *
 *                                                                   *
 *   Declarations used in mht CORNER_TRACK-following (trackCorners). *
 *   The classes declared here are derived from the base classes in  *
 *   mdlmht.h.                                                       *
 *                                                                   *
 *                                                                   *
 *   The following describes the classes defined here:               *
 *                                                                   *
 *                            FALARM                                 *
 *								     *
 *          structure to store false alarms			     *
 *								     *
 *                        CORNER_TRACK_ELEMENT                       *
 *								     *
 *          an element of the CORNER_TRACK structure                 *
 *								     *
 *                            CORNER_TRACK                           *
 *								     *
 *          list of CORNER_TRACK_ELEMENTS                            *
 *                                                                   *
 *                                                                   *
 *                          CORNER_TRACK_MHT                         *
 *                                                                   *
 *   Derived from the base class MDL_MHT defined in mdlmht.H         *
 *   It takes the CONSTVEL_MDL, CONSTVEL_STATE, CONSTPOS_REPORT      *
 *   and uses them in the mht algorithm to group noisy pixel         *
 *   data into CORNER_TRACKs.                                        *
 *                                                                   *
 *                          CONSTPOS_REPORT                          *
 *                                                                   *
 *   A CONSTPOS_REPORT is a single corner (x,y)                      *
 *                                                                   *
 *   The falarmLogLikelihood is assigned by the routine creating the *
 *   CONSTPOS_REPORT (probably CORNER_TRACK_MHT::measure(), in       *
 *   trackCorners.c).                                                *
 *                                                                   *
 *                          CONSTVEL_STATE                           *
 *                                                                   *
 *   The member variables of CONSTVEL_STATEs can be divided into two *
 *   groups.  The first group contains the normal, Kalman filter     *
 *   values that describe a state estimate: the estimate itself, the *
 *   covariance matrix, and the log of the likelihood.               *
 *                                                                   *
 *   The second group contains all the parts of the Kalman filter    *
 *   calculations that can be computed without reference to a        *
 *   report.  These will be used in validating reports to the        *
 *   CORNER_TRACK, and in computing the resulting state estimates    *
 *   (and CONSTVEL_STATE objects).                                   *
 *                                                                   *
 *   This second group of variables is not computed at the time of   *
 *   the CONSTVEL_STATE's construction, since the CONSTVEL_STATE     *
 *   might be pruned away from the tree before it has a chance to    *
 *   have any reports validated to it.  Instead, they are computed   *
 *   by the member function setup().                                 *
 *                                                                   *
 *   The first thing that setup() does is to decide the length of    *
 *   the time step to use.  The time step is chosen such that it     *
 *   makes the predicted state estimate land in a neighboring pixel. *
 *   The time step is stored in the member variable m_ds.            *
 *                                                                   *
 *                           CONSTVEL_MDL                            *
 *                                                                   *
 *   The CONSTVEL_MDL class makes new CONSTVEL_STATEs from old ones. *
 *
 *   beginNewStates() tells how many new state should be generated   *
 *   We have restricted new track initiation to just the first frame *
 *   However, this restriction can easily be removed                 *
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

#ifndef MOTION_MDL_H
#define MOTION_MDL_H

#include "except.h"
#include "matrix.h"
#include "mdlmht.h"
#include "param.h"
#include "corner.h"
#include <math.h>
#include <cstdio>		// for  sprintf
#include <list>			// for std::list<>

static int g_numTracks;

class CONSTPOS_REPORT;
class CORNER_TRACK_MDL;
class CONSTPOS_STATE;
class CONSTPOS_MDL;
class CONSTVEL_STATE;
class CONSTVEL_MDL;
class CONSTCURV_STATE;
class CONSTCURV_MDL;
class CORNER_TRACK_MHT;


/*-------------------------------------------------------------------*
 *
 * Need to define measurement vector CONSTPOS_REPORT.
 * This must be derived from the generic base class MDL_REPORT

 * CONSTPOS_REPORT -- reported corner measurement for a CORNER_TRACK
 *
 * It is a vector containing the xy position
 *
 *
 *-------------------------------------------------------------------*/

class CONSTPOS_REPORT: public MDL_REPORT
{
    friend class CONSTVEL_STATE;
    friend class CONSTVEL_MDL;


private:

    double m_falarmLogLikelihood;    // log of the likelihood that
                                     // this report is a false alarm
                                     // (not really part of a CORNER_TRACK)
    MATRIX m_z;                      // (x, dx, y, dy)

public:
    Texture_t m_textureInfo;
    int m_frameNo;
    CONSTPOS_REPORT( const double &falarmLogLikelihood,
                     const double &x, const double &y,
                     const Texture_t &textureInfo,
                     const int &f):
        MDL_REPORT(),
        m_falarmLogLikelihood( falarmLogLikelihood ),
        m_z( 2, 1 ),
        m_frameNo(f),
        m_textureInfo(textureInfo)

    {
        m_z.set( x, y);
    }

    CONSTPOS_REPORT( const CONSTPOS_REPORT &src ):
        MDL_REPORT(),
        m_falarmLogLikelihood( src.m_falarmLogLikelihood ),
        m_z( src.m_z ),
        m_frameNo(src.m_frameNo)
    {
    }

    virtual void describe(int spaces)
    {
        m_z.print();
    }

    virtual void print()
    {
        std::cout << "  " <<m_z(0) << " " << m_z(1);
    }

    virtual double getFalarmLogLikelihood()
    {
        return m_falarmLogLikelihood;
    }

    MATRIX &getZ()
    {
        return m_z;
    }

    double getX()
    {
        return m_z( 0 );
    }
    double getY()
    {
        return m_z( 1 );
    }
    void printMeas()
    {
        printf("%lf %lf frame=%d\n",m_z(0),m_z(1),m_frameNo);
    }
};



/*-------------------------------------------------------------------*
 | CORNER_TRACK_MDL -- model of CORNER_TRACKs
 *-------------------------------------------------------------------*/

class CORNER_TRACK_MDL:public MODEL
{
public:
    int type;
    virtual double getStateX(MDL_STATE*)
    {
        return 0;
    }

    virtual double getStateY(MDL_STATE*)
    {
        return 0;
    }
};


/*-------------------------------------------------------------------*
 *
 * CONSTVEL_MDL -- model for cornerTracks
 *
 * CONSTVEL_MDL must be derived from MODEL
 *
 * CONSTVEL_MDL describes the user's model of a CORNER_TRACK
 * Here the model is implemented as a simple linear Kalman filter
 *
 *-------------------------------------------------------------------*/


class CONSTVEL_MDL: public CORNER_TRACK_MDL
{
private:
    double m_lambda_x;
    double m_startLogLikelihood;     // likelihood of a CORNER_TRACK starting
    double m_endLogLikelihood;       // likelihood of a CORNER_TRACK ending
    double m_continueLogLikelihood;  // likelihood of a CORNER_TRACK not
                                     //   ending
    double m_skipLogLikelihood;      // likelihood of not detecting a
                                     //   CORNER_TRACK that hasn't ended
    double m_detectLogLikelihood;    // likelihood of detecting a
                                     //   CORNER_TRACK that hasn't ended

    double m_maxDistance;            // maximum mahalanobis distance
                                     //   allowed for validating a
                                     //   report to a CORNER_TRACK

    double m_processVariance;        // process noise
    double m_intensityVariance;
    double m_stateVariance;
    MATRIX m_R;                      // measurement covariance
    MATRIX m_startP;                 // covariance matrix to use at
                                     //   start of a CORNER_TRACK
    double m_intensityThreshold;

public:

    CONSTVEL_MDL( double positionMeasureVarianceX,
                  double positionMeasureVarianceY,
                  double gradientMeasureVariance,
                  double intensityVariance,
                  double processVariance,
                  double startProb,
                  double lambda_x,
                  double detectProb,
                  double stateVar,
                  double intThreshold,
                  double maxDistance);

    virtual int beginNewStates( MDL_STATE *mdlState,
                                MDL_REPORT *mdlReport );
    virtual MDL_STATE *getNewState( int stateNum,
                                    MDL_STATE *mdlState,
                                    MDL_REPORT *mdlReport );
    virtual double getEndLogLikelihood( MDL_STATE * );
    virtual double getContinueLogLikelihood( MDL_STATE * );
    virtual double getSkipLogLikelihood( MDL_STATE *mdlState );
    virtual double getDetectLogLikelihood( MDL_STATE * )
    {
        return m_detectLogLikelihood;
    }
    virtual double getStateX(MDL_STATE *s);
    virtual double getStateY(MDL_STATE *s);

    double getCorr(CONSTVEL_STATE *s, CONSTPOS_REPORT *r);
private:

    CONSTVEL_STATE* getNextState( CONSTVEL_STATE *state,
                                  CONSTPOS_REPORT *report );
};

/*-------------------------------------------------------------------*
 *
 * CONSTVEL_STATE -- state estimate for a CORNER_TRACK
 *
 * CONSTVEL_STATE hold the state information for the CORNER_TRACK
 * E.g. the vector (x, dx, y, dy)
 *
 *-------------------------------------------------------------------*/

class CONSTVEL_STATE: public MDL_STATE
{
    friend class ONSTPOS_REPORT;
    friend class CONSTCURV_MDL;
    friend class CONSTVEL_MDL;
    friend class CONSTPOS_MDL;

private:

    MATRIX m_x;                      // state estimate (x, dx, y, dy)
    MATRIX m_P;                      // covariance matrix
    double m_logLikelihood;          // likelihood that this state
                                     //   is the true state of the
                                     //   CORNER_TRACK after the state
                                     //   that it was born from (in
                                     //   CONSTVEL_MDL::getNewState())

    int m_numSkipped;
    int m_hasBeenSetup;              // 0 before the following variables
                                     //   have been filled in, 1 after

    double m_ds;                     // "time" step until the next state
                                     //   (chosen so that the next state
                                     //   lands in a neighboring pixel)
    double m_logLikelihoodCoef;      // part of likelihood calculation
                                     //   that's independent of the
                                     //   inovation
    MATRIX *m_Sinv;                  // inverse of the innovation
                                     //   covariance
    MATRIX *m_W;                     // filter gain
    MATRIX *m_nextP;                 // updated state covariance
                                     //   (covariance for next state)
    MATRIX *m_x1;                    // state prediction
    Texture_t m_prevTextureInfo;

private:

    CONSTVEL_STATE( CONSTVEL_MDL *mdl,
                    const double &x,
                    const double &dx,
                    const double &y,
                    const double &dy,
                    const Texture_t &info,
                    MATRIX &P,
                    const double &logLikelihood,
                    const int &numSkipped):
        MDL_STATE( mdl ),
        m_logLikelihood( logLikelihood ),
        m_hasBeenSetup( 0 ),
        m_numSkipped(numSkipped),
        m_x(4,1),
        m_P(P),
        m_ds( 0 ),
        m_x1( 0 ),
        m_nextP( 0 ),
        m_Sinv( 0 ),
        m_W( 0 ),
        m_prevTextureInfo(info)
    {
        m_x(0)=x;
        m_x(1)=dx;
        m_x(2)=y;
        m_x(3)=dy;
    }


    CONSTVEL_STATE( const CONSTVEL_STATE &src ):
        MDL_STATE( src.getMdl() ),
        m_x( src.m_x ),
        m_P( src.m_P ),
        m_logLikelihood( src.m_logLikelihood ),
        m_hasBeenSetup( 0 ),
        m_numSkipped(src.m_numSkipped),
        m_ds( 0 ),
        m_x1( 0 ),
        m_nextP( 0 ),
        m_Sinv( 0 ),
        m_W( 0 ),
        m_prevTextureInfo(src.m_prevTextureInfo)
    {
    }


private:

    void setup( double processVariance, const MATRIX &R );

    void cleanup()
    {


        if( m_hasBeenSetup )
        {
            delete m_x1;
            m_x1 = 0;
            delete m_nextP;
            m_nextP = 0;
            delete m_Sinv;
            m_Sinv = 0;
            delete m_W;
            m_W = 0;

            m_hasBeenSetup = 0;
        }
    }


    int getNumSkipped()
    {
        return m_numSkipped;
    }
    double getLogLikelihoodCoef()
    {
        checkSetup();
        return m_logLikelihoodCoef;
    }
    MATRIX &getPrediction()
    {
        checkSetup();
        return *m_x1;
    }
    MATRIX &getNextP()
    {
        checkSetup();
        return *m_nextP;
    }
    MATRIX &getSinv()
    {
        checkSetup();
        return *m_Sinv;
    }
    MATRIX &getW()
    {
        checkSetup();
        return *m_W;
    }

#ifdef TSTBUG

    void checkSetup()
    {
        assert( m_hasBeenSetup );
        //  THROW_ERR( "Trying to get derived info from a CONSTPOS state"
        //             " that hasn't been setup()" )
    }

#else
    void checkSetup() {}
#endif

public:

    ~CONSTVEL_STATE()
    {
        cleanup();    //SHOULD THIS BE PRIVATE?
    }
    virtual double getLogLikelihood()
    {
        return m_logLikelihood;
    }

    virtual void print()
    {
        std::cout << "ConstVel State: "<< m_x(0) << " ,"
                  <<m_x(2);
    }
    double getX()
    {
        return m_x( 0 );
    }
    double getDX()
    {
        return m_x( 1 );
    }
    double getY()
    {
        return m_x( 2 );
    }
    double getDY()
    {
        return m_x( 3 );
    }

    void setDX(double val)
    {
        m_x( 1 )=val;
    }
    void setDY(double val)
    {
        m_x( 3 )=val;
    }

    double getX1()
    {
        checkSetup();
        return (*m_x1)( 0 );
    }
    double getDX1()
    {
        checkSetup();
        return (*m_x1)( 1 );
    }
    double getY1()
    {
        checkSetup();
        return (*m_x1)( 2 );
    }
    double getDY1()
    {
        checkSetup();
        return (*m_x1)( 3 );
    }

    double getDS()
    {
        checkSetup();
        return m_ds;
    }
};

/*-------------------------------------------------------------------*
 *
 * FALARM -- is a structure for holding flase alarms on an
 *           "intrusive" list.
 *
 * For the definition of an intrusive list see the file list.h
 *
 *-------------------------------------------------------------------*/

class FALARM: public DLISTnode
{
public:
    double rX, rY;
    int frameNo;

    FALARM( CONSTPOS_REPORT *xreport ):
        DLISTnode(),
        rX( xreport->getX() ),
        rY( xreport->getY() ),
        frameNo( xreport->m_frameNo )
    {
    }

protected:

    MEMBERS_FOR_DLISTnode( FALARM )
};


/*-------------------------------------------------------------------*
 *
 * CORNER_TRACK_ELEMENT -- is a structure for holding an individual
 * element of a CORNER_TRACK on an "intrusive" doubly linked list
 *
 * For the definition of an intrusive list see the file list.h
 *
 *-------------------------------------------------------------------*/


class CORNER_TRACK_ELEMENT: public DLISTnode
{
public:
    int hasReport;
    double sx,sy;
    double rx,ry;
    int frameNo;
    int time;
    double logLikelihood;
    char   model[30];

    CORNER_TRACK_ELEMENT(double s_x, double s_y, double r_x, double r_y, double prob, int type,int t,int f):
        DLISTnode(),
        sx(s_x),sy(s_y),rx(r_x),ry(r_y),logLikelihood(prob),time(t),frameNo(f)
    {
        if (!isnan(r_x) && !isnan(r_y))
        {
            hasReport=1;
        }
        else
        {
            hasReport=0;
        }

        switch(type)
        {
        case 1:
            sprintf(model,"CONSTANT MODEL");
            break;
        case 2:
            sprintf(model,"CONSTANT VELOCITY");
            break;
        case 3:
            sprintf(model,"CONSTANT CURV");
            break;
        }
    }

protected:

    MEMBERS_FOR_DLISTnode( CORNER_TRACK_ELEMENT )
};

/*-------------------------------------------------------------------*
 *
 * CORNER_TRACK -- is a structure to save info about a CORNER_TRACK
 * on an "intrusive" doubly linked list
 *
 * For the definition of an intrusive list see the file list.h
 *
 *-------------------------------------------------------------------*/

class CORNER_TRACK: public DLISTnode
{
public:
    int id;
    int color;

    std::list< CORNER_TRACK_ELEMENT > list;

    CORNER_TRACK( int idArg, int colorArg ):
        DLISTnode(),
        id( idArg ),
        color( colorArg ),
        list()
    {
    }

protected:

    MEMBERS_FOR_DLISTnode( CORNER_TRACK )
};



/*-------------------------------------------------------------------*
 *
 * CORNER_TRACK_MHT -- MDL_MHT class for CORNER_TRACK
 *
 *-------------------------------------------------------------------*/

class CORNER_TRACK_MHT: public MDL_MHT
{
public:

    CORNER_TRACK_MHT( double fprob,int maxDepth, double minGHypoRatio, int maxGHypos,
                      ptrDLIST_OF<MODEL> mdlist ):
        MDL_MHT( maxDepth, minGHypoRatio, maxGHypos ),
        m_falarmLogLikelihood( log(fprob) ),
        m_cornerTracks(),
        m_falarms()
    {
        m_modelList.appendCopy( mdlist );
    }
    virtual void describe(int spaces=0);
    virtual std::list< CORNER_TRACK > GetTracks() const
    {
        return m_cornerTracks;
    }
    virtual std::list< FALARM > GetFalseAlarms() const
    {
        return m_falarms;
    }

private:
    double m_falarmLogLikelihood;
    std::list< CORNER_TRACK > m_cornerTracks;
    std::list< FALARM > m_falarms;

protected:

    virtual void measure(const std::list<CORNER> &newReports);

    virtual void startTrack( int trackId, int,
                             MDL_STATE *state, MDL_REPORT *report )
    {
        g_numTracks++;
        CONSTPOS_REPORT* r= (CONSTPOS_REPORT*)report;

        CONSTVEL_MDL* mdl = (CONSTVEL_MDL*)(state->getMdl());
//      printf("Calling Verify in statrtTRack\n");
        verify( trackId, r->getX(),
                r->getY(),
                mdl->getStateX(state),
                mdl->getStateY(state),
                state->getLogLikelihood(),
                mdl->type,r->m_frameNo);
    }

    virtual void continueTrack( int trackId, int,
                                MDL_STATE *state, MDL_REPORT *report )
    {
        CONSTPOS_REPORT* r= (CONSTPOS_REPORT*)report;
        CONSTVEL_MDL* mdl = (CONSTVEL_MDL*)(state->getMdl());
//      printf("Calling Verify in continueTRack\n");
        verify( trackId, r->getX(),
                r->getY(),
                mdl->getStateX(state),
                mdl->getStateY(state),
                state->getLogLikelihood(),
                mdl->type,r->m_frameNo);
    }

    virtual void skipTrack( int trackId, int, MDL_STATE *state )
    {
        CONSTVEL_MDL* mdl = (CONSTVEL_MDL*)(state->getMdl());
//      printf("Calling Verify in skipTRack\n");
        verify( trackId, NAN, NAN, mdl->getStateX(state), mdl->getStateY(state),
                state->getLogLikelihood(),
                mdl->type,-9);
    }

    virtual void endTrack( int, int )
    {
//      printf("Verifying endTrack\n");
        g_numTracks--;
    }

    virtual void falseAlarm( int, MDL_REPORT *report )
    {
        saveFalarm( (CONSTPOS_REPORT*)report );
    }

private:
    CORNER_TRACK* findTrack( const int &id );
    void saveFalarm( CONSTPOS_REPORT *report );
    void verify( int trackID, double r_x, double r_y, double s_x,
                 double s_y, double likelihood, int modelType, int frame);
};

int getTrackColor( int trackId );

#endif







