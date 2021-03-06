/******************************************************/
/*                                                    */
/* angle.cpp - angles as binary fractions of rotation */
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

#include <cstring>
#include <cstdio>
#include <vector>
#include "angle.h"
using namespace std;

double sqr(double x)
{
  return x*x;
}

double sin(int angle)
{return sinl(angle*M_PIl/1073741824.);
 }

double cos(int angle)
{return cosl(angle*M_PIl/1073741824.);
 }

double sinhalf(int angle)
{return sinl(angle*M_PIl/2147483648.);
 }

double coshalf(int angle)
{return cosl(angle*M_PIl/2147483648.);
 }

double tanhalf(int angle)
{return tanl(angle*M_PIl/2147483648.);
 }

double cosquarter(int angle)
{return cosl(angle*M_PIl/4294967296.);
 }

double tanquarter(int angle)
{return tanl(angle*M_PIl/4294967296.);
 }

int atan2i(double y,double x)
{
  return rint(atan2(y,x)/M_PIl*1073741824.);
}

int atan2i(xy vect)
{
  return atan2i(vect.north(),vect.east());
}

int twiceasini(double x)
{
  return rint(asin(x)/M_PIl*2147483648.);
}

xy cossin(double angle)
{
  return xy(cos(angle),sin(angle));
}

xy cossin(int angle)
{
  return xy(cos(angle),sin(angle));
}

xy cossinhalf(int angle)
{
  return xy(coshalf(angle),sinhalf(angle));
}

int foldangle(int angle)
{
  if (((unsigned)angle>>30)%3)
    angle^=0x80000000;
  return angle;
}

bool isinsector(int angle,int sectors)
/* Quick check for ranges of angles. angle is in [0°,720°), so sectors
 * is normally an unsigned multiple of 65537. Bit 0 is [0,22.5°), bit 1
 * is [22.5°,45°), etc.
 */
{
  return ((sectors>>((unsigned)angle>>27))&1);
}

double bintorot(int angle)
{
  return angle/2147483648.;
}

double bintogon(int angle)
{
  return bintorot(angle)*400;
}

double bintodeg(int angle)
{
  return bintorot(angle)*360;
}

double bintomin(int angle)
{
  return bintorot(angle)*21600;
}

double bintosec(int angle)
{
  return bintorot(angle)*1296000;
}

double bintorad(int angle)
{
  return bintorot(angle)*M_PIl*2;
}

int rottobin(double angle)
{
  double iprt=0,fprt;
  fprt=2*modf(angle/2,&iprt);
  if (fprt>=1)
    fprt-=2;
  if (fprt<-1)
    fprt+=2;
  return lrint(2147483648.*fprt);
}

int degtobin(double angle)
{
  return rottobin(angle/360);
}

int mintobin(double angle)
{
  return rottobin(angle/21600);
}

int sectobin(double angle)
{
  return rottobin(angle/1296000);
}

int gontobin(double angle)
{
  return rottobin(angle/400);
}

int radtobin(double angle)
{
  return rottobin(angle/M_PIl/2);
}

double radtodeg(double angle)
{
  return angle*180/M_PIl;
}

double degtorad(double angle)
{
  return angle/180*M_PIl;
}

double radtogon(double angle)
{
  return angle*200/M_PIl;
}

double gontorad(double angle)
{
  return angle/200*M_PIl;
}

string radtoangle(double angle,int unitp)
{
  double angmult,prec;
  string ret,unitsign;
  char digit[8];
  int i,base,sign,dig;
  if (!compatible_units(unitp,ANGLE))
    throw badunits;
  base=unitp&0xf0;
  switch (base)
  {
    case 0:
      base=10;
      break;
    case 16:
      base=2;
      break;
    case 32:
      base=60;
      break;
    default:
      base=0;
  }
  switch (unitp&0xffffff00)
  {
    case DEGREE:
      angmult=radtodeg(angle);
      unitsign="°";
      break;
    case GON:
      angmult=radtogon(angle);
      break;
    case RADIAN:
      angmult=angle;
      break;
    default:
      throw badunits;
  }
  angmult=rint((prec=precision(unitp))*angmult);
  sign=1;
  if (angmult<0)
  {
    angmult=-angmult;
    sign=-1;
  }
  for (;base>10 && (int)prec%59>1;prec/=10)
  {
    dig=angmult-10*trunc(angmult/10);
    angmult=trunc(angmult/10);
    sprintf(digit,"%01d",dig);
    ret=digit+ret;
  }
  if (ret.length())
    ret="."+ret;
  for (;prec>1;prec/=base)
  {
    dig=angmult-base*trunc(angmult/base);
    angmult=trunc(angmult/base);
    sprintf(digit,(base>10)?"%02d":"%01d",dig);
    if (base>10)
      if (ret.substr(0,1)==".")
	ret+=(prec>60)?"″":"′";
      else
	strcat(digit,(prec>60)?"″":"′");
    ret=digit+ret;
  }
  if (base>10)
    ret=unitsign+ret;
  else
    ret="."+ret+unitsign;
  sprintf(digit,"%.0f",angmult);
  ret=digit+ret;
  if (sign<0)
    ret="-"+ret;
  return ret;
}

string bintoangle(int angle,int unitp)
{
  return radtoangle(bintorad(angle),unitp);
}

double parseangle(string angstr,int unitp)
{
  double angmult=0,prec,angle;
  int i,ulen;
  bool point,six;
  string uchar;
  for (point=six=i=0,prec=1;i<angstr.length();i++)
    switch (angstr[i]&0xc0)
    {
      case 0: // digits, point, and hyphen
	if (angstr[i]=='-' || angstr[i]=='\'' || angstr[i]=='"')
	{
	  point=six=true;
	  unitp=DEGREE;
	}
	if (angstr[i]=='.')
	  point=true;
	if (isdigit(angstr[i]))
	{
	  angmult=angmult*(six?6:10)+angstr[i]-'0';
	  if (point)
	    prec*=six?6:10;
	  six=false;
	}
	break;
      case 0x40: // letters
	if (tolower(angstr[i])=='g')
	  unitp=GON;
	break;
      case 0x80: // subsequent bytes of UTF-8: ignore
	break;
      case 0xc0: // first byte of UTF-8: check for degree sign
	ulen=angstr[i]&0xff;
	ulen=(ulen>=0x80)+(ulen>=0xc0)+(ulen>=0xe0)+(ulen>=0xf0)+(ulen>=0xf8)+(ulen>=0xfc);
	uchar=angstr.substr(i,ulen);
	if (uchar=="°" || uchar=="′" || uchar=="″")
	{
	  point=six=true;
	  unitp=DEGREE;
	}
	break;
    }
  switch (unitp&0xffffff00)
  {
    case DEGREE:
      angle=degtorad(angmult/prec);
      break;
    case GON:
      angle=gontorad(angmult/prec);
      break;
    case RADIAN:
      angle=angmult/prec;
      break;
    default:
      throw badunits;
  }
  return angle;
}

int parseiangle(string angstr,int unitp)
{
  return radtobin(parseangle(angstr,unitp));
}

int parseazimuth(string angstr,int unitp)
{
  return DEG90-parseiangle(angstr,unitp);
}

int parsesignedangle(string angstr,int unitp)
{
  int sign=1;
  if (angstr[0]=='-')
  {
    angstr.erase(0,1);
    sign=-1;
  }
  return parseiangle(angstr,unitp)*sign;
}

int parsebearing(string angstr,int unitp)
{
  int ns,ew,quadrant=-1,angle;
  if (angstr.length())
  {
    ns=tolower(angstr[0]);
    angstr.erase(0,1);
  }
  else
    ns=0;
  if (angstr.length())
  {
    ew=tolower(angstr[angstr.length()-1]);
    angstr.erase(angstr.length()-1,1);
  }
  else
    ew=0;
  if (ns=='n' && ew=='e')
    quadrant=1;
  if (ns=='s' && ew=='e')
    quadrant=2;
  if (ns=='s' && ew=='w')
    quadrant=3;
  if (ns=='n' && ew=='w')
    quadrant=4;
  angle=parseazimuth(angstr,unitp);
  switch (quadrant)
  {
    case 2:
      angle=-angle;
      break;
    case 3:
      angle-=0x40000000;
      break;
    case 4:
      angle=0x40000000-angle;
      break;
    case -1:
      throw badunits;
  }
  return angle;
}
