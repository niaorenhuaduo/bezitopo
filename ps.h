/******************************************************/
/*                                                    */
/* ps.h - PostScript output                           */
/*                                                    */
/******************************************************/
#include <string>
#include "bezier3d.h"

extern FILE *psfile;
extern int orientation;
void psprolog();
void startpage();
void endpage();
void dot(xy pnt,std::string comment="");
void circle(xy pnt,double radius);
void line(edge lin,int num,bool colorfibaster,bool directed=false);
void pstrailer();
void psopen(const char * psfname);
void psclose();
void line2p(xy pnt1,xy pnt2);
void spline(bezier3d spl);
void widen(double factor);
void setcolor(double r,double g,double b);
void setscale(double minx,double miny,double maxx,double maxy,int ori=0);
void pswrite(xy pnt,std::string text);
