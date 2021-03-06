/******************************************************/
/*                                                    */
/* convertgeoid.cpp - convert geoidal undulation data */
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
#include <iostream>
#include <ctime>
#include "config.h"
#include "geoid.h"
#include "sourcegeoid.h"
#include "refinegeoid.h"
#include "document.h"
#include "raster.h"
#include "hlattice.h"
#include "relprime.h"
#include "ps.h"
#include "manysum.h"
#include "latlong.h"
#include "halton.h"
using namespace std;

document doc;
vector<geoformat> formatlist;
int verbosity=1;
bool helporversion=false;
int qsz=4;
vector<string> infilebasenames;
string outfilename;

struct option
{
  char shopt;
  string lopt;
  string args;
  string desc;
};

struct token
{
  int optnum;
  string nonopt;
};

vector<option> options(
  {
    {'h',"help","","Help using the program"},
    {'\0',"version","","Output version number"},
    {'v',"verbose","","Increase verbosity"},
    {'f',"format","e.g. ngs","Format of the geoid file"},
    {'o',"output","filename","Output geoid file"},
    {'c',"circle","lat long radius","Excerpt a region"},
    {'q',"quadsample","n 4-16","Geoquad sampling fineness"}
  });

vector<token> cmdline;
geoid outputgeoid;

void outhelp()
{
  int i,j;
  for (i=0;i<options.size();i++)
  {
    cout<<(options[i].shopt?options[i].shopt:' ')<<' ';
    cout<<options[i].lopt;
    for (j=options[i].lopt.length();j<14;j++)
      cout<<' ';
    cout<<options[i].args;
    for (j=options[i].args.length();j<20;j++)
      cout<<' ';
    cout<<options[i].desc<<endl;
  }
}

/* The factors used when setting the six components of a geoquad are
 * 0: 1/1
 * 1: 256/85
 * 2: 256/85
 * 3: 589824/51409
 * 4: 65536/7225
 * 5: 589824/51409
 */

xy unfold(vball pnt)
{
  xy ret(-2,-2);
  switch (pnt.face)
  {
    case 1:
      ret=xy(pnt.x,pnt.y);
      break;
    case 2:
      ret=xy(2-pnt.y,pnt.x);
      break;
    case 3:
      ret=xy(pnt.y,2-pnt.x);
      break;
    case 4:
      ret=xy(-pnt.y,pnt.x-2);
      break;
    case 5:
      ret=xy(-2-pnt.y,pnt.x);
      break;
    case 6:
      ret=xy(4-pnt.x,-pnt.y);
      break;
  }
  return ret;
}

void plotcenter(PostScript &ps,geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotcenter(ps,*quad.sub[i]);
  else
    ps.dot(unfold(quad.vcenter()));
}

void plotcenters(PostScript &ps)
{
  ps.setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotcenter(ps,cube.faces[i]);
}

void plotinter(PostScript &ps,geoquad &quad)
{
  int i;
  if (quad.subdivided())
    for (i=0;i<4;i++)
      plotinter(ps,*quad.sub[i]);
  else
  {
    ps.setcolor(0,0,1);
    for (i=0;i<quad.nums.size();i++)
      ps.dot(unfold(vball(quad.face,quad.nums[i])));
    ps.setcolor(1,0,0);
    for (i=0;i<quad.nans.size();i++)
      ps.dot(unfold(vball(quad.face,quad.nans[i])));
  }
}

void plotinters(PostScript &ps)
{
  ps.setscale(-3,-3,3,5,DEG90);
  int i;
  for (i=0;i<6;i++)
    plotinter(ps,cube.faces[i]);
}

void outund(string loc,int lat,int lon)
{
  int i;
  cout<<"Undulation in "<<loc<<" is"<<endl;
  for (i=0;i<geo.size();i++)
    cout<<i<<": "<<geo[i].elev(lat,lon)<<endl;
  cout<<"c: "<<cube.undulation(lat,lon)<<endl;
}

void test360seam()
/* Tests for continuity of the derivative across the seam in a geoid file
 * that covers the whole earth.
 */
{
  int i,j,seamlong,bigpos,creasecount=0;
  double bigdiff;
  vector<double> elevs,diff2;
  for (i=0;i<geo.size();i++)
    if (geo[i].glat && geo[i].glat->wbd-geo[i].glat->ebd==DEG360)
      seamlong=geo[i].glat->wbd;
  for (i=-60*DEG1;i<=60*DEG1;i+=DEG1)
  {
    elevs.clear();
    diff2.clear();
    for (j=-561000;j<=561000;j+=33000) // apx -5'40" to 5'40" by 20"
      elevs.push_back(avgelev(Sphere.geoc(i,j+seamlong,0)));
    for (j=1;j<elevs.size()-1;j++)
      diff2.push_back(elevs[j-1]+elevs[j+1]-2*elevs[j]);
    for (bigdiff=bigpos=j=0;j<diff2.size();j++)
      if (fabs(diff2[j])>bigdiff)
      {
        bigpos=j;
        bigdiff=fabs(diff2[j]);
      }
    //for (j=0;j<diff2.size();j++)
    //  cout<<((bigpos==j)?'*':' ');
    //cout<<endl;
    if (bigpos==diff2.size()/2)
      creasecount++;
  }
  cout<<"creasecount "<<creasecount<<endl;
  if (creasecount>40)
    cout<<"The seam of the whole-earth geoid file is not smoothed properly."<<endl;
}

void initformat(string cmd,string ext,string desc,int readfunc(geoid&,string),void writefunc(geoid&,string))
{
  geoformat gf;
  gf.cmd=cmd;
  gf.ext=ext;
  gf.desc=desc;
  gf.readfunc=readfunc;
  gf.writefunc=writefunc;
  formatlist.push_back(gf);
}

int readgeoid(string filename)
/* Returns 0, 1, or 2 like the geoformat.readfunc functions.
 * If it returns 2, it pushes a geoid back onto geo
 * and the filename and format onto hdr.
 */
{
  int i,ret;
  size_t extpos;
  string basename;
  geoid gd;
  for (i=0,ret=1;i<formatlist.size() && ret==1;i++)
    ret=formatlist[i].readfunc(gd,filename);
  i--;
  if (ret==2)
  {
    geo.push_back(gd);
    outputgeoid.ghdr->namesFormats.push_back(filename);
    outputgeoid.ghdr->namesFormats.push_back(formatlist[i].cmd);
    cout<<"Read "<<filename<<" in format "<<formatlist[i].cmd<<endl;
    /*if (formatlist[i].cmd=="bol")
    {
      ofstream gdump("gd.dump");
      geo.back().cmap->dump(gdump);
    }*/
    extpos=filename.rfind("."+formatlist[i].ext);
    if (extpos+formatlist[i].ext.length()+1==filename.length())
      basename=filename.substr(0,extpos);
    else
      basename=filename;
    //cout<<"basename="<<basename<<endl;
    infilebasenames.push_back(basename);
  }
  if (ret==1)
    cout<<filename<<" does not appear to be a geoid file."<<endl;
  if (ret==0)
    cout<<"Could not read "<<filename<<endl;
  return ret;
}

void frontformat(string fmt)
{
  int i;
  for (i=formatlist.size()-1;i>0;i--)
    if (formatlist[i].cmd==fmt)
      swap(formatlist[i],formatlist[i-1]);
  if (fmt!=formatlist[0].cmd)
    cout<<"Unrecognized format: "<<fmt<<endl;
}

histogram errorspread()
{
  histogram ret(-0.001,0.001);
  halton hal;
  latlong ll;
  xyz loc;
  int i;
  double origelev,cvtelev;
  for (i=0;i*ret.nbars()<16777216;i++)
  {
    ll=hal.onearth();
    loc=Sphere.geoc(ll,0);
    cvtelev=outputgeoid.elev(loc);
    origelev=avgelev(loc);
    if (isfinite(cvtelev) && isfinite(origelev))
      ret<<(cvtelev-origelev);
  }
  return ret;
}

void argpass1(int argc, char *argv[])
{
  int i,j;
  token tok;
  for (i=1;i<argc;i++)
  {
    tok.optnum=-1;
    tok.nonopt=argv[i];
    for (j=0;j<options.size();j++)
    {
      if (options[j].shopt && argv[i]==string("-")+options[j].shopt)
      {
	tok.optnum=j;
	tok.nonopt="";
	cmdline.push_back(tok);
      }
      else if (options[j].lopt.length() && argv[i]=="--"+options[j].lopt)
      {
	tok.optnum=j;
	tok.nonopt="";
	cmdline.push_back(tok);
      }
    }
    if (tok.optnum<0)
      cmdline.push_back(tok);
  }
}

void argpass2()
{
  int i,j;
  string centerstr;
  latlong ll;
  double radius;
  smallcircle cir;
  for (i=0;i<cmdline.size();i++)
    switch (cmdline[i].optnum)
    {
      case 0:
	helporversion=true;
	outhelp();
	break;
      case 1:
	helporversion=true;
	cout<<"Convertgeoid, part of Bezitopo version "<<VERSION<<" © 2016 Pierre Abbat\n"
	<<"Distributed under GPL v3 or later. This is free software with no warranty."<<endl;
	break;
      case 2:
	++verbosity;
	cout<<"verbosity "<<verbosity<<endl;
	break;
      case 3:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
	  frontformat(cmdline[i].nonopt);
	}
	else
	{
	  cout<<"-f / --format requires an argument, one of:"<<endl;
	  for (j=0;j<formatlist.size();j++)
	    cout<<formatlist[j].cmd<<'\t'<<formatlist[j].desc<<endl;
	}
	break;
      case 4:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          outfilename=cmdline[i].nonopt;
	  //cout<<"Geoid will be output to "<<cmdline[i].nonopt<<endl;
	}
	else
	{
	  cout<<"-o / --output requires an argument, a filename"<<endl;
	}
	break;
      case 5:
	centerstr="";
	radius=NAN;
	ll=parselatlong(centerstr,DEGREE); // sets it to NAN,NAN
	for (j=1;ll.valid()<2 && i+j<cmdline.size() && cmdline[i+j].optnum<0;j++)
	{
	  centerstr+=cmdline[i+j].nonopt+" ";
	  ll=parselatlong(centerstr,DEGREE);
	}
	i+=j;
	if (ll.valid()==2 && i<cmdline.size() && cmdline[i].optnum<0)
	  radius=stod(cmdline[i++].nonopt)*1000;
	if (radius>0 && radius<=1e7)
	{
	  cout<<"Excerpt will be centered on "<<radtodeg(ll.lat)<<','<<radtodeg(ll.lon)<<" with radius "<<radius<<endl;
	  cir.center=Sphere.geoc(ll,0);
	  cir.setradius(radtobin(radius/Sphere.avgradius()));
	  excerptcircles.push_back(cir);
	}
	else
	{
	  cout<<"-c / --circle requires two arguments, a center (latitude/longitude) and a radius"<<endl;
	  cout<<"radius is 10000 max, in kilometers"<<endl;
	}
	i--;
	break;
      case 6:
	if (i+1<cmdline.size() && cmdline[i+1].optnum<0)
	{
	  i++;
          qsz=stod(cmdline[i].nonopt);
	}
	else
	{
	  cout<<"-q / --quadsample requires an argument, an integer from 4 to 16"<<endl;
	}
	break;
        
      default:
	if (!helporversion)
	  readgeoid(cmdline[i].nonopt);
    }
}
	
/* Command line syntax:
 * -f format		Puts format first on the list of formats to try.
 * -o file		Sets the output filename. The file is written after
 * 			all input files are read.
 * -c lat long radius	Excerpts a circle from the geoid file.
 * Outputting the KML file is automatic; there is no option for it.
 * Arguments not tagged by an option are input files.
 * 
 * Example:
 * convertgeoid -f ngs g2012bu0.bin -c 38N99W 150 -f gsf -o Macksville.gsf
 * Reads the Lower 48 file in NGS format, outputs an excerpt called Macksville.gsf
 * containing a circle of radius 150 km centered at 38N99W in GSF format,
 * and outputs the boundary to file Macksville.gsf.kml .
 */

int main(int argc, char *argv[])
{
  ofstream ofile;
  int i;
  vball v;
  PostScript ps;
  histogram errorHist;
  vector<cylinterval> excerptintervals,inputbounds;
  array<int,6> undrange;
  array<int,5> undhisto;
  cylinterval latticebound;
  int fineness=10800;
  initformat("bol","bol","Bezitopo Boldatni",readboldatni,writeboldatni);
  initformat("ngs","bin","US National Geodetic Survey binary",readusngsbin,writeusngsbin);
  initformat("gsf","gsf","Carlson Geoid Separation File",readcarlsongsf,writecarlsongsf);
  initformat("ngatxt","grd","US National Geospatial-Intelligence Agency text",readusngatxt,writeusngatxt);
  initformat("ngabin","","US National Geospatial-Intelligence Agency binary",readusngabin,nullptr);
  outputgeoid.cmap=new cubemap;
  outputgeoid.ghdr=new geoheader;
  outputgeoid.glat=new geolattice;
  outputgeoid.cmap->scale=1/65536.;
  outputgeoid.ghdr->logScale=-16;
  outputgeoid.ghdr->planet=BOL_EARTH;
  outputgeoid.ghdr->dataType=BOL_UNDULATION;
  outputgeoid.ghdr->encoding=BOL_VARLENGTH;
  outputgeoid.ghdr->ncomponents=1;
  outputgeoid.ghdr->tolerance=0.003;
  outputgeoid.ghdr->sublimit=1000;
  outputgeoid.ghdr->spacing=1e5;
  correctionHist.setdiscrete(1);
  argpass1(argc,argv);
  argpass2();
  if (qsz<4)
    qsz=4;
  if (qsz>16)
    qsz=16;
  for (i=0;i<excerptcircles.size();i++)
    excerptintervals.push_back(excerptcircles[i].boundrect());
  for (i=0;i<geo.size();i++)
    inputbounds.push_back(geo[i].boundrect());
  if (excerptintervals.size())
    excerptinterval=combine(excerptintervals);
  else
    excerptinterval.setfull();
  excerptinterval.round(fineness);
  latticebound=intersect(excerptinterval,combine(inputbounds));
  cout<<"latticebound "<<formatlatlong(latlong(latticebound.sbd,latticebound.wbd),DEGREE+SEXAG2);
  cout<<' '<<formatlatlong(latlong(latticebound.nbd,latticebound.ebd),DEGREE+SEXAG2)<<endl;
  /*readgeoid("../g2012bu0.bin");
  readgeoid("../g2012ba0.bin");
  readgeoid("../g2012bh0.bin");
  readgeoid("../g2012bg0.bin");
  readgeoid("../g2012bs0.bin");
  readgeoid("../g2012bp0.bin");
  readgeoid("NCGreenHill150KM.gsf");
  readgeoid("NCAsheville100M.gsf");
  readgeoid("contour.ps");
  readgeoid("ceiling.txt");*/
  //geo[i].settest();
  //drawglobemicro(1024,xy(1.3429,0.2848),3e-4,1,0,"geowrangell.ppm");
  /* (-.4304,-.3142,1) 143.86986°W 61.9475°N 143°52'12"W 61°56'51"N
   * This is a local maximum of the geoid in Alaska at Mount Wrangell.
   * It clearly shows artifacts when interpolated bilinearly.
   */
  //drawglobemicro(1024,xy(0.384566,1.27153),3e-3,1,0,"geonieves.ppm");
  /* (-.230868,-1,0.45694) 103°W 24°N
   * This is on the boundary of the Lower 48 file.
   */
  //drawglobemicro(1024,xy(1.21676,0.294214),3e-3,1,0,"geogranisle.ppm");
  /* (-.411572,-.56648,1) 126°W 55°N
   * This is on the boundary of the overlap between the Alaska file
   * and the Lower 48 file.
   */
  //drawglobemicro(1024,xy(3.579192,1.5984015),1e-3,1,0,"geosamoan.ppm");
  //drawglobemicro(1024,xy(3.6062785,1.627449),1e-3,1,0,"geosamoae.ppm");
  //drawglobemicro(1024,xy(3.579192,1.654771),1e-3,1,0,"geosamoas.ppm");
  //drawglobemicro(1024,xy(3.552552,1.6253505),1e-3,1,0,"geosamoaw.ppm");
  //drawglobemicro(1024,xy(1.5,1.5),2e-2,1,0,"geotest.ppm");
  v=encodedir(Sphere.geoc(degtorad(62),degtorad(-144),0.));
  cout<<"Wrangell "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(24),degtorad(-103),0.));
  cout<<"Nieves "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(55),degtorad(-126),0.));
  cout<<"Granisle "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-11),degtorad(-171),0.));
  cout<<"Samoa N "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-14),degtorad(-168),0.));
  cout<<"Samoa E "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-17),degtorad(-171),0.));
  cout<<"Samoa S "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  v=encodedir(Sphere.geoc(degtorad(-14),degtorad(-174),0.));
  cout<<"Samoa W "<<v.face<<' '<<v.x<<' '<<v.y<<endl;
  if (!helporversion)
  {
    if (formatlist[0].cmd=="bol")
    {
      for (i=0;i<6;i++)
      {
        //cout<<"Face "<<i+1;
        //cout.flush();
        interroquad(outputgeoid.cmap->faces[i],3e5);
        /*if (cube.faces[i].isfull()>=0)
          cout<<" has data"<<endl;
        else
          cout<<" is empty"<<endl;*/
        refine(outputgeoid.cmap->faces[i],outputgeoid.cmap->scale,outputgeoid.ghdr->tolerance,outputgeoid.ghdr->sublimit,outputgeoid.ghdr->spacing,qsz);
      }
      outProgress();
      cout<<endl;
      undrange=outputgeoid.cmap->undrange();
      cout<<"Undulation range: constant "<<undrange[0]<<'-'<<undrange[1];
      cout<<" linear "<<undrange[2]<<'-'<<undrange[3];
      cout<<" quadratic "<<undrange[4]<<'-'<<undrange[5]<<endl;
      undhisto=outputgeoid.cmap->undhisto();
      cout<<"1 byte "<<undhisto[0]<<"; 2 bytes "<<undhisto[1]<<"; 3 bytes "<<undhisto[2]<<"; 4 bytes "<<undhisto[3]<<endl;
      if (dataArea.total()>510e12)
        test360seam();
      delete outputgeoid.glat;
      outputgeoid.glat=nullptr;
    }
    else
    {
      outputgeoid.glat->setbound(latticebound);
      outputgeoid.glat->setfineness(fineness);
      outputgeoid.glat->setundula();
      delete outputgeoid.ghdr;
      delete outputgeoid.cmap;
      outputgeoid.ghdr=nullptr;
      outputgeoid.cmap=nullptr;
    }
  }
  outund("Green Hill",degtobin(35.4),degtobin(-82.05));
  outund("Charlotte",degtobin(35.22),degtobin(-80.84));
  outund("Kitimat",degtobin(54.0547),degtobin(-128.6578)); // in the overlap of two files
  outund("Denali",degtobin(63.0695),degtobin(-151.0074));
  outund("Haleakala",degtobin(20.7097),degtobin(-156.2533));
  /*drawglobemicro(1024,xy(1.3429,0.2848),3e-4,2,0,"geowrangellcvt.ppm");
  drawglobemicro(1024,xy(0.384566,1.27153),3e-3,2,0,"geonievescvt.ppm");
  drawglobemicro(1024,xy(1.21676,0.294214),3e-3,2,0,"geogranislecvt.ppm");
  drawglobemicro(1024,xy(3.579192,1.5984015),1e-3,2,0,"geosamoancvt.ppm");
  drawglobemicro(1024,xy(3.6062785,1.627449),1e-3,2,0,"geosamoaecvt.ppm");
  drawglobemicro(1024,xy(3.579192,1.654771),1e-3,2,0,"geosamoascvt.ppm");
  drawglobemicro(1024,xy(3.552552,1.6253505),1e-3,2,0,"geosamoawcvt.ppm");*/
  //drawglobemicro(1024,xy(1.5,1.5),2e-2,2,0,"geotestcvt.ppm");
  /*ps.open("geoid.ps");
  ps.prolog();
  ps.startpage();
  plotcenters(ps);
  ps.endpage();
  //ps.startpage();
  //plotinters(ps);
  //ps.endpage();
  ps.trailer();
  ps.close();*/
  //hdr.hash=cube.hash();
  if (!helporversion)
  {
    if (!outfilename.length())
    {
      if (infilebasenames.size()==1)
      {
        outfilename=infilebasenames[0]+"."+formatlist[0].ext;
        cout<<"Writing "<<outfilename<<endl;
      }
      else
        cout<<"Please specify a filename with -o"<<endl;
    }
    if (outfilename.length())
      if (formatlist[0].writefunc)
        formatlist[0].writefunc(outputgeoid,outfilename);
      else
        cerr<<"Can't write in format "<<formatlist[0].cmd<<"; it is a whole-earth-only format."<<endl;
    drawglobecube(1024,62,-7,&outputgeoid,0,"geoid.ppm");
    cout<<"avgelev called "<<avgelev_interrocount<<" times from interroquad, "<<avgelev_refinecount<<" times from refine"<<endl;
    //correctionHist.dump();
    cout<<"Computing error histogram"<<endl;
    errorHist=errorspread();
    if (outfilename.length())
    {
      if (errorHist.gettotal())
      {
        ps.open(outfilename+".ps");
        ps.setpaper(papersizes["A4 portrait"],1);
        ps.prolog();
        ps.startpage();
        errorHist.plot(ps,HISTO_LINEAR);
        ps.close();
      }
      else
        cout<<"Error histogram empty, not plotting"<<endl;
    }
  }
  return 0;
}
