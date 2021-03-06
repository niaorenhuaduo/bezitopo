/******************************************************/
/*                                                    */
/* document.cpp - main document class                 */
/*                                                    */
/******************************************************/
/* Copyright 2015,2016 Pierre Abbat.
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
#include "bezitopo.h"
#include "pnezd.h"
#include "document.h"
using namespace std;

void document::copytopopoints(criteria crit)
{
  ptlist::iterator i;
  if (pl.size()<2)
    pl.resize(2);
  pl[1].clear();
  int j;
  bool include;
  for (i=pl[0].points.begin();i!=pl[0].points.end();i++)
  {
    include=false;
    for (j=0;j<crit.size();j++)
      if (i->second.note.find(crit[j].str)!=string::npos)
	include=crit[j].istopo;
    if (include)
      pl[1].addpoint(i->first,i->second);
  }
}

int document::readpnezd(string fname,bool overwrite)
{
  return ::readpnezd(this,fname,overwrite);
}

int document::writepnezd(string fname)
{
  ::writepnezd(this,fname);
}

int document::readpenzd(string fname,bool overwrite)
{
  return ::readpenzd(this,fname,overwrite);
}

int document::writepenzd(string fname)
{
  ::writepenzd(this,fname);
}

void document::addobject(drawobj *obj)
// The drawobj must be created with new; it will be destroyed with delete.
{
  objrec o;
  if (curlayer<0 || curlayer>=layers.size())
  {
    curlayer=0;
    if (layers.size()==0)
    {
      layer l;
      l.colr=WHITE;
      l.visible=true;
      l.name="0";
      layers.push_back(l);
    }
  }
  o.layr=curlayer;
  o.ltype=o.colr=o.thik=SAMECOLOR;
  o.obj=obj;
  objlist.push_back(o);
}

void document::writeXml(ofstream &ofile)
{
  int i;
  ofile<<"<Bezitopo>";
  for (i=0;i<pl.size();i++)
    pl[i].writeXml(ofile);
  ofile<<"</Bezitopo>"<<endl;
}

void document::changeOffset (xyz newOffset)
/* Changes the offset of the document, moving everything so that its coordinates
 * added to the offset do not change. Everything, that is, except the quad index,
 * which will have to be recreated.
 */
{
  int i;
  for (i=0;i<pl.size();i++)
    pl[i].roscat(newOffset,0,1,offset); // FIXME: roscat takes xy;
  for (i=0;i<objlist.size();i++)
    objlist[i].obj->roscat(newOffset,0,1,offset); // the z has to be adjusted elsewise.
  offset=newOffset;
}
