#include <cstdlib>
#include <cstdio>	// for sscanf
#include <string.h>
#include <cmath>
#include <iostream>	// for std::cout, std::endl, std::cerr
#include <fstream>	// for ifstream and ofstream
#include <sstream>	// for stringstream
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
    Parameter read_param(const std::string &paramFileName);
    void writeCornerTrackFile(const std::string &trackFileName, const Parameter &param);
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

    static struct option TheLongOptions[] =
    {
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

    const Parameter param = read_param(paramFileName.c_str());

    /*
     * Read the corners
     */

    inputData = new iDLIST_OF<CORNERLIST>;
    readCorners(inputFileName, inputData);

    /*
     * Create constant velocity model
     */

    CONSTVEL_MDL *cvmdl = new CONSTVEL_MDL
    ( param.positionVarianceX,
      param.positionVarianceY,
      param.gradientVariance,
      param.intensityVariance,
      param.processVariance,
      param.meanNew,
      param.probEnd,
      param.probDetect,
      param.stateVariance,
      param.intensityThreshold,
      param.maxDistance2);
    mdl.append( (*cvmdl) );




    /*
     * Setup mht algorithm with CornerTrack model mdl
     */

    CORNER_TRACK_MHT mht( param.meanFalarms,
                          param.maxDepth,
                          param.minGHypoRatio,
                          param.maxGHypos,
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
        std::cout << "******************CURRENT_TIME=" << mht.getCurrentTime() << ' '
                  << "ENDTIME=" << param.endScan << "****************\n";
        g_time=mht.getCurrentTime();
        mht.printStats(2);
        if( mht.getCurrentTime() > param.endScan )
        {
            break;
        }

        if (didIscan)
        {
            if ( g_isFirstScan)
            {
                g_isFirstScan=0;
            }
//        mht.describe();
            ++cornerPtr;
            if (!cornerPtr.isValid())
            {
                break;
            }
            g_currentCornerList = cornerPtr.get();
        }

    }
//    mht.describe();
    std::cout << "\n CLEARING \n";
    mht.clear();
//    mht.describe();

    /*
     * Finished finding CORNER_TRACKs so write out result
     * And write out dataFile containing list of CORNER_TRACKs and
     * associated corners
     */

    writeCornerTrackFile(outputFileName, param);

}

/*----------------------------------------------------------*
 * read_param():  Read the parameter file
 * A line that begins with ; in the file is considered
 * a comment. So skip the comments.
 *----------------------------------------------------------*/

Parameter read_param(const std::string &paramFile)
{

    FILE *fp;

    fp = fopen( paramFile.c_str(), "r" );
//  std::cout << "Open Parameter File :" << paramFile << std::endl;
    if (fp <= 0)
    {
        throw std::runtime_error("Couldn't open parameter file: " + paramFile);
    }


    std::cout << "Using Parameter File: " << paramFile << std::endl;
    Parameter param;
    char buf[ 100 ];
    char *f;

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.positionVarianceX = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.positionVarianceY = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.gradientVariance = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.intensityVariance = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.processVariance = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.probDetect = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.probEnd = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.meanNew = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.meanFalarms = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.maxGHypos = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.maxDepth = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.minGHypoRatio = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.intensityThreshold = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.maxDistance1 = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.maxDistance2 = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.maxDistance3 = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.stateVariance = atof( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.endScan = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.pos2velLikelihood = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.vel2curvLikelihood = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.startA = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.startB = atoi( buf );
    }

    while( (f = fgets( buf, sizeof( buf ), fp )) && buf[ 0 ] == ';' ) ;
    if( f )
    {
        param.startC = atoi( buf );
    }


    fclose( fp );


    std::cout << " positionVarianceX = " << param.positionVarianceX << std::endl;
    std::cout << " positionVarianceY = " << param.positionVarianceY << std::endl;
    std::cout << " gradientVariance = " << param.gradientVariance << std::endl;
    std::cout << " intensityVariance = " << param.intensityVariance << std::endl;
    std::cout << " processVariance = " << param.processVariance << std::endl;
    std::cout << " probDetect = " << param.probDetect << std::endl;
    std::cout << " probEnd = " << param.probEnd << std::endl;
    std::cout << " meanNew = " << param.meanNew << std::endl;
    std::cout << " meanFalarms = " << param.meanFalarms << std::endl;
    std::cout << " maxGHypos = " << param.maxGHypos << std::endl;
    std::cout << " maxDepth = " << param.maxDepth << std::endl;
    std::cout << " minGHypoRatio = " << param.minGHypoRatio << std::endl;
    std::cout << " intensityThreshold= " << param.intensityThreshold << std::endl;
    std::cout << " maxDistance1= " << param.maxDistance1 << std::endl;
    std::cout << " maxDistance2= " << param.maxDistance2 << std::endl;
    std::cout << " maxDistance3= " << param.maxDistance3 << std::endl;

    return param;
}

/*----------------------------------------------------------*
 * writeCornerTrackFile():  Write all information regarding
 * the CORNER_TRACKs into a file.
 *----------------------------------------------------------*/

void writeCornerTrackFile(const std::string &name, const Parameter &param)
{

    PTR_INTO_iDLIST_OF< CORNER_TRACK > cornerTrack;
    PTR_INTO_iDLIST_OF< CORNER_TRACK_ELEMENT > cornerTrackEl;
    PTR_INTO_iDLIST_OF< FALARM > falarm;
    int id;

    std::ofstream CornerTrackFile(name.c_str(), std::ios_base::out);

    if (!CornerTrackFile.is_open())
    {
        throw std::runtime_error("Could not open corner track file: " + name);
    }

    CornerTrackFile << "#INFORMATION REGARDING THIS CORNER TRACKER\n"
                    << "#___________________________________________\n"
                    << "#\n#\n"
                    /*
                     * Write out the parameters that were used
                     */
                    << "#    Parameters: \n"
                    << "#\n"
                    << "#         PositionVarianceX:  " << param.positionVarianceX << "\n"
                    << "#\n"
                    << "#         PositionVarianceY:  " << param.positionVarianceY << "\n"
                    << "#\n"
                    << "#         GradientVariance:  " << param.gradientVariance << "\n"
                    << "#\n"
                    << "#         intensityVariance:  " << param.intensityVariance << "\n"
                    << "#\n"
                    << "#         ProcessVariance:  " << param.processVariance << "\n"
                    << "#\n"
                    << "#         StateVariance:  " << param.stateVariance << "\n"
                    << "#\n"
                    << "#         Prob. Of Detection:  " << param.probDetect << "\n"
                    << "#\n"
                    << "#         Prob Of Track Ending:  " << param.probEnd << "\n"
                    << "#\n"
                    << "#         Mean New Tracks:  " << param.meanNew << "\n"
                    << "#\n"
                    << "#         Mean False Alarms:  " << param.meanFalarms << "\n"
                    << "#\n"
                    << "#         Max Global Hypo:  " << param.maxGHypos << "\n"
                    << "#\n"
                    << "#         Max Depth:  " << param.maxDepth << "\n"
                    << "#\n"
                    << "#         MinGHypoRatio:  " << param.minGHypoRatio << "\n"
                    << "#\n"
                    << "#         intensity Threshold:  " << param.intensityThreshold << "\n"
                    << "#\n"
                    << "#         Max Mahalinobus Dist1:  " << param.maxDistance1 << "\n"
                    << "#\n"
                    << "#         Max Mahalinobus Dist2:  " << param.maxDistance1 << "\n"
                    << "#\n"
                    << "#         Max Mahalinobus Dist3:  " << param.maxDistance1 << "\n"
                    << "#" << std::endl;

    /*
     * Write the number of CornerTracks & falsealarms
     */
    CornerTrackFile << g_cornerTracks_ptr->getLength() << "\n"
                    << g_falarms_ptr->getLength() << std::endl;

    /*
     * Information about each CornerTrack
     *     CornerTrackId  CornerTrackLength CornerTrackColor
     *     Code Measurement(x dx y dy)  EstimatedState(x dx y dy)
     *
     *     The Code is either M or S. M, if there was a mesurement
     *     and 'S'(skipped) if no measurement was found at that
     *      time step
     */
    // Looping over each track
    id = 0;
    LOOP_DLIST( cornerTrack, *g_cornerTracks_ptr )
    {
        CornerTrackFile << id++ << ' ' << (*cornerTrack).list.getLength() << std::endl;

        // Looping over each corner of the track
        LOOP_DLIST( cornerTrackEl, (*cornerTrack).list )
        {
            CornerTrackFile << ((*cornerTrackEl).hasReport ? 'M':'S') << ' '
                            << (*cornerTrackEl).rx << ' '
                            << (*cornerTrackEl).ry << ' '
                            << (*cornerTrackEl).sx << ' '
                            << (*cornerTrackEl).sy << ' '
                            << (*cornerTrackEl).logLikelihood << ' '
                            << (*cornerTrackEl).time << ' '
                            << (*cornerTrackEl).frameNo << ' '
                            << (*cornerTrackEl).model << std::endl;
        }
    }

    /*
     * Information about each false alarm
     *     x y t
     */
    // Looping over the false alarms
    LOOP_DLIST( falarm, *g_falarms_ptr )
    {
        CornerTrackFile << (*falarm).rX << ' '
                        << (*falarm).rY << ' '
                        << (*falarm).frameNo << std::endl;
    }
    CornerTrackFile.close();
}

/*------------------------------------------------------------------*
 * readCorners():  Read corners from the motion sequence.  Currently
 * information about corner files, their name, start frame end frame
 * and the number of corners in each frame are read from stdin
 *
 *------------------------------------------------------------------*/


void readCorners(const std::string &inputFileName, iDLIST_OF<CORNERLIST> *inputData)
{

    PTR_INTO_iDLIST_OF<CORNERLIST> cptr;
    PTR_INTO_iDLIST_OF<CORNER> ptr;
    int ncorners[100];
    std::string str;
    int npoints;
    // TODO: There is probably a very smart reason for this, but it escapes me at the moment
    //       I suspect that it has something to do with a case where the control file is empty
    //       maybe?
    int startFrame=4;
    int totalFrames;
    std::string basename;
    std::ifstream controlFile(inputFileName.c_str(), std::ios_base::in);
    if (!controlFile.is_open())
    {
        throw std::runtime_error("Could not open the input data file: " + inputFileName + "\n");
    }

    /*
     * Read basename for image sequence, total # of  frames, start
     * frame number, and number of corners in each frame
     */

    controlFile >> basename >> totalFrames >> startFrame;
    for (int frameIndex=0; frameIndex < totalFrames; frameIndex++)
    {
        controlFile >> npoints;
        ncorners[frameIndex] = npoints;
        std::cout << "ncorners[" << frameIndex << "]=" << ncorners[frameIndex] << std::endl;
        inputData->append(new CORNERLIST(ncorners[frameIndex]));
    }

    controlFile.close();

    /*
     * Open each frame and read the corner Data from them, saving
     * the data in inputData
     */

    int i = startFrame;
    LOOP_DLIST(cptr,*inputData)
    {
        int x,y;
        USHORT i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15;
        USHORT i16,i17,i18,i19,i20,i21,i22,i23,i24,i25;
        std::stringstream stringRep;
        stringRep << basename << '.' << i++;
        std::string fname = stringRep.str();
        std::ifstream inDataFile(fname.c_str(), std::ios_base::in);
//    std::cout << "Reading file " << fname << "\n";

        if (!inDataFile.is_open())
        {
            throw std::runtime_error("Could not open the input data file: " + fname);
        }

        int j=0;
        while (std::getline(inDataFile, str) && j < ncorners[i-startFrame-1])
        {
#ifdef OXFORD    // float data
            float fx,fy;
            sscanf(str.c_str(),"%f %f",&fx,&fy);
            x = (int)fx;
            y = (int)fy;
#else           // integer data

            sscanf(str.c_str(),"%d %d %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd %hd",&x,&y,&i1,&i2,&i3,&i4,&i5,&i6,&i7,&i8, &i9, &i10, &i11, &i12, &i13, &i14, &i15,&i16, &i17, &i18, &i19, &i20, &i21, &i22, &i23, &i24, &i25 );

#endif

            (*cptr).list.append(new CORNER(x,y,i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20,i21,i22,i23,i24,i25,i-1));
            j++;
        }
        inDataFile.close();
    }

}

