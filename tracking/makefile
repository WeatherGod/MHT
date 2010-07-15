
# Some of the code that this makefile compiles generates
# a few warning messages.  In particular, "_G_errStream defined
# but not used".  These can be ignored.

# --------------------------------------------- macros

# One of the following two versions of C++FLAGS should be
# commented out.  Be sure, also, to comment out the desired
# combination of TSTBUG and DEBUG in except.h.  These
# make much bigger differences in speed than the -g and -O
# flags.

INC = ../mht
#C++ = CC
C++ = g++
#C++FLAGS = -O 
#C++FLAGS = -O -D__SGICC__ -DSDBG
C++FLAGS = -O -I../mht

build = $(C++) $(C++FLAGS) -o $@
touch = touch $@

trackCorners: param.h motionModel.h $(INC)/assign.h \
		trackCorners.o  motionModel.o
	$(build) trackCorners.o motionModel.o \
	-L../mht -lmht -lm

motionModel.o: motionModel.c motionModel.h param.h \
	$(INC)/except.h $(INC)/mdlmht.h $(INC)/matrix.h\
	$(INC)/safeglobal.h $(INC)/mht.h $(INC)/list.h $(INC)/tree.h \
	$(INC)/links.h $(INC)/vector.h 
	$(C++) -c $(C++FLAGS) motionModel.c

trackCorners.o: trackCorners.c motionModel.h $(INC)/except.h 
	$(C++) -c $(C++FLAGS) trackCorners.c


clean:
	rm *.o
