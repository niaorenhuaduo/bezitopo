/******************************************************/
/*                                                    */
/* manysum.cpp - add many numbers                     */
/*                                                    */
/******************************************************/
#include <cfloat>
#include <iostream>
#include <cmath>
#include <vector>
#include "manysum.h"
using namespace std;

void manysum::clear()
{
  bucket.clear();
}

double manysum::total()
{
  map<int,double>::iterator i;
  double t;
  for (t=0,i=bucket.begin();i!=bucket.end();i++)
    t+=i->second;
  return t;
}

void manysum::dump()
{
  map<int,double>::iterator i;
  for (i=bucket.begin();i!=bucket.end();i++)
    cout<<i->first<<' '<<i->second<<endl;
}

void manysum::prune()
{
  vector<int> delenda;
  int j;
  map<int,double>::iterator i;
  for (i=bucket.begin();i!=bucket.end();i++)
    if (i->second==0)
      delenda.push_back(i->first);
  for (j=0;j<delenda.size();j++)
    bucket.erase(delenda[j]);
}

manysum& manysum::operator+=(double x)
{
  int i,j;
  while (x!=0)
  {
    frexp(x,&i);
    bucket[i]+=x;
    frexp(bucket[i],&j);
    if (j>i)
    {
      x=bucket[i];
      bucket[i]=0;
    }
    else
      x=0;
  }
}
