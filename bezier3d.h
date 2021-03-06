/******************************************************/
/*                                                    */
/* bezier3d.h - 3d Bézier splines                     */
/*                                                    */
/******************************************************/
/* Copyright 2014,2015,2016 Pierre Abbat.
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

#ifndef BEZIER3D_H
#define BEZIER3D_H
#include <vector>
#include "point.h"

class bezier3d
{
private:
  std::vector<xyz> controlpoints;
public:
  bezier3d(xyz kra,xyz con1,xyz con2,xyz fam);
  bezier3d(xyz kra,int bear0,double slp0,double slp1,int bear1,xyz fam);
  bezier3d();
  int size() const; // number of Bézier segments
  std::vector<xyz> operator[](int n);
  xyz station(double along);
  friend bezier3d operator+(const bezier3d &l,const bezier3d &r); // concatenates, not adds
  bezier3d& operator+=(const bezier3d &r);
};

double bez3destimate(xy kra,int bear0,double len,int bear1,xy fam);
#endif
