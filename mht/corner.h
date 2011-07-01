#ifndef CORNER_H
#define CORNER_H

#include "list.h"		// for DLISTnode
#include <vector>		// for std::vector<>

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
    std::vector<float> m_int;

public:
    Texture_t() :
        m_int(25, 0)
    {
    }


    Texture_t(float I1, float I2, float I3, float I4, float I5,
              float I6, float I7, float I8, float I9, float I10,
              float I11, float I12, float I13, float I14, float I15,
              float I16, float I17, float I18, float I19, float I20,
              float I21, float I22, float I23, float I24, float I25):
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

    float& operator[](const size_t &index)
    {
        return(m_int[index]);
    }

    const float& operator[](const size_t &index) const
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
    size_t m_cornerID;

    CORNER(const double &a, const double &b, const Texture_t &info, const int &f,const size_t &cornerID):
        DLISTnode(),
        x(a),y(b),
        m_textureInfo(info),
        m_frameNo(f),
        m_cornerID(cornerID)
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
