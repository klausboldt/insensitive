# Insensitive

Insensitive (Incredible Nuclear Spin EvolutioN SImulation Tool Intended for 
Visual Education) is a simulation of the nuclear magnetic resonance (NMR)
experiment for educational purposes. It allows to interact with every step 
from spin precession to Fourier transformation and processing of acquired 1D 
and 2D spectra. Two papers have been published on Insensitive, in [Concepts of 
Magnetic Resonance](https://onlinelibrary.wiley.com/doi/full/10.1002/cmr.a.20203)
(DOI:10.1002/cmr.a.20203) and in [Applied Magnetic Resonance](https://doi.org/10.1007/s00723-023-01552-9) 
(DOI:10.1007/s00723-023-01552-9).

Until version 0.9.33 Insensitive was written in Objective-C and based on 
Apple's Cocoa framework. For version 0.9.34 it has been ported to C with GLib
and GTK3 for better portability. The new version is compatible with the old
file formats for spin systems, pulse programs, and spectra.

## Installation

The program supports both the Meson Build system and GNU Autotools. It can be 
compiled and installed with the following commands:

```
meson build
ninja -C build/
ninja -C build/ install
```

or, alternatively:

```
./configure
make
make install
```

Insensitive can be compiled for Microsoft Windows (7 and above) using 
[MSYS2](https://www.msys2.org/). It has been tested in the ucrt64 environment 
after installing the required dependencies using `pacman` and invoking 
`./configure --prefix=/usr`.

On Apple macOS Insensitive has been successfully compiled using GTK-OSX and 
[JHBuild](https://wiki.gnome.org/Projects/GTK/OSX/Building). Insensitive for
macOS makes use of the `gtk-osx-integration` library and `gtk-mac-bundler`
to create a self-consistent application bundle. an installation script,
moduleset and bundle file are provided in the `macosx` directory of the
source tree.

There are no working ports of WebKit2GTK for Windows or macOS. On these
systems Insensitive will fall back to displaying the tutorial pages using
the default web browser.


## Dependencies

- BLAS/LAPACK
- FFTW3
- GLib 2.0
- Gio >= 2.50
- GTK+ >= 3.22
- Cairo
- WebKit2GTK 4.0
- libxml 2.0
- gfortran

On Ubuntu-based systems, install the dependencies with

```
sudo apt install libblas-dev liblapack-dev libfftw3-dev libxml2-dev \
                 libglib2.0-dev libgtk-3-dev libwebkit2gtk-4.0-dev \
                 libgfortran5
```

On Fedora-based systems, install the dependencies with

```
yum install blas-devel lapack-devel fftw3-devel libxml2-devel \
            glib2-devel gtk3-devel webkit2gtk3-devel libgfortran5
```

## Support

Insensitive includes a tutorial and user manual, written in HTML, that can be 
accessed from the graphical user interface. It includes practical examples and
discusses the limitations of the simulation. Example files for useful spin
systems, common pulse programs, and 1D and 2D spectra can be found in the
`examples/` directory of the distribution. 

Insensitive 0.9.13 made changes to the *.iss file format. Insensitive 0.9.24
made changes to the *.ipp file format. Insensitive 0.9.22 made changes to the
*.igg file format. Insensitive is fully backwards compatible, but files created 
with a newer version of the program cannot be opened with older (macOS) 
versions of Insensitive.

For any further question on the usage of Insensitive, or to report bugs or 
make suggestions, please contact [Klaus Boldt](mailto:klaus.boldt@uni-rostock.de).

## Third Party Software

Insensitive makes use of the Levenberg-Marquardt fitting algorithm written by 
[Ron Babich](https://gist.github.com/rbabich/3539146/) and CONREC by 
[Paul Bourke](http://paulbourke.net/papers/conrec/) to draw contour plots.

## License

Copyright (C) 2009-2023 Klaus Boldt

Licensed under the MIT License.
