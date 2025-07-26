# FFPIRATEVIEW(1)              Farbfeld Image Viewer             FFPIRATEVIEW(1)

## NAME
       ffpirateview  -  simple  viewer  for farbfeld (.ff), and 1fpirate image
       files

## SYNOPSIS
       ffpirate [FF_FILE]

       zviewer.sh FF_FILE]

       gimp_open.sh FF_FILE]

## DESCRIPTION
       ffpirate is a lightweight SDL2-based image viewer designed  to  display
       uncompressed or decompressed Farbfeld image files. It can also directly
       view  1fpirate image files. It supports basic interaction and image manipulation.  
       It also includes a helper script, "zviewer.sh"  which  allows  you  to 
       open a file compressed in a number of different formats.  The formats 
       for zviewer.sh , are bzip2  (ff.bz2,  ffbz),  lzip  (ff.lz, ffz),  
       (ff.xz, ffxz).  It can also view jpeg, png, and webp by converting them
       to farbfeld before viewing.  This conversion by zviewer relies on 
       external farbfeld tools, 2ff (jpg2ff, png2ff).
