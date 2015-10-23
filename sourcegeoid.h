/******************************************************/
/*                                                    */
/* sourcegeoid.h - geoidal undulation source data     */
/*                                                    */
/******************************************************/
/* http://www.hypack.com/new/portals/1/PDF/sb/07_10/New Geoid Model for France and Information about Geoid File Formats.pdf
 * describes some geoid file formats. These are latitude-longitude grids.
 * I don't know what formats are used for polar geoid files.
 */
#include <vector>
#include "angle.h"

class geolattice
{
  /* nbd must be greater than sbd; both must be in [-DEG90,DEG90].
   * ebd-sbd must be positive. Neither has to be in [-DEG180,DEG180].
   * undula is 4-byte integers with 0x10000 meaning 1 meter. 0x80000000 means NaN.
   * size of undula is (width+1)*(height+1) - note fencepost!
   */
public:
  int type; // not used yet - will distinguish lat-long grid from whatever is used at the poles
  int nbd,ebd,sbd,wbd; // fixed-point binary - 18 mm is good enough for geoid work
  int width,height;
  std::vector<int> undula; // starts at southwest corner
  double elev(int lat,int lon);
  double elev(xyz dir);
};