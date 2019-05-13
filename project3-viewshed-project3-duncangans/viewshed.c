#include "viewshed.h"
#include <stdio.h> 
#include <time.h> 
#include <assert.h> 
#include <stdlib.h> 
#include <math.h>

//This function reads in a asci file representing a terrain and computes the
//viewshed from a specific point. After reading in the grid and computing the
//viewshed, the viewshed, represented by 0s and 1s is then scanned into a file
//of the users choice

int main(int argc, char **argv)
{
  if (argc < 5) {
    printf("usage: viewshed <filename>");
    exit(0); 
  }

  int testrow = atol(argv[3]);
  int testcol = atol(argv[4]);

  //Grid is created
  Grid grid;
  
  //Grid's values are entered from the file entered
  //The arguments represent the files and the grid adress
  readGridfromFile (argv[1], argv[2], &grid);
  //The viewshed is created
  createViewshed(&grid, testrow, testcol);
  //The viewshed is visualized and printed, (commented out for now)
  //printGrid(&grid);
  //The viewshed is then read into the file
  shedIntoFile(&grid, argv[2]);
}

//This function reads a viewshed from the file given and puts it into row
//major format. It also adds the first few lines to the new file so it doesn't
//have to do it later.
void readGridfromFile (char * filename, char * newfile, Grid *grid)
{
  //Creates files and name for scanning as well as necessary integers
  FILE* f; 
  FILE* n;
  char s[100]; 
  int nrows, ncols, ndval;
  
  n=fopen(newfile, "w");
  f=fopen(filename, "r");
  if (f== NULL || n==NULL) {
     printf("cannot open files...");
     exit(1);
  }
  
  //Collecting the number of columns and rows for the grid details 
  fscanf(f, "%s", s);
  fscanf(f, "%d", &ncols);
  //Beginning to add stuff to new file
  fprintf(n, "%s         %d\n", s, ncols);
  fscanf(f, "%s", s);
  fscanf(f, "%d", &nrows);
  //Continuing to add details to new file
  fprintf(n, "%s         %d\n", s, nrows);

  //Scans past unecessary information
  for (int i = 0; i < 4; i++)
  {
    fscanf(f, "%s", s); 
    //Here I am just using ndval to indicate the second int, the last round
    //it will actually set the value to the correct int
    fscanf(f, "%d", &ndval);
    //Continuing to move stuff into new file
    fprintf(n, "%s    %d\n", s, ndval);
  }

  //Grid variables are set
  grid->rows = nrows;
  grid->cols = ncols;
  grid->ndvalue = ndval;

  //Grid is created based on ncols and nrows, and then filled from the file
  grid->data_rowmajor = (float*) calloc(nrows * ncols, sizeof(float));

  float newNum;
  for (int i = 0; i < nrows * ncols; i++)
  {
    fscanf(f, "%f", &newNum);
    grid->data_rowmajor[i] = newNum;  
  }
}

//The grid is given as a parameter and printed in row major format
void printGrid(Grid * grid)
{
  //Grid's viewshed is printed
  for (int i = 0; i < grid->rows; i++)
  {
    for (int j = 0; j < grid->cols; j++)
    {
      printf("%d ", grid->view_shed[(i * grid->cols) + j]);
    }
    printf("\n------------------\n");
  }
}
//When given a row and a column and a grid, it will return the height at
//the given location
float getRowMajor(Grid *grid, int row, int col)
{
  return grid->data_rowmajor[row * grid->cols + col];
}

//When given a test row and a single column it tests whether the point is
//visible from the other point. To do this it iterates through each column/row
//between the two and finds the point that could theoretically block the view.
//If said point is lower than the visibility line it is all good. Otherwise,
//a 0 is returned indicating the point is not visible.
int isVisible(Grid *grid, int row, int col, int testrow, int testcol)
{
  //Distance and slopes from the viewpoint (testrow, testcol) to the point 
  //being tested (row and col) is calculated
  int deltaX = col - testcol;
  int deltaY = testrow - row;
  double delta = sqrt((deltaX*deltaX)+(deltaY*deltaY));
  float visSlope = (getRowMajor(grid, row, col) - 
    getRowMajor(grid, testrow, testcol)) / delta;
  double slope = (double)(testrow - row) / (double)(col - testcol);
  
  //Upper and Lower bounds for rows ints are created, and colChange created
  int ltempRow, htempRow, colChange;
  if (col >= testcol) {colChange = 1;} else {colChange = -1;}
  double midPoint, height, tempDelta, tempSlopeVis;

  //For each line intersection through the columns, the intersection point is
  //created and tested for visibility
  for (int i = testcol; i != col; i+=colChange)
  {
    //Point between rows is created, the height at that location is calculated
    //Finally the slope to that point is checked for visibility
    midPoint = slope * (i - testcol) - floor(slope * (i - testcol));
    ltempRow = testrow - floor(slope * (i - testcol));
    htempRow = ltempRow - 1;
    height = midPoint * getRowMajor(grid, htempRow, i) + (1-midPoint) 
             * getRowMajor(grid, ltempRow, i);
    tempDelta = sqrt((pow(i-testcol, 2)+pow(testrow - (ltempRow-midPoint), 2)));
    tempSlopeVis = (height - getRowMajor(grid, testrow, testcol)) / tempDelta; 
    if (tempSlopeVis > visSlope+.0001 && i!=testcol) {;return 0;}    
  }

  //The code below mirrors the code above, but does so for iterating through
  //the rows instead of the columns.
  double invSlope = (double)(col - testcol) / (double)(testrow - row);
  int ltempCol, htempCol, rowChange;
  if (row >= testrow) {rowChange = 1;} else {rowChange = -1;}
  for (int i = testrow; i!= row; i+=rowChange)
  {
    midPoint = invSlope * (testrow - i) - floor(invSlope * (testrow - i));
    htempCol = testcol - floor(invSlope*(i - testrow));
    ltempCol = htempCol - 1;
    height = midPoint * getRowMajor(grid, i, htempCol) + 
		(1-midPoint) * getRowMajor(grid, i, ltempCol);
    tempDelta = sqrt((pow(i-testrow, 2)+pow(testcol-(ltempCol + midPoint), 2)));
    tempSlopeVis = (height - getRowMajor(grid, testrow, testcol)) / tempDelta;
    if (tempSlopeVis > visSlope+.0001 && i!=testrow) {return 0;}
  }
  //If the function makes it to this point, it means nothing blocks the view
  //and the point IS visible 
  return 1;
}

//Viewshed grid is (c)allocated and then each row and column in the grid is
//tested against the testrow and the testcol for visibility.
void createViewshed(Grid * grid, int testRow, int testCol)
{
  //Grid is allocated and iterated through
  grid->view_shed = (int*) calloc(grid->rows * grid->cols, sizeof(int));  
  for (int row = 0; row < grid->rows; row++)
  {
    for (int col = 0; col < grid->cols; col++)
    {
      if (getRowMajor(grid, row, col) == grid->ndvalue)
      {
        //If the value is a ndvalue, it will be set to NOT visible
	grid->view_shed[(row * grid->cols) + col] = 0;
      }
      else if (testRow == row && testCol == col)
      {
        grid->view_shed[(row * grid->cols) + col] = 1;
      }
      else
      {
        int vis = isVisible(grid, row, col, testRow, testCol);
        grid->view_shed[(row * grid->cols) + col] = vis;
      }
    }
  }
}

//Viewshed grid is then read into the file. 
void shedIntoFile(Grid *grid, char * newfile)
{
  FILE* n;
  n=fopen(newfile, "a");
  for (int row = 0; row < grid->rows; row++)
  {
    for (int col = 0; col < grid->cols; col++)
    {
      fprintf(n, "%d ", grid->view_shed[row * grid->cols + col]);
    }
  fprintf(n, "\n");

  }
}

