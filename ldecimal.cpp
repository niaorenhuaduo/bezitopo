/******************************************************/
/*                                                    */
/* ldecimal.cpp - lossless decimal representation     */
/*                                                    */
/******************************************************/
/* Copyright 2015 Pierre Abbat.
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

#include <cstdio>
#include <cfloat>
#include <cstring>
#include "ldecimal.h"
using namespace std;

string ldecimal(double x)
{
  double x2;
  int i,iexp,chexp;
  size_t zpos;
  char *dotpos,*epos;
  string ret,s,m,antissa,exponent;
  char buffer[32],fmt[8];
  for (i=DBL_DIG-1,x2=-1/x;x!=x2 && i<DBL_DIG+3;i++)
  {
    sprintf(fmt,"%%.%de",i);
    sprintf(buffer,fmt,x);
    x2=atof(buffer);
  }
  dotpos=strchr(buffer,'.');
  epos=strchr(buffer,'e');
  if (dotpos&&epos)
  {
    m=string(buffer,dotpos-buffer);
    antissa=string(dotpos+1,epos-dotpos-1);
    exponent=string(epos+1);
    if (m.length()>1)
    {
      s=m.substr(0,1);
      m.erase(0,1);
    }
    iexp=atoi(exponent.c_str());
    zpos=antissa.find_last_not_of('0');
    antissa.erase(zpos+1);
    iexp=stoi(exponent);
    if (iexp<0 && iexp>-5)
    {
      antissa=m+antissa;
      m="";
      iexp++;
    }
    if (iexp>0)
    {
      chexp=iexp;
      if (chexp>antissa.length())
	chexp=antissa.length();
      m+=antissa.substr(0,chexp);
      antissa.erase(0,chexp);
      iexp-=chexp;
    }
    while (iexp>-5 && iexp<0 && m.length()==0)
    {
      antissa="0"+antissa;
      iexp++;
    }
    while (iexp<3 && iexp>0 && antissa.length()==0)
    {
      m+='0';
      iexp--;
    }
    sprintf(buffer,"%d",iexp);
    exponent=buffer;
    ret=s+m;
    if (antissa.length())
      ret+='.'+antissa;
    if (iexp)
      ret+='e'+exponent;
  }
  else
    ret=buffer;
  return ret;
}
