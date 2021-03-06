/******************************************************/
/*                                                    */
/* brent.cpp - Brent's root-finding method            */
/*                                                    */
/******************************************************/
/* Copyright 2016 Pierre Abbat.
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
#include <utility>
#include <iostream>
#include <iomanip>
#include <cmath>
#include "cogo.h"
#include "ldecimal.h"
#include "brent.h"

using namespace std;

/* Input:
 * 9s trit is the sign of the contrapoint,
 * 3s trit is the sign of the new point,
 * 1s trit is the sign of the old point.
 * Output:
 * Bits 0 and 1:
 * 0: done
 * 1: new point becomes old point
 * 2: new point becomes contrapoint
 * 3: should never happen.
 * Bit 2: true if new point should be moved one ulp toward old point, if s==x.
 */
char sidetable[27]=
{
  3,7,6, 3,0,0, 3,3,1,
  3,3,7, 3,0,3, 7,3,3,
  1,3,3, 0,0,3, 6,7,3
};

double invquad(double x0,double y0,double x1,double y1,double x2,double y2)
{
  double z0,z1,z2,offx;
  z0=x0;
  z1=x1;
  z2=x2;
  if (z0>z1)
    swap(z0,z1);
  if (z1>z2)
    swap(z1,z2);
  if (z0>z1)
    swap(z0,z1);
  if (z0<0 && z2>0)
    offx=0;
  if (z0>=0)
    offx=z0;
  if (z2<=0)
    offx=z2;
  x0-=offx;
  x1-=offx;
  x2-=offx;
  z0=x0*y1*y2/(y0-y1)/(y0-y2);
  z1=x1*y2*y0/(y1-y2)/(y1-y0);
  z2=x2*y0*y1/(y2-y0)/(y2-y1);
  return (z0+z1+z2)+offx;
}

bool brent::between(double s)
{
  double g=(3*a+b)/4;
  return (g<s && s<b) || (b<s && s<g);
}

double brent::init(double x0,double y0,double x1,double y1,bool intmode)
{
  imode=intmode;
  debug=false;
  if (fabs(y0)>fabs(y1))
  {
    c=a=x0;
    fc=fa=y0;
    b=x1;
    fb=y1;
  }
  else
  {
    c=a=x1;
    fc=fa=y1;
    b=x0;
    fb=y0;
  }
  mflag=false;
  side=1;
  x=b-fb*(a-b)/(fa-fb);
  if (imode)
    x=rint(x);
  if (!between(x))
  {
    x=(a+b)/2;
    mflag=true;
  }
  if (imode)
    x=rint(x);
  if ((y0>0 && y1>0) || (y0<0 && y1<0))
  {
    x=NAN;
    side=3;
  }
  return x;
}

double brent::step(double y)
{
  double s,bsave=b,asave=a;
  bool iq;
  side=sidetable[9*sign(fa)+3*sign(y)+sign(fb)+13];
  if ((side&3)%3)
  {
    d=c;
    c=b;
    fd=fc;
    fc=fb;
  }
  switch (side&3)
  {
    case 0:
      s=x;
      break;
    case 1:
      b=x;
      fb=y;
      break;
    case 2:
      a=x;
      fa=y;
      break;
    case 3:
      s=NAN;
  }
  if (debug)
    cout<<"side="<<side<<endl;
  if ((side&3)%3)
  {
    if (fabs(fb)>fabs(fa))
    {
      swap(fa,fb);
      swap(a,b);
    }
  }
  if (fa==fb || fb==fc || fc==fa)
  {
    s=b-fb*(b-a)/(fb-fa);
    iq=false;
  }
  else
  {
    s=invquad(a,fa,b,fb,c,fc);
    iq=true;
  }
  if (imode)
    s=rint(s);
  if ((side&3)%3 && x==s)
  {
    if (debug)
      cout<<"Same as last time"<<endl;
    if (imode)
    {
      if (bsave<x)
        side^=4;
      if (side&4)
        s++;
      else
        s--;
    }
    else
      s=nextafter(s,(side&4)?bsave:asave);
  }
  if (debug)
  {
    cout<<setw(23)<<ldecimal(a)<<setw(23)<<ldecimal(b)<<setw(23)<<ldecimal(c)<<' '<<iq<<endl;
    cout<<setw(23)<<ldecimal(fa)<<setw(23)<<ldecimal(fb)<<setw(23)<<ldecimal(fc)<<endl;
    cout<<"s="<<ldecimal(s);
  }
  if (between(s) && fabs(s-b)<fabs(mflag?b-c:c-d)/2)
    mflag=false;
  else
  {
    mflag=true;
    s=(a+b)/2;
  }
  if (imode)
    s=rint(s);
  if (debug)
    cout<<' '<<ldecimal(s)<<endl;
  if (mflag && (s==a || s==b)) // interval [a,b] is too small to bisect, we're done
  {
    s=b;
    side=0;
  }
  if ((side&3)%3)
    x=s;
  return s;
}
