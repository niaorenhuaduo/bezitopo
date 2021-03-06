/******************************************************/
/*                                                    */
/* geoidboundary.h - geoid boundaries                 */
/*                                                    */
/******************************************************/
/* Copyright 2016,2017 Pierre Abbat.
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
#include <vector>

bool operator==(const vball &a,const vball &b);
bool sameEdge(const vball &a,const vball &b);

struct vsegment
{
  vball start,end;
};

int splitLevel(double coord);
int splitLevel(vball v);
int splitLevel(vsegment v);

class g1boundary
{
private:
  std::vector<vball> bdy;
public:
  bool isempty();
  void push_back(vball v);
  vsegment seg(int n);
  std::vector<int> segmentsAtLevel(int l);
  std::vector<int> nullSegments();
  void positionSegment(int n);
  void splice(g1boundary &b);
  void split(int n,g1boundary &b);
  void splice(int m,g1boundary &b,int n);
  void split(int m,int n,g1boundary &b);
};

bool overlap(vsegment a,vsegment b);

class gboundary
{
private:
  std::vector<g1boundary> bdy;
public:
  void consolidate(int l);
  void splitoff(int l);
  void deleteNullSegments();
  void deleteEmpty();
  friend gboundary operator||(const gboundary &l,const gboundary &r);
  friend gboundary operator+(const gboundary &l,const gboundary &r);
};
