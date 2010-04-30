#include "subgraph.h"
#include "node.h"

#include <QDebug>

/********************************
 * Function: center
 * ----------------
 * calculates the average x,y,h,s,l values of a subgraph and returns them in a
 * point
 *
 * Returns:
 *    Point containing averages of subgraph values
 */
Point Subgraph::center()
{
  // average values
  qreal x, y, hue, saturation, lightness;
  x = y = hue = saturation = lightness = 0;
 
  Point pt;
  foreach (GngNode* node, *this){
    pt = node->location();
    x += pt[0];
    y += pt[1];
    hue += pt[2];
    saturation += pt[3];
    lightness += pt[4];
  }
  
  x/size();
  y/size();
  hue/size();
  saturation/size();
  lightness/size();
  
  Point avg;
  avg.resize(5);
  avg[0] = x;
  avg[1] = y;
  avg[2] = hue;
  avg[3] = saturation;
  avg[4] = lightness;
  
  return avg;
}
