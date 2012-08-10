/******************************************************/
/*                                                    */
/* bezitopo.cpp - main program                        */
/*                                                    */
/******************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "point.h"
#include "cogo.h"
#include "bezitopo.h"
#include "test.h"
#include "tin.h"
#include "measure.h"
#include "pnezd.h"
#include "angle.h"

using namespace std;

int main(int argc, char *argv[])
{int i,j,itype;
 randfil=fopen("/dev/urandom","rb");
 set_length_unit(SURVEYFOOT);
 if (readpnezd("topo0.asc")<0)
   readpnezd("../topo0.asc");
 rotate(2);
 /*for (i=0;i<9;i++)
     for (j=0;j<=i;j++)
         {itype=intersection_type(points[i],points[i+1],points[j],points[j+1]);
          e=intersection(points[i],points[i+1],points[j],points[j+1]);
          printf("i=%d j=%d Intersection type %d\nIntersection is (%f,%f)\n",i,j,itype,e.east(),e.north());
          }*/
 maketin();
 fclose(randfil);
 return EXIT_SUCCESS;
 }
