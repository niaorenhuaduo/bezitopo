/******************************************************/
/*                                                    */
/* segment.h - 3d line segment                        */
/* base class of arc and spiral                       */
/*                                                    */
/******************************************************/
/* Copyright 2012,2014,2015,2016 Pierre Abbat.
 * This file is part of Bezitopo.
 * 
 * Bezitopo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Bezitopo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bezitopo. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SEGMENT_H
#define SEGMENT_H
#include <cstdlib>
#include <vector>
#include "point.h"
#include "angle.h"
#include "bezier3d.h"
#include "cogo.h"
#include "drawobj.h"
#define START 1
#define END 2

double minquad(double x0,double y0,double x1,double y1,double x2,double y2);

class segment: public drawobj
{
protected:
  xyz start,end;
  double control1,control2;
public:
  segment();
  segment(xyz kra,xyz fam);
  segment(xyz kra,double c1,double c2,xyz fam);
  xyz getstart()
  {
    return start;
  }
  xyz getend()
  {
    return end;
  }
  virtual bool operator==(const segment b) const;
  virtual double length() const;
  std::vector<double> vextrema(bool withends);
  void setslope(int which,double s);
  void setctrl(int which,double el);
  virtual void setdelta(int d,int s=0)
  {
  }
  virtual void setcurvature(double startc,double endc)
  {
  }
  double elev(double along) const;
  double slope(double along);
  double startslope();
  double endslope();
  double contourcept(double e);
  bool crosses(double e)
  {
    return (end.elev()<e)^(start.elev()<e);
  }
  virtual xyz station(double along) const;
  double avgslope()
  {
    return (end.elev()-start.elev())/length();
  }
  double chordlength() const
  {
    return dist(xy(start),xy(end));
  }
  int chordbearing() const
  {
    return atan2i(xy(end)-xy(start));
  }
  virtual int startbearing() const
  {
    return chordbearing();
  }
  virtual int endbearing() const
  {
    return chordbearing();
  }
  virtual int bearing(double along)
  {
    return chordbearing();
  }
  virtual double radius(double along) const
  {
    return INFINITY;
  }
  virtual double curvature(double along) const
  {
    return 0;
  }
  virtual double clothance()
  {
    return 0;
  }
  virtual int getdelta()
  {
    return 0;
  }
  virtual int getdelta2()
  {
    return 0;
  }
  xy center();
  xyz midpoint() const;
  virtual int diffinside(xy pnt)
  {
    return 0;
  }
  virtual double diffarea()
  {
    return 0;
  }
  virtual int in(xy pnt)
  {
    return 0;
  }
  virtual double sthrow()
  {
    return 0;
  }
  double closest(xy topoint,double closesofar=INFINITY,bool offends=false);
  double dirbound(int angle,double boundsofar=INFINITY);
  virtual void split(double along,segment &a,segment &b);
  bezier3d approx3d(double precision);
  friend xy intersection (segment seg1,segment seg2);
  friend inttype intersection_type(segment seg1,segment seg2);
  friend class arc;
  friend class spiralarc;
  virtual void writeXml(std::ofstream &ofile);
};

#endif
