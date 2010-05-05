#!/bin/bash

function input () {
  echo $1
  echo "Continue?"
  read foo
  $1
}

cd build/
input "./gng-image -c ../config/gng-image.cfg -p ../images/rgb/rgb01.png"

input "./gng-imageseries -c ../config/gng-imageseries.cfg -d 1 -w 0.1"

input "./gng-image -c ../config/gng-image.cfg -p ../images/simple/paper1.jpg"

input "./gng-image -c ../config/gng-image.cfg -p ../images/simple/paper2.jpg"

input "./gng-image -c ../config/gng-image.cfg -p ../images/simple/paper3.jpg"

input "./gng-image -c ../config/gng-image.cfg -p ../images/simple/paper4.jpg"

input "./gng-image -c ../config/gng-image.cfg -p ../images/simple/paper4.jpg -e 0.01"

input "./gng-camera -c ../config/gng-camera.cfg -e 0.015 -m 20"

cd ..
