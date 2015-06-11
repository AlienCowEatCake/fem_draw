#!/bin/bash

sizes="16 32 48 64 128"

for size in ${sizes} ; do
	inkscape -z -e icon_${size}.png -w ${size} -h ${size} drawing.svg
done

echo "Don't forget change color depth with GIMP!"

