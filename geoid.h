/******************************************************/
/*                                                    */
/* geoid.h - geoidal undulation                       */
/*                                                    */
/******************************************************/
/* The native format is a set of six quadtrees, one each for Arctic, Antarctic,
 * Benin, Galápagos, Howland, and Bengal. Each leaf square is a set of six
 * values for the constant, linear, and quadratic components.
 */
#ifndef GEOID_H
#define GEOID_H
#include <vector>
#include <array>
#include "xyz.h"
#include "ellipsoid.h"

#define BOL_EARTH 0
#define BOL_UNDULATION 0
#define BOL_VARLENGTH 1

struct vball // so called because a sphere so marked looks like a volleyball
{
  int face;
  double x,y;
  vball();
  vball(int f,xy p);
  xy getxy();
};

vball encodedir(xyz dir);
xyz decodedir(vball code);

class geoquad
{
public:
  union
  {
    geoquad *sub[4];
    int und[6];
  };
  xy center;
  float scale; // always a power of 2
  int face;
  std::vector<xy> nans,nums;
  bool subdivided();
  bool isnan();
  geoquad();
  ~geoquad();
  vball vcenter();
  void clear();
  void subdivide();
  bool in(xy pnt); // does not check whether it's on the right face
  bool in(vball pnt); // does check
  double undulation(double x,double y);
  xyz centeronearth();
  double length(); // length, width, and apxarea are accurate only for small squares
  double width(); // and ignore the orientation of the square relative to the
  double apxarea(); // displacement from the center of the face.
  double angarea();
  double area();
  int isfull(); // -1 if empty, 0 if partly full or unknown, 1 if full
  std::array<unsigned,2> hash();
  void writeBinary(std::ostream &ofile,int nesting=0);
};

class cubemap
{
public:
  geoquad faces[6]; // note off-by-one: faces[0] is face 1, the Benin face
  double scale; // always a power of 2
  cubemap();
  double undulation(int lat,int lon);
  double undulation(xyz dir);
  std::array<unsigned,2> hash();
  void writeBinary(std::ostream &ofile);
};

struct geoheader
{
  std::array<unsigned,2> hash;
  int planet;
  int dataType;
  int encoding;
  int ncomponents;
  int logScale;
  double tolerance;
  double sublimit;
  double spacing;
  std::vector<std::string> namesFormats;
  void writeBinary(std::ostream &ofile);
};

extern cubemap cube;
#endif
