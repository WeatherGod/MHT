#
# Makefile last modified 4/18/95
# Ingemar J. Cox
#

# Some of the code that this makefile compiles generates
# a few warning messages.  In particular, "_G_errStream defined
# but not used".  These can be ignored.

# --------------------------------------------- macros

# One of the following two versions of C++FLAGS should be
# commented out.  Be sure, also, to comment out the desired
# combination of TSTBUG and DEBUG in except.h.  These
# make much bigger differences in speed than the -g and -O
# flags.

C++ = g++
AR = /usr/bin/ar
ARFLAGS = -ruc 
C++FLAGS = -O 
build = $(C++) $(C++FLAGS) -o $@ 
touch = touch $@

libmht.a: apqueue.o assign.o \
	  except.o \
	  links.o list.o \
	  matrix.o mdlmht.o \
	  mht.o mht_group.o \
	  mht_report.o mht_track.o tree.o
	  $(AR) $(ARFLAGS) libmht.a $?
	  @echo lib is now up-to-date

apqueue_test: apqueue_test.o libmht.a
	$(build) apqueue_test.o -L. -lmht -lm

assign_test: assign_test.o bassign.o libmht.a
	$(build) assign_test.o bassign.o -L. -lmht -lm


mdlmht.o: mdlmht.h mht.h except.h safeglobal.h list.h tree.h links.h vector.h mdlmht.c
	$(C++) -c $(C++FLAGS) mdlmht.c

mht.o: mht.h safeglobal.h list.h tree.h links.h vector.h except.h mht.c
	$(C++) -c $(C++FLAGS) mht.c

mht_group.o: mht.h pqueue.h apqueue.h safeglobal.h list.h tree.h \
	links.h vector.h assign.h  except.h mht_group.c
	$(C++) -c $(C++FLAGS) mht_group.c

mht_report.o: mht.h pqueue.h apqueue.h safeglobal.h list.h tree.h \
                links.h vector.h assign.h  except.h mht_report.c
	$(C++) -c $(C++FLAGS) mht_report.c

mht_track.o: mht.h safeglobal.h list.h tree.h links.h vector.h except.h mht_track.c
	$(C++) -c $(C++FLAGS) mht_track.c

apqueue_test.o: apqueue.h except.h apqueue_test.c
	$(C++) -c $(C++FLAGS) apqueue_test.c

apqueue.o: apqueue.h except.h safeglobal.h list.h assign.h vector.h apqueue.c
	$(C++) -c $(C++FLAGS) apqueue.c

assign_test.o: bassign.h except.h assign_test.c
	$(C++) -c $(C++FLAGS) assign_test.c

assign.o: assign.h queue.h except.h vector.h assign.c
	$(C++) -c $(C++FLAGS) assign.c

bassign.o: bassign.h safeglobal.h vector.h assign.h except.h bassign.c
	$(C++) -c $(C++FLAGS) bassign.c

links.o: links.h safeglobal.h list.h except.h links.c
	$(C++) -c $(C++FLAGS) links.c

tree.o: tree.h except.h safeglobal.h list.h tree.c
	$(C++) -c $(C++FLAGS) tree.c

list.o: list.h except.h  safeglobal.h list.c
	$(C++) -c $(C++FLAGS) list.c

matrix.o: matrix.h vector.h safeglobal.h except.h matrix.c
	$(C++) -c $(C++FLAGS) matrix.c

pqueue.o: pqueue.h except.h safeglobal.h pqueue.c
	$(C++) -c $(C++FLAGS) pqueue.c

#mem.o: mem.h except.h  safeglobal.h mem.c
#	$(C++) -c $(C++FLAGS) mem.c

except.o: except.h  safeglobal.h except.c
	$(C++) -c $(C++FLAGS) except.c

normrand.o: normrand.h normrand.c
	$(C++) -c $(C++FLAGS) normrand.c
