#ifndef DoubleCoordinateNitrate_H
#define DoubleCoordinateNitrate_H

namespace rootmap
{

// rvh 20020710 made this a derived struct (like a derived class)
//              of DoubleCoordinate, rather than a different struct altogether
struct DoubleCoordinateNitrate : public DoubleCoordinate
{
    double_cs s;
}; //vmd 10/07/02 add a size dimension to double_cs coordinate

//typedef struct DoubleCoordinateNitrate DoubleIndexNitrate; //rvh 20020710
typedef struct DoubleCoordinateNitrate DoubleCoordinateNitrate; //vmd 10/07/02
typedef DoubleCoordinateNitrate DoubleCoordinateArrayNitrate[7];

} // namespace rootmap

#endif // #ifndef DoubleCoordinateNitrate_H
