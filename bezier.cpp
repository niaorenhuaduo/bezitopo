/********************************************************/
/*                                                      */
/* bezier.cpp - computes functions of a Bézier triangle */
/* For Bézier functions of one variable, see vcurve.cpp.*/
/*                                                      */
/********************************************************/
#include <cstring>
#include "bezier.h"
#include "angle.h"
using namespace std;

triangle::triangle()
{
  a=b=c=NULL;
  aneigh=bneigh=cneigh=NULL;
  memset(ctrl,0,sizeof(ctrl));
}

double triangle::area()
{return area3(*a,*b,*c);
 }

double triangle::perimeter()
{
  return dist(xy(*a),xy(*b))+dist(xy(*c),xy(*a))+dist(xy(*b),xy(*c));
}

double triangle::elevation(xy pnt)
/* Computes the elevation of triangle tri at the point x,y. */
{double p,q,r, // Fraction of distance from a side to opposite corner. p+q+r=1.
        s;   // Area of triangle.
 s=area();
 p=area3(pnt,*b,*c)/s;
 q=area3(*a,pnt,*c)/s;
 r=area3(*a,*b,pnt)/s;
 return q*q*q*b->z+3*q*q*r*ctrl[5]+3*p*q*q*ctrl[2]+
        3*q*r*r*ctrl[6]+6*p*q*r*ctrl[3]+3*p*p*q*ctrl[0]+
        p*p*p*a->z+3*p*p*r*ctrl[1]+3*p*r*r*ctrl[4]+r*r*r*c->z;
 }

bool triangle::in(xy pnt)
{return area3(pnt,*b,*c)>=0 && area3(*a,pnt,*c)>=0 && area3(*a,*b,pnt)>=0;
 }

xy triangle::centroid()
{return (xy(*a)+xy(*b)+xy(*c))/3; //FIXME: check if this affects numerical stability
 }

void triangle::setcentercp()
{
  ctrl[3]=((ctrl[0]+ctrl[1]+ctrl[2]+ctrl[4]+ctrl[5]+ctrl[6])*3-(a->z+b->z+c->z)*2)/12;
}

bool triangle::iscorner(point *v)
{
  return (a==v)||(b==v)||(c==v);
}

void triangle::flatten()
{
  ctrl[0]=(2*a->z+b->z)/3;
  ctrl[1]=(2*a->z+c->z)/3;
  ctrl[2]=(2*b->z+a->z)/3;
  ctrl[3]=(a->z+b->z+c->z)/3;
  ctrl[4]=(2*c->z+a->z)/3;
  ctrl[5]=(2*b->z+c->z)/3;
  ctrl[6]=(2*c->z+b->z)/3;
}

void triangle::setgradient(xy pnt,xy grad)
// Sets the gradient at corner pnt to grad. If pnt is not a corner,
// either sets the nearest corner or does nothing.
{
  int which;
  double crit;
  crit=1/2.0*(1/dist(xy(*a),xy(*b))+1/dist(xy(*c),xy(*a))+1/dist(xy(*b),xy(*c)));
  grad/=3; // control points are 1/3 of the way along sides
  if (dist(pnt,*a)<crit)
  {
    ctrl[0]=a->z+dot(grad,xy(*b)-xy(*a));
    ctrl[1]=a->z+dot(grad,xy(*c)-xy(*a));
  }
  if (dist(pnt,*b)<crit)
  {
    ctrl[5]=b->z+dot(grad,xy(*c)-xy(*b));
    ctrl[2]=b->z+dot(grad,xy(*a)-xy(*b));
  }
  if (dist(pnt,*c)<crit)
  {
    ctrl[4]=c->z+dot(grad,xy(*a)-xy(*c));
    ctrl[6]=c->z+dot(grad,xy(*b)-xy(*c));
  }
}

void triangle::setneighbor(triangle *neigh)
{
  bool sha,shb,shc;
  sha=neigh->iscorner(a);
  shb=neigh->iscorner(b);
  shc=neigh->iscorner(c);
  if (sha&&shb)
    cneigh=neigh;
  if (shb&&shc)
    aneigh=neigh;
  if (shc&&sha)
    bneigh=neigh;
}

triangle *triangle::nexttoward(xy pnt)
// If the point is in the triangle, return the same triangle.
// Else return which triangle to look in next.
// If returns NULL, the point is outside the convex hull.
{double p,q,r;
 p=area3(pnt,*b,*c);
 q=area3(*a,pnt,*c);
 r=area3(*a,*b,pnt);
 if (p>=0 && q>=0 && r>=0)
    return this;
 else if (p<q && p<r)
    return aneigh;
 else if (q<r)
    return bneigh;
 else
    return cneigh;
 }

triangle *triangle::findt(xy pnt,bool clip)
{
  triangle *here,*there;
  here=there=this;
  while (here && !here->in(pnt))
  {
    here=here->nexttoward(pnt);
    if (here)
      there=here;
  }
  return clip?there:here;
}

vector<double> triangle::xsect(int angle,double offset)
/* Where s is the semiperimeter, samples the surface at four points,
 * -3s/2, -s/2, s/2, 3s/2, relative to the centroid, offset by offset.
 * offset is multplied by the semiperimeter and ranges from -1.5 to 1.5.
 */
{
  double s;
  int i;
  vector<double> ret;
  xy along,across,cen;
  s=peri/2;
  cen=centroid();
  along=cossin(angle)*s;
  across=cossin(angle+536870912)*s;
  for (i=-3;i<5;i+=2)
    ret.push_back(elevation(cen+across*offset+along*i*0.5));
  return ret;
}

double deriv0(vector<double> xsect)
{
  return (-xsect[3]+9*xsect[2]+9*xsect[1]-xsect[0])/16;
}

double deriv1(vector<double> xsect)
{
  return (xsect[0]-27*xsect[1]+27*xsect[2]-xsect[3])/24;
}

double deriv2(vector<double> xsect)
{
  return (xsect[3]-xsect[2]-xsect[1]+xsect[0])/2;
}

double deriv3(vector<double> xsect)
{
  return xsect[3]-3*xsect[2]+3*xsect[1]-xsect[0];
}
