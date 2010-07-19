#ifndef PARAM_H
#define PARAM_H

#define MAX_COLORS 17
typedef struct
{
    double positionVarianceX;
    double positionVarianceY;
    double gradientVariance;
    double intensityVariance;
    double processVariance ;
    double stateVariance ;

    double probDetect ;
    double probEnd ;
    double meanNew ;
    double meanFalarms ;

    int maxGHypos ;
    int maxDepth ;
    double minGHypoRatio ;
    double intensityThreshold;
    double maxDistance1 ;
    double maxDistance2 ;
    double maxDistance3 ;

    double pos2velLikelihood;
    double vel2curvLikelihood;

    int endScan ;
    int startA ;
    int startB ;
    int startC ;

} Parameter;

typedef unsigned short* USHORT_PTR;

#endif
