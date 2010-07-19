#ifndef CORNER_H
#define CORNER_H

#include "list.h"		// for DLISTnode

typedef unsigned short USHORT;

/*-----------------------------------------------*
 * Data structure for storing input corner data
 *-----------------------------------------------*/


class CORNER:public DLISTnode
{
public:
    double x,y;
    USHORT i1,i2,i3,i4,i5,i6,i7,i8,i9,i10,i11,i12,i13,i14,i15,i16,i17,i18,i19,i20;
    USHORT i21,i22,i23,i24,i25;
    int frameNo;
    CORNER(int a, int b, USHORT I1, USHORT I2, USHORT I3, USHORT I4,
           USHORT I5, USHORT I6, USHORT I7, USHORT I8, USHORT I9, USHORT I10,
           USHORT I11, USHORT I12, USHORT I13, USHORT I14, USHORT I15, USHORT I16,
           USHORT I17, USHORT I18, USHORT I19, USHORT I20, USHORT I21, USHORT I22,
           USHORT I23, USHORT I24, USHORT I25, int f):
        DLISTnode(),
        x(double(a)),y(double(b)),i1(I1),i2(I2),i3(I3),i4(I4),i5(I5),i6(I6),i7(I7),i8(I8),
        i9(I9),i10(I10),i11(I11),i12(I12),i13(I13),i14(I14),i15(I15),i16(I16),
        i17(I17),i18(I18),i19(I19),i20(I20),i21(I21),i22(I22),i23(I23),i24(I24),i25(I25),
        frameNo(f)
    {
    }

protected:
    MEMBERS_FOR_DLISTnode(CORNER)

};


class CORNERLIST: public DLISTnode
{
public:
    int ncorners;
    std::list<CORNER> list;
    CORNERLIST(int npts):
        ncorners(npts), list(), DLISTnode()
    {
    }

protected:
    MEMBERS_FOR_DLISTnode(CORNERLIST)
};



#endif
