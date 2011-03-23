#!/usr/bin/sh -f

### $Id: build_tar.sh,v 1.1.1.1 2000/06/08 00:10:03 afikes Exp $
###
### Builds osuif tar release.
### - The script should be invoked from this directory!
### - The CVSROOT environment variable must be set
###   (Currently /fs/oo1/users/osuif/SUIF_repository/)
### - If the script is invoked in ".", the script builds
###   - ./<release_name>.tar  (tar release)
###   - ./src (root of untared release)
 
package_name=osuif
release_name=osuif-`date '+%d-%m-%y'`

rm -rf src

mkdir src
cd src

xargs -L 1 -Ixxx -t cvs -q checkout xxx < $OSUIFHOME/release/checkout.lst

# Remove CVS directories created by checkout
find . -name CVS -print | xargs -L 1 -t rm -rf

# Rename directories and generate link
mv ${package_name} ${release_name}
ln -s ${release_name} ${package_name}

# Build tar file
cd ..
tar cf ${release_name}.tar src
compress ${release_name}.tar
chmod a+r ${release_name}.tar.Z
