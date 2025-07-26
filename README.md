# FFPIRATEVIEW(1)              Farbfeld Image Viewer             FFPIRATEVIEW(1)

## NAME
       ffpirateview  -  simple  viewer  for farbfeld (.ff), and 1fpirate image
       files

## SYNOPSIS
       ffpirate [FF_FILE]

       zviewer.sh FF_FILE]

       gimp_open.sh FF_FILE]

## DESCRIPTION
       ffpirate is a lightweight SDL2-based image viewer designed to display 
       uncompressed or decompressed Farbfeld image files. It can also directly 
       view 1fpirate image files and supports basic interaction and image manipulation.
       It can save 1fpirate files as farbfeld (conversion)

       It includes a helper script, zviewer.sh, which allows you to open files
       compressed in a variety of formats. Supported formats for zviewer.sh include:

       - bzip2 (.ff.bz2, .ffbz)
       - lzip (.ff.lz, .ffz)
       - xz (.ff.xz, .ffxz)

       Additionally, it can view JPEG, PNG, and WebP images by converting them 
       to Farbfeld format before viewing. This conversion, performed by zviewer.sh, 
       relies on external Farbfeld tools such as 2ff (e.g., jpg2ff, png2ff).
       
## 1fpirate
      1fpirate is a single-bit-per-pixel image format modeled after farbfeld. 
      It includes an 8-byte magic header, a width, a height, and pixel data.  
      The pixel data is stored as bits within bytes. This should, in theory, allow 
      for incredibly small drawings, documents, and raster line art.  It can 
      be further compressed using algorithms like bzip2 to reduce the size even 
      more.
## Conversion tools
       ff21f - convert farbeld file to 1fpirate  echo file.ff | ff21f > file.1fpirate
       png21f - convert png's to 1fpirate (use dithering)
       pirate1ffix - Fix magic header on 1fpirate files. (if corrupted)
