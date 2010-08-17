
def SaveMHTParams(filename, mhtParams) :
    file = open(filename, 'w')

    file.write("""; position varianceX
%(posVarX)f
;
; position varianceY
%(posVarY)f
;
; gradient variance
%(gradVar)f
;
; intensity Variance
%(intVar)f
;
; process variance
%(procVar)f
;
; probability of detection
%(pod)f
;
;Lamda_x
%(lambda_x)f
;
; mean new tracks per scan
%(newTracksPerScan)f
;
; mean false alarms per scan
%(meanFAlarmsPerScan)f
;
; maximum number of global hypotheses per group
%(maxGlobHypoPerGrp)d
;
; maximum depth of track trees
%(maxDepth)d
;
; minimum ratio between likelihoods of worst and best
;   global hypotheses
%(minRatio)f
;
; Intensity Threshold
%(intThresh)f
;
;
; maximum mahalanobis distance for validationi MODEL 1
5.9
;
; maximum mahalanobis distance for validationi MODEL 2 (CONSTANTVEL with z=2)
%(maxDist)f
;
; maximum mahalanobis distance for validationi MODEL 3
12.9
;
;Initial state Variance(Velocity component)
%(initVarVel)f
;
; number of scans to make
%(frameCnt)d
;
; scan at which to start diagnostic A
%(diagA)d
;
; scan at which to start diagnostic B
%(diagB)d
;
; scan at which to start diagnostic C
%(diagC)d
""" % mhtParams)

    file.close()



if __name__ == '__main__' :
    import argparse     # for command-line parsing


    parser = argparse.ArgumentParser("Create a parameter file for MHT")
    parser.add_argument("filename",
                        help="Create a parameter file called FILE",
                        metavar="FILE")


    parser.add_argument("--varx", dest="posVarX", type=float, default=1.0)
    parser.add_argument("--vary", dest="posVarY", type=float, default=1.0)
    parser.add_argument("--vargrad", dest="gradVar", type=float, default=0.01)
    parser.add_argument("--varint", dest="intVar", type=float, default=100.0)
    parser.add_argument("--varproc", dest="procVar", type=float, default=0.5)

    parser.add_argument("--pod", dest="pod", type=float, default=0.9999)
    parser.add_argument("--lambdax", dest="lambda_x", type=float, default=20)
    parser.add_argument("--ntps", dest="newTracksPerScan", type=float, default=0.004)
    parser.add_argument("--mfaps", dest="meanFAlarmsPerScan", type=float, default=0.0002)
    parser.add_argument("--mxghpg", dest="maxGlobHypoPerGrp", type=int, default=300)

    parser.add_argument("--mxdpth", dest="maxDepth", type=int, default=3)
    parser.add_argument("--mnratio", dest="minRatio", type=float, default=0.001)
    parser.add_argument("--intthrsh", dest="intThresh", type=float, default=0.90)
    parser.add_argument("--mxdist", dest="maxDist", type=float, default=5.9)

    parser.add_argument("--varvel", dest="initVarVel", type=float, default=200.0)

    parser.add_argument("--frames", dest="frameCnt", type=int, default=999999)
    parser.add_argument("-A", dest="diagA", type=int, default=999999)
    parser.add_argument("-B", dest="diagB", type=int, default=999999)
    parser.add_argument("-C", dest="diagC", type=int, default=999999)

    args = parser.parse_args()


    SaveMHTParams(args.filename, args.dict())

