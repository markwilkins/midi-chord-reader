#!/bin/bash

usage="usage: updateversion oldver newver"

[[ -z "$1" ]] && { echo "$usage" ; exit 1; }
[[ -z "$2" ]] && { echo "$usage" ; exit 1; }

# escape the period separators on the input
oldversion=$(echo "$1" | sed -e "s/\./\\\./g")
newversion=$2

# Do the source files
sedscript="s/^\( *\* *@version  *\)$oldversion *$/\1$newversion/"
# echo "$sedscript"
pushd .
cd ../src
sed -i.bak -e "$sedscript" *.cpp
sed -i.bak -e "$sedscript" *.h
rm *.bak
popd

# cmakefile
sedscript="s/^\(project(MidiChords VERSION \)$oldversion) *$/\1$newversion)/"
sed -i.bak -e "$sedscript" ../CMakeLists.txt
rm ../CMakeLists.txt.bak

# build script
#version="0.8.0"
sedscript="s/^version=\"$oldversion\"/version=\"$newversion\"/"
sed -i.bak -e "$sedscript" buildinst.sh
rm buildinst.sh.bak
