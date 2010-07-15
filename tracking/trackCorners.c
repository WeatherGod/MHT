#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "param.h"       //  contains values of needed parameters 
#include "motionModel.h"

/* 
 * External Variables
 */
int g_isFirstScan=1;
iDLIST_OF< FALARM > *g_falarms_ptr;   // list of false alarms found
iDLIST_OF< CORNER_TRACK > *g_cornerTracks_ptr; // list of cornerTracks found
CORNERLIST *g_currentCornerList;
int g_time;

Parameter g_param;        

int main(int argc, char **argv)
{
  void read_param(char*);
  void writeCornerTrackFile(char *name);
  void readCorners(iDLIST_OF<CORNERLIST> *in);
  int numPixels;
  iDLIST_OF<CORNERLIST> *inputData;
  ptrDLIST_OF<MODEL> mdl;

/*
 * Check if program called correctly
 */

  if ( argc < 2 )
  {
    cerr<<"Usage:"<< argv[0]<<" OutDataFile -p paramFile"<< "InDataFile" << endl;
    exit( -1 );
  }


/*
 * Create the global list of CornerTracks & false alarms
 * Every corner will be assigned to to either a CornerTrack or a false alarm
 */

  g_cornerTracks_ptr = new iDLIST_OF<CORNER_TRACK>;
  g_falarms_ptr = new iDLIST_OF< FALARM >;


/*
 * Read the parameters
 */

  char *paramFile;
  if (argc > 2) {
     if (argv[2][0] =='-' && argv[2][1] == 'p')
       paramFile = argv[3];
     printf("%s %s\n",argv[3],paramFile);
  } else {
    paramFile="Parameters";
  }
  read_param(paramFile);




/*
 * Read the corners
 */

  inputData = new iDLIST_OF<CORNERLIST>;
  readCorners(inputData);




/*
 * Create constant velocity model
 */   

  CONSTVEL_MDL *cvmdl = new CONSTVEL_MDL
                    ( g_param.positionVarianceX,
                      g_param.positionVarianceY,
                      g_param.gradientVariance,
                      g_param.intensityVariance,
                      g_param.processVariance,
                      g_param.meanNew,
                      g_param.probEnd,
                      g_param.probDetect,
                      g_param.stateVariance,
                      g_param.intensityThreshold,
                      g_param.maxDistance2);
  mdl.append( (*cvmdl));




/*
 * Setup mht algorithm with CornerTrack model mdl
 */

  CORNER_TRACK_MHT mht( g_param.meanFalarms,
                   g_param.maxDepth,
                   g_param.minGHypoRatio,
                   g_param.maxGHypos,
                   mdl );



/*
 *  Get the 1st set of measurements or the corners of the
 *  1st frame
 */

  PTR_INTO_iDLIST_OF<CORNERLIST> cornerPtr(*inputData, START_AT_HEAD);
  g_currentCornerList = cornerPtr.get();


/*
 * Now do all the work by calling mht.scan()
 * mht.scan() returns 0 when there are no more measurements to be processed
 * Otherwise it gets the next set of measurements and processes them.
 */

  int didIscan=0;
  while( (didIscan=mht.scan()) != 0 )
  {
      printf("******************CURRENT_TIME=%d ENDTIME=%d****************\n",
                 mht.getCurrentTime(),g_param.endScan);
      g_time=mht.getCurrentTime();
      mht.printStats(2);
      if( mht.getCurrentTime() > g_param.endScan )
           break;

      if (didIscan) {
        if ( g_isFirstScan)   g_isFirstScan=0;
//        mht.describe();
        ++cornerPtr;
        if (!cornerPtr.isValid()) break;
        g_currentCornerList = cornerPtr.get();
      }

    }
//    mht.describe();
    printf("\n CLEARING \n");
    mht.clear();
//    mht.describe();

/* 
 * Finished finding CORNER_TRACKs so write out result
 * And write out dataFile containing list of CORNER_TRACKs and 
 * associated corners
 */
 
  writeCornerTrackFile(argv[1]);

}

/*----------------------------------------------------------*
 * read_param():  Read the parameter file
 * A line that begins with ; in the file is considered
 * a comment. So skip the comments.
 *----------------------------------------------------------*/

void read_param(char* paramFile)
{
BGN
  FILE *fp;

  fp = fopen( paramFile, "r" );
//  cout << "Open Parameter File :" <<paramFile<<endl;
  if (fp == 0) 
      THROW_ERR("Couldn't open file parameter File")
  else if( fp != 0 )
  {
    cout << "Using Parameter File :" << paramFile << endl;
    char buf[ 100 ];
    char *f;

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.positionVarianceX = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.positionVarianceY = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.gradientVariance = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.intensityVariance = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.processVariance = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.probDetect = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.probEnd = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.meanNew = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.meanFalarms = atof( buf );
   
    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.maxGHypos = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.maxDepth = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.minGHypoRatio = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.intensityThreshold = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.maxDistance1 = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.maxDistance2 = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.maxDistance3 = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.stateVariance = atof( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.endScan = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.pos2velLikelihood = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.vel2curvLikelihood = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.startA = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.startB = atoi( buf );

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f ) g_param.startC = atoi( buf );


    fclose( fp );
  }

  cout << " positionVarianceX = " << g_param.positionVarianceX << endl;
  cout << " positionVarianceY = " << g_param.positionVarianceY << endl;
  cout << " gradientVariance = " << g_param.gradientVariance << endl;
  cout << " intensityVariance = " << g_param.intensityVariance << endl;
  cout << " processVariance = " << g_param.processVariance << endl;
  cout << " probDetect = " << g_param.probDetect << endl;
  cout << " probEnd = " << g_param.probEnd << endl;
  cout << " meanNew = " << g_param.meanNew << endl;
  cout << " meanFalarms = " << g_param.meanFalarms << endl;
  cout << " maxGHypos = " << g_param.maxGHypos << endl;
  cout << " maxDepth = " << g_param.maxDepth << endl;
  cout << " minGHypoRatio = " << g_param.minGHypoRatio << endl;
  cout << " intensityThreshold= " << g_param.intensityThreshold << endl;
  cout << " maxDistance1= " << g_param.maxDistance1 << endl;
  cout << " maxDistance2= " << g_param.maxDistance2 << endl;
  cout << " maxDistance3= " << g_param.maxDistance3 << endl;





}

/*----------------------------------------------------------*
 * writeCornerTrackFile():  Write all information regarding 
 * the CORNER_TRACKs into a file.
 *----------------------------------------------------------*/

void writeCornerTrackFile(char *name)
{
BGN
  PTR_INTO_iDLIST_OF< CORNER_TRACK > cornerTrack;
  PTR_INTO_iDLIST_OF< CORNER_TRACK_ELEMENT > cornerTrackEl;
  PTR_INTO_iDLIST_OF< FALARM > falarm;
  int id;

  FILE *CornerTrackFile;
  CornerTrackFile = fopen( name, "w" );

  fprintf(CornerTrackFile,
		"#INFORMATION REGARDING THIS CORNER TRACKER\n");
  fprintf(CornerTrackFile,
		"#___________________________________________\n");
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#\n");

 /*
   * Write out the parameters that were used 
   */
  fprintf(CornerTrackFile,"#    Parameters: \n");
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         PositionVarianceX:  %f\n",
				g_param.positionVarianceX);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         PositionVarianceY:  %f\n",
				g_param.positionVarianceY);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         GradientVariance:  %f\n",
				g_param.gradientVariance);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         intensityVariance:  %f\n",
				g_param.intensityVariance);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         ProcessVariance:  %f\n",
				g_param.processVariance);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         StateVariance:  %f\n",
				g_param.stateVariance);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Prob. Of Detection:  %f\n",
				g_param.probDetect);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Prob Of Track Ending:  %f\n",
				g_param.probEnd );
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Mean New Tracks:  %f\n",
				g_param.meanNew);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Mean False Alarms:  %f\n",
				g_param.meanFalarms);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Max Global Hypo:  %d\n",
				g_param.maxGHypos);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Max Depth:  %d\n",
				g_param.maxDepth);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         MinGHypoRatio:  %f\n",
				g_param.minGHypoRatio);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         intensity Threshold:  %f\n",
				g_param.intensityThreshold);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Max Mahalinobus Dist1:  %f\n",
				g_param.maxDistance1);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Max Mahalinobus Dist2:  %f\n",
				g_param.maxDistance1);
  fprintf(CornerTrackFile,"#\n");
  fprintf(CornerTrackFile,"#         Max Mahalinobus Dist3:  %f\n",
				g_param.maxDistance1);
  fprintf(CornerTrackFile,"#\n");

  /*
   * Write the number of CornerTracks & falsealarms
   */
  fprintf( CornerTrackFile, "%d\n", g_cornerTracks_ptr->getLength() );
  fprintf( CornerTrackFile, "%d\n", g_falarms_ptr->getLength() );

  /*
   * Information about each CornerTrack
   *     CornerTrackId  CornerTrackLength CornerTrackColor
   *     Code Measurement(x dx y dy)  EstimatedState(x dx y dy)
   *     
   *     The Code is either M or S. M, if there was a mesurement
   *     and 'S'(skipped) if no measurement was found at that
   *      time step
   */
  id = 0;
  LOOP_DLIST( cornerTrack, *g_cornerTracks_ptr )
  {
    fprintf( CornerTrackFile, "%d %d\n",
                            id++,
                            (*cornerTrack).list.getLength());

    LOOP_DLIST( cornerTrackEl, (*cornerTrack).list )
    {
      fprintf( CornerTrackFile, "%c %f %f %f %f %f %d %d %s\n",
                              (*cornerTrackEl).hasReport ? 'M':'S',
                              (*cornerTrackEl).rx,
                              (*cornerTrackEl).ry,
                              (*cornerTrackEl).sx,
                              (*cornerTrackEl).sy,
                              (*cornerTrackEl).logLikelihood,
                              (*cornerTrackEl).time,
                              (*cornerTrackEl).frameNo,
                              (*cornerTrackEl).model );
    }
  }

  /*
   * Information about each false alarm
   *     x dx y dy
   */
  LOOP_DLIST( falarm, *g_falarms_ptr )
  {
    fprintf( CornerTrackFile, "%f %f %d\n",
                            (*falarm).rX,
                            (*falarm).rY,
                            (*falarm).frameNo);
  }
  fclose( CornerTrackFile );
}

/*------------------------------------------------------------------*
 * readCorners():  Read corners from the motion sequence.  Currently
 * information about corner files, their name, start frame end frame
 * and the number of corners in each frame are read from stdin
 * 
 *------------------------------------------------------------------*/


void readCorners( iDLIST_OF<CORNERLIST> *inputData)
{
BGN
  int i;
  PTR_INTO_iDLIST_OF<CORNERLIST> cptr;
  PTR_INTO_iDLIST_OF<CORNER> ptr;
  int ncorners[100];
  int strSize=4096;
  char str[4096];
  int npoints;
  int startFrame=4;
  int totalFrames;
  char basename[80];

/*
 * Read basename for image sequence, total # of  frames, start 
 * frame number, and number of corners in each frame
 */

  scanf("%s %d %d",basename,&totalFrames,&startFrame);
  for (i=0; i < totalFrames; i++) {
    scanf("%d",&npoints);
    ncorners[i] = npoints;
    printf("ncorners[%d]=%d\n",i,ncorners[i]);
    inputData->append(new CORNERLIST(ncorners[i]));
  }

/*
 * Open each frame and read the corner Data from them, saving
 * the data in inputData
 */

  i=startFrame;
  LOOP_DLIST(cptr,*inputData) {
    int x,y;
    USHORT i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15;
    USHORT i16,i17,i18,i19,i20,i21,i22,i23,i24,i25; 
    FILE *inDataFile;
    char fname[80];
    sprintf(fname,"%s.%d",basename,i++);
//    printf("Reading file %s\n",fname);
    inDataFile = fopen( fname, "r" );
    int j=0;
    while (fgets(str,strSize,inDataFile) && j <ncorners[i-startFrame-1]) 
    {
#ifdef OXFORD    // float data
      float fx,fy;
      sscanf(str,"%f %f",&fx,&fy);
      x = (int)fx;
      y = (int)fy;
#else           // integer data

//      sscanf(str,"%d %d %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd",&x,&y,&i1,&i2,&i3,&i4,&i5,&i6,&i7,&i8, &i9, &i10, &i11, &i12, &i13, &1i4, &i15,&i16, &i17, &i18, &i19, &i20, &i21, &i22, &i23, &i24, &i25);
      sscanf(str,"%d %d %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd",&x,&y,&i1,&i2,&i3,&i4,&i5,&i6,&i7,&i8, &i9, &i10, &i11, &i12, &i13, &i14, &i15, &i16 , &i17, &i18, &i19, &i20, &i21, &i22, &i23, &i24, &i25 );

#endif

      (*cptr).list.append(new CORNER(x,y,i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20,i21,i22,i23,i24,i25,i-1));
      j++;
    }
    fclose(inDataFile);
  }

}




















