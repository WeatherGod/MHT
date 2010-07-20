#ifndef CORNER_H
#define CORNER_H

#include "list.h"		// for DLISTnode
#include <vector>		// for std::vector<>

typedef unsigned short USHORT;

/*-----------------------------------------------*
 * Data structures for storing input corner data
 *-----------------------------------------------*/
class Texture_t
// Class to "hide-away' texture information
// to make it easier to modify in the future.
// The current list of member variables is
// temporary and only exists for legacy's sake.
{
private:
    std::vector<USHORT> m_int;

public:
    Texture_t() :
        m_int(25, 0)
    {
    }


    Texture_t(USHORT I1, USHORT I2, USHORT I3, USHORT I4, USHORT I5,
              USHORT I6, USHORT I7, USHORT I8, USHORT I9, USHORT I10,
              USHORT I11, USHORT I12, USHORT I13, USHORT I14, USHORT I15,
              USHORT I16, USHORT I17, USHORT I18, USHORT I19, USHORT I20,
              USHORT I21, USHORT I22, USHORT I23, USHORT I24, USHORT I25):
        m_int(25)
    {
        m_int[0] = I1;
        m_int[1] = I2;
        m_int[2] = I3;
        m_int[3] = I4;
        m_int[4] = I5;
        m_int[5] = I6;
        m_int[6] = I7;
        m_int[7] = I8;
        m_int[8] = I9;
        m_int[9] = I10;
        m_int[10] = I11;
        m_int[11] = I12;
        m_int[12] = I13;
        m_int[13] = I14;
        m_int[14] = I15;
        m_int[15] = I16;
        m_int[16] = I17;
        m_int[17] = I18;
        m_int[18] = I19;
        m_int[19] = I20;
        m_int[20] = I21;
        m_int[21] = I22;
        m_int[22] = I23;
        m_int[23] = I24;
        m_int[24] = I25;
    }

    USHORT& operator[](const int &index)
    {
        return(m_int[index]);
    }

    const USHORT& operator[](const int &index) const
    {
        return(m_int[index]);
    }
};

class CORNER:public DLISTnode
{
public:
    double x,y;
    Texture_t m_textureInfo;
    int m_frameNo;

    CORNER(const double &a, const double &b, const Texture_t &info, const int &f):
        DLISTnode(),
        x(a),y(b),
        m_textureInfo(info),
        m_frameNo(f)
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
