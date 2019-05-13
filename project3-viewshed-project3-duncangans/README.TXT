Duncan Gans
GIS Algorithms
October 11, 2017

viewshed.c
  This file is what runs the viewshed algorithm. It takes a file with a terrain
  grid, a location on the grid, and a file to read the viewshed into. The 
  viewshed is created and read into the file.  

bocked.h
  Includes the code that creates the Grid structure as well as all of the 
  function declarations

test1.asc, test2.asc, test3.asc, set1.asc
  Various test files. They all work well.

set1visComparison.png
  This file illustrates the comparison of Professor Toma's viewshed and mine.
It illustrates that they are identical. It also shows the way that the render
displays my file in a slightly different way than Professor Toma's. It could
be because the set1.asc has a different length of row on various rows. 
 
render 
  This directory includes all the files included and necessary for Professor's 
  viewshed rendering. Most importantly though, this has correct renderings 
  for Professor Toma's viewshed algorithms as well as mine. Mine are labeled 
  set1vis.100.100.Gans.asc and set1vis.250.250.Gans.asc. These can be compared
  to Professor Toma's set1vis.100.100.asc and set1vis.250.250.

Problems, qualifications:
  To my knowledge, the code works well. In all of the small tests I created
for it, it worked well. It would not entirely surprise me if there are minor
problems, but I have not come across them. When testing it against the set1.vis
offered by professor Toma, my results matched Toma's. However, the way they
are rendered is slightly different. Mine map across the other side in a way that
Professor Toma's don't. The actual positioning is the same, but my rendering 
seems to be cut and wrapped onto the other side. It will be more obvious with
the attached images. This difference was likely in the format of the files or
how render works and NOT in my algorithm. This is obvious because the renders
are identical once the wrapping is done. Overall a fun, challenging lab. It
probably took me around 10 hours total. 
