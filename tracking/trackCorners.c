#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <cmath>
#include <iostream>	// for std::cout, std::endl, std::cerr
#include "param.h"       //  contains values of needed parameters 
#include "motionModel.h"

#include <stdexcept>	// for std::runtime_error

#include <unistd.h>	// for optarg, opterr, optopt
#include <getopt.h>	// for getopt_long()

/* 
 * External Variables
 */
int g_isFirstScan=1;
iDLIST_OF< FALARM > *g_falarms_ptr;   // list of false alarms found
iDLIST_OF< CORNER_TRACK > *g_cornerTracks_ptr; // list of cornerTracks found
CORNERLIST *g_currentCornerList;
int g_time;
Parameter g_param;        

void PrintSyntax()
{
	std::cerr << "trackCorners -o OUTFILE [-p PARAM_FILE] -i INFILE\n"
		  << "             [--syntax | -x] [--help | -h]\n";
}

void PrintHelp()
{
	PrintSyntax();
        std::cerr << "-o  --output  OUTFILE\n"
                  << "The file that you want to write the track data to.\n\n";
 
        std::cerr << "-p  --param   PARAM_FILE\n"
                  << "The file where the tracking parameters can be found.\n"
                  << "Defaults to './Parameters'.\n\n";
 
        std::cerr << "-i  --input   INFILE\n"
                  << "INFILE contains the filestem of the corner files, the range of frames\n"
                  << "and the number of identified features for each frame.\n\n";
  
        std::cerr << "-x  --syntax\n"
                  << "Print the syntax for running this program.\n\n";
 
        std::cerr << "-h  --help\n"
                  << "Print this help page.\n\n";
}



int main(int argc, char **argv)
{
  void read_param(const char*);
  void writeCornerTrackFile(const char *name);
  void readCorners(const std::string &inputFileName, iDLIST_OF<CORNERLIST> *in);
  int numPixels;
  iDLIST_OF<CORNERLIST> *inputData;
  ptrDLIST_OF<MODEL> mdl;

  std::string outputFileName = "";
  std::string paramFileName = "./Parameters";
  std::string inputFileName = "";

  int OptionIndex = 0;
  int OptionChar = 0;
  bool OptionError = false;
  opterr = 0;			// don't print out error messages, let this program do that.

  static struct option TheLongOptions[] = {
    {"output", 1, NULL, 'o'},
    {"param", 1, NULL, 'p'},
    {"input", 1, NULL, 'i'},
    {"syntax", 0, NULL, 'x'},
    {"help", 0, NULL, 'h'},
    {0, 0, 0, 0}
  };

  while ((OptionChar = getopt_long(argc, argv, "o:p:i:xh", TheLongOptions, &OptionIndex)) != -1)
  {
    switch (OptionChar)
    {
      case 'o':
        outputFileName = optarg;
        break;
      case 'p':
        paramFileName = optarg;
        break;
      case 'i':
        inputFileName = optarg;
        break;
      case 'x':
        PrintSyntax();
        return(1);
        break;
      case 'h':
        PrintHelp();
        return(1);
        break;
      case '?':
        std::cerr << "ERROR: Unknown arguement: -" << (char) optopt << std::endl;
        OptionError = true;
        break;
      case ':':
        std::cerr << "ERROR: Missing value for arguement: -" << (char) optopt << std::endl;
        OptionError = true;
        break;
      default:
        std::cerr << "ERROR: Programming error... Unaccounted option: -" << (char) OptionChar << std::endl;
        OptionError = true;
        break;
    }
  }

  if (OptionError)
  {
    PrintSyntax();
    return(-1);
  }
 
  if (outputFileName.empty())
  {
    std::cerr << "ERROR: Missing OUTFILE name\n";
    PrintSyntax();
    return(-1);
  }
 
  if (inputFileName.empty())
  {
    std::cerr << "ERROR: Missing INFILE name\n";
    PrintSyntax();
    return(-1);
  }

  if (paramFileName.empty())
  {
    std::cerr << "ERROR: Missing or empty PARAM_FILE name\n";
    PrintSyntax();
    return(-1);
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

  read_param(paramFileName.c_str());

/*
 * Read the corners
 */

  inputData = new iDLIST_OF<CORNERLIST>;
  readCorners(inputFileName, inputData);

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
  mdl.append( (*cvmdl) );




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
  std::cout << "About to scan...\n";

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
 
  writeCornerTrackFile(outputFileName.c_str());

}

/*----------------------------------------------------------*
 * read_param():  Read the parameter file
 * A line that begins with ; in the file is considered
 * a comment. So skip the comments.
 *----------------------------------------------------------*/

void read_param(const char* paramFile)
{

  FILE *fp;

  fp = fopen( paramFile, "r" );
//  cout << "Open Parameter File :" <<paramFile<<endl;
  if (fp <= 0)
  {
	throw std::runtime_error("Couldn't open parameter file");
  //    THROW_ERR("Couldn't open file parameter File")
  }
  else
  {
    std::cout << "Using Parameter File: " << paramFile << std::endl;
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

  std::cout << " positionVarianceX = " << g_param.positionVarianceX << std::endl;
  std::cout << " positionVarianceY = " << g_param.positionVarianceY << std::endl;
  std::cout << " gradientVariance = " << g_param.gradientVariance << std::endl;
  std::cout << " intensityVariance = " << g_param.intensityVariance << std::endl;
  std::cout << " processVariance = " << g_param.processVariance << std::endl;
  std::cout << " probDetect = " << g_param.probDetect << std::endl;
  std::cout << " probEnd = " << g_param.probEnd << std::endl;
  std::cout << " meanNew = " << g_param.meanNew << std::endl;
  std::cout << " meanFalarms = " << g_param.meanFalarms << std::endl;
  std::cout << " maxGHypos = " << g_param.maxGHypos << std::endl;
  std::cout << " maxDepth = " << g_param.maxDepth << std::endl;
  std::cout << " minGHypoRatio = " << g_param.minGHypoRatio << std::endl;
  std::cout << " intensityThreshold= " << g_param.intensityThreshold << std::endl;
  std::cout << " maxDistance1= " << g_param.maxDistance1 << std::endl;
  std::cout << " maxDistance2= " << g_param.maxDistance2 << std::endl;
  std::cout << " maxDistance3= " << g_param.maxDistance3 << std::endl;
}

/*----------------------------------------------------------*
 * writeCornerTrackFile():  Write all information regarding 
 * the CORNER_TRACKs into a file.
 *----------------------------------------------------------*/

void writeCornerTrackFile(const char *name)
{

  PTR_INTO_iDLIST_OF< CORNER_TRACK > cornerTrack;
  PTR_INTO_iDLIST_OF< CORNER_TRACK_ELEMENT > cornerTrackEl;
  PTR_INTO_iDLIST_OF< FALARM > falarm;
  int id;

  FILE *CornerTrackFile;
  CornerTrackFile = fopen( name, "w" );

  if (CornerTrackFile <= 0)
  {
    throw std::runtime_error("Could not open corner track file...");
  }

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


void readCorners(const std::string &inputFileName, iDLIST_OF<CORNERLIST> *inputData)
{

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
  FILE *controlFile = fopen( inputFileName.c_str(), "r" );
  if (controlFile <= 0)
  {
     throw std::runtime_error("Could not open the input data file: " + inputFileName + "\n");
  }

/*
 * Read basename for image sequence, total # of  frames, start 
 * frame number, and number of corners in each frame
 */

  fscanf(controlFile, "%s %d %d",basename,&totalFrames,&startFrame);
  for (i=0; i < totalFrames; i++) {
    fscanf(controlFile,"%d",&npoints);
    ncorners[i] = npoints;
    printf("ncorners[%d]=%d\n",i,ncorners[i]);
    inputData->append(new CORNERLIST(ncorners[i]));
  }

  fclose(controlFile);  

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

    if (inDataFile <= 0)
    {
       throw std::runtime_error("Could not open the input data file...\n");
    }
    
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

