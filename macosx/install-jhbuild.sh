#!/bin/sh

echo Make sure there is no Homebrew, MacPorts or Fink installed, or the 
echo installation of jhbuild will fail. Remove references to the

curl https://gitlab.gnome.org/GNOME/gtk-osx/raw/master/gtk-osx-setup.sh
export CXXFLAGS="-std=c++11 -stdlib=libc++"
sh ./gtk-osx-setup.sh

jhbuild bootstrap-gtk-osx
jhbuild build python3
jhbuild build pygments
jhbuild build meta-gtk-osx-bootstrap
jhbuild build meta-gtk-osx-gtk3

echo Compilation if Insensitive on macOS required Fortran runtime libraries,
echo which can be obtained by installing the GNU Fortran compiler from
echo
echo     https://github.com/fxcoudert/gfortran-for-macOS/releases
echo
echo Modifying jhbuildrc and jhbuildrc-custom in ~/.config/jhbuildrc to
echo use local moduleset...

sed -i '' "s/use_local_modulesets = False/use_local_modulesets = True/g" ~/.config/jhbuildrc
sed -i '' "s/#\ moduleset = \"gtk-osx\"/moduleset\ =\ \"insensitive\"/g" ~/.config/jhbuildrc-custom
echo cmakeargs = \'-DCMAKE_SYSTEM_IGNORE_PATH=\"/opt/homebrew:/opt/macports:/sw:/usr/local\"\' >> ~/.config/jhbuildrc-custom
if test -f ./insensitive.modules ; then
    sed -i '' -e '/use_local_modulesets = True/ {' -e "n; s|modulesets_dir.*|modulesets_dir = \"`pwd`\"|" -e '}' ~/.config/jhbuildrc
    jhbuild build fftw openblas insensitive
else
    echo
    echo You need to add the path to insensitive.modules manually to ~/.config/jhbuild
    echo and invoke \"jhbuild build insensitive\" to compile insensitive.  
fi
echo
echo Copy the directory \"Catalina\" from the macosx directory in the source tree
echo to \~/gtk/inst/share/themes and build the application bundle by running
echo "    jhbuild shell"
echo "    cd ~/gtk/source/insensitive/macosx"
echo "    gtk-mac-bundler ./Insensitive.bundle"
echo

