#!/bin/bash

machine=$(uname -sm | tr ' ' '-')
echo $machine

function download() {
	url="$1"
	target="$2"
	if [ -e "$target" ]; then return; fi
	curl "$url" -LJo "$target"
}

topdir="$(pwd)"
mkdir -p dependencies
download https://github.com/BessyHDFViewer/StaticImglibs/releases/download/latest/StaticImglibs_$machine.tar.bz2 dependencies/StaticImglibs.tar.bz2

download https://github.com/auriocus/kbskit/releases/download/latest/kbskit_$machine.tar.bz2 dependencies/kbskit.tar.bz2

( cd dependencies
  tar xvf StaticImglibs.tar.bz2
  tar xvf kbskit.tar.bz2 )


builddir="$topdir/build"
distdir="$topdir/dist"
kbskitdir="$topdir/dependencies/kbskit_$machine"
tcldir="$kbskitdir/lib"
staticimglibdir="$topdir/dependencies/StaticImglibs_$machine"

rm -rf "$builddir"

autoconf

./configure LDFLAGS="-L$tcldir" --with-tcl="$tcldir" --with-staticimglibs="$staticimglibdir" --prefix="$builddir" --libdir="$builddir/lib" --exec-prefix="$kbskitdir"
make
make install

cp -r "$staticimglibdir/licenses" "$builddir/lib/3rdparty"

mkdir -p "$distdir"
tar cvjf "$distdir/hdfpp_$machine.tar.bz2" -C "$builddir/lib" $(basename -a "$builddir/lib/"*)
