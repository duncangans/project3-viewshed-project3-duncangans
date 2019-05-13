#ifndef viewshed
#define viewshed

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>


typedef struct _grid {

     int  rows, cols, ndvalue;//Necessary variables

     float* data_rowmajor;   //the values in the grid, in row-major order

     float* data_blocked;    //the values in blocked layout(not actually used)

     int* view_shed;  //Viewshed Grid

} Grid;

//Function declarations
void readGridfromFile (char * filename, char * newfile, Grid *grid);
void printGrid(Grid * grid);
float getRowMajor(Grid *grid, int row, int col);
void createViewshed(Grid * grid, int testRow, int testCol);
int isVisible(Grid *grid, int row, int col, int testrow, int testcol);
void shedIntoFile(Grid *grid, char * newfile);



#endif
