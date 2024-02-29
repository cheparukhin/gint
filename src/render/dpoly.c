#include <gint/defs/util.h>
#include <gint/display.h>

void dpoly(int *polyX, int *polyY, int polyCorners, int fill_color, int border_color){

  if (polyCorners<=2) return;

  if (fill_color == C_NONE && border_color == C_NONE)
    return;

  if (fill_color != C_NONE) {
    
    float *nodeX, pixelX, pixelY, swap ;
    int i, j, nodes;
    
    nodeX = malloc( polyCorners * sizeof(float));

    //  Loop through the rows of the image.
    for (pixelY=0; pixelY<DHEIGHT; pixelY++) {
      
      //  Build a list of nodes.
      nodes=0; j=polyCorners-1;
      for (i=0; i<polyCorners; i++) {
        if ((polyY[i]<(float) pixelY && polyY[j]>=(float) pixelY)
        ||  (polyY[j]<(float) pixelY && polyY[i]>=(float) pixelY)) {
          nodeX[nodes++]=(int) (polyX[i]+(pixelY-polyY[i])/(polyY[j]-polyY[i])*(polyX[j]-polyX[i]));
        }
        j=i;
      }

      //  Sort the nodes, via a simple “Bubble” sort.
      i=0;
      while (i<nodes-1) {
        if (nodeX[i]>nodeX[i+1]) {
          swap=nodeX[i]; nodeX[i]=nodeX[i+1]; nodeX[i+1]=swap; if (i) i--;
        }
        else {
          i++;
        }
      }

      //  Fill the pixels between node pairs.
      for (i=0; i<nodes; i+=2) {
        if (nodeX[i  ]>=DWIDTH) break;
        if (nodeX[i+1]<= 0 ) break;
        if (nodeX[i  ]< 0 ) nodeX[i  ]=0 ;
        if (nodeX[i+1]> DWIDTH) nodeX[i+1]=DWIDTH;
        //for (pixelX=nodeX[i]; pixelX<nodeX[i+1]; pixelX++) fillPixel(pixelX,pixelY);
        dline(nodeX[i], pixelY, nodeX[i+1], pixelY, fill_color);
      }
    }       

    free(nodeX);
  }

  if (border_color != C_NONE) {
    for (int i = 0; i < polyCorners; i++) {
      int px = polyX[i];
      int py = polyY[i];
      int px2 = polyX[(i + 1) % polyCorners];
      int py2 = polyY[(i + 1) % polyCorners];
      dline(px, py, px2, py2, border_color);
    }
  }

  return;
}
