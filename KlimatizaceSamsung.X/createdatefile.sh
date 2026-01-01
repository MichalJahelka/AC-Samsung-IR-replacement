#!/usr/bin/bash

den=`date +%_d`
mesic=`date +%_m`
rok=`date +%_y`

den=$(echo "#define D_DEN    $den")
mesic=$(echo "#define D_MESIC $mesic")
rok=$(echo "#define D_ROK    $rok")

if [ -f Datum.h ]; then

# Create new file handle 5
exec 5< Datum.h

read -r dline <&5
read -r mline <&5
read -r rline <&5

# Close file handle 5
exec 5<&-

if [ "$den" != "$dline" ]; then 
WriteFile=true
fi

if [ "$mesic" != "$mline" ]; then 
WriteFile=true
fi

if [ "$rok" != "$rline" ]; then 
WriteFile=true
fi

else

WriteFile=true

fi

if [ "$WriteFile" = true ]; then
echo "$den" > Datum.h
echo "$mesic" >> Datum.h
echo "$rok" >> Datum.h
#echo Updating
fi
