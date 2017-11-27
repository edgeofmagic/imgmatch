# Imgmatch

## A program for finding duplicate images

### Features

* Simple, flexible command line interface.
* Use you favorite image viewer to browse matches easily.
* Finds matching images regardless of resizing, re-compression, rotating, or flipping.
* Finds matches within a directory, or between directories.
* Works with JPEG, PNG, and BMP files.
* At present, runs on macOS. Porting to Linux should be straightforward.

### Quick example

In this example, assume our home directory has a subdirectory named 'images', 
which contains six JPEG files:
```` bash
$ cd ~images
$ ls
aardvark.jpg
anteater.jpg
giraffe.jpg
tortise.jpg
turtle.jpg
zebra.jpg
````
Also assume (for the sake of the example) that aardvark.jpg and anteater.jpg 
are the same image, as are tortise.jpg and turtle.jpg. They need not be exact 
duplicates&mdash;aardvark.jpg might be a resized copy of anteater.jpg, or 
tortise.jpg might be a copy of turtle.jpg with increased JPEG compression.

Run the imgmatch command:
```` bash
$ imgmatch
````
When there are no command arguments, imgmatch searches for duplicate images
(JPEG, BMP, or PNG) in the current working directory. If any duplicates are
found, imgmatch creates a subdirectory named 'matches':
```` bash
$ ls
aardvark.jpg
anteater.jpg
giraffe.jpg
matches
tortise.jpg
turtle.jpg
zebra.jpg
````
'matches' contains two directories:
```` bash
$ ls matches
m0
m1
````
These contain symbolic links the files that appear to be duplicate images:
```` bash
$ ls -l matches/m0
lrwxr-xr-x  1 dcurtis  staff  49 Nov 22 19:36 aardvark.jpg -> /Users/dcurtis/images/aardvark.jpg
lrwxr-xr-x  1 dcurtis  staff  49 Nov 22 19:36 anteater.jpg -> /Users/dcurtis/images/anteater.jpg
$ ls -l matches/m1
lrwxr-xr-x  1 dcurtis  staff  49 Nov 22 19:36 tortise.jpg -> /Users/dcurtis/images/tortise.jpg
lrwxr-xr-x  1 dcurtis  staff  49 Nov 22 19:36 turtle.jpg -> /Users/dcurtis/images/turtle.jpg
````

If we open the links in an image viewer by double-clicking on them in the 
Finder (macOS), we can see the images and decide which to keep and which to 
delete. See below, in [Dealing with match results](#dealing-with-match-results)
for convenient ways to delete the images.

When we're finished, we can delete 'matches' and its subdirectories to clean up
the links, either with a command:

```` bash
$ cd ~/images
$ rm -rf matches
````
or by using the Finder to put the matches folder in the trash.

### Search directories and targets

Each search has one or more *search paths*. A search path identifies a directory 
that contains image files. Search paths are specified with arguments on the 
imgmatch command:

````bash
$ imgmatch image_folder wallpaper_images ~/Pictures/cats
````
If no search path is specified in the command, imgmatch will use the current 
working directory as a search path.

A search may optionally specify a *target*. The target is a path that may 
identify a single image file, or a directory containing image files. The target 
is specified with a command option:

````bash
$ imgmatch -t aardvark.jpg
````
or
````bash
$ imgmatch -t image_folder
````

If the target is a JPEG, PNG, or BMP image, it will be compared with the 
contents of the search directories. If the target is a directory, all of the 
image files contained in the target directory are compared against images in 
the search directories.

If no target is specified, imgmatch finds matching images within each search 
directory&mdash;it does't compare images between search directories if no 
target is specified.

#### Example

Let's consider a slightly different scenario from the example above. The 
same files are split into two directories&mdash;'images' and 'more_images':

```` bash
$ cd ~
$ ls
images
more_images
$ ls images
anteater.jpg
tortise.jpg
zebra.jpg
$ ls more_images
aardvark.jpg
giraffe.jpg
turtle.jpg
````

If we execute this command:

```` bash
$ imgmatch --target images more_images
````

imgmatch will compare every image file in 'images' with the image files
in 'more_images'.

```` bash
$ ls
images
matches
more_images
$ ls matches
m0
m1
$ ls matches/m0
anteater.jpg
aardvark.jpg
$ ls matches/m1
tortise.jpg
turtle.jpg
````

Alternatively, we could search with a target file, rather than a directory:

```` bash
$ imgmatch --target images/anteater.jpg more_images
````

In which case the results would include only one set of matching files:

```` bash
$ ls matches
m0
$ ls matches/m0
aardvark.jpg
anteater.jpg
````

### Dealing with Match Results

> There is no command line interface so bad 
> that it can't be made worse by adding a GUI

&mdash;The author

I *really* didn't want to add a GUI. I've used image duplicate-finding
applications with GUIs, and their built-in image viewers are never as good as
stand-alone viewers that are freely available on most platforms. Nevertheless,
when potential matches are found, it's usually necessary to compare them visually
before deciding what to keep and what to delete. The first version of imgmatch
simply generated output text listing the matching files. The process of
navigating to those files in a Finder window so I could open them with a
viewer, and then deleting the unwanted duplicates proved awkward and 
frustrating, particularly if the images were in different directories, or 
the directories were large. The program was simple, but the resulting workflow 
was terrible.

The solution I settled on is simple, and it provides a relatively smooth
workflow when de-duping large image sets. When imgmatch finds set of matching
images, it creates a new subdirectory and puts symbolic links to the image files
in that directory. Navigate to the results directory in the Finder (macOS), and
open the images with a viewer by double-clicking on the symbolic links, the 
decide what to keep and what to delete.

#### Deleting unwanted duplicates

It's important to note that you want to delete the image files themselves,
not the links generated by imgmatch (delete the links all at once when we're
finished, to clean up).

##### With the viewer

Some image viewers allow you to delete images directly within the viewer. My 
favorite viewer (Xee<sup>3</sup> - [https://theunarchiver.com/xee]) supports 
this capability (via command-delete, or from the 
menu&mdash;File > Move to Trash). The most important thing to note is this: *if
you open an image by clicking on a symbolic link, and then delete it with 
Xee<sup>3</sup>, it will delete the target of the symbolic link (the image 
file itself), not the link.* This feature makes using imgmatch and 
Xee<sup>3</sup> together a reasonably smooth workflow for eliminating 
duplicate images.

##### From the command line

If you end up deleting from the command line, the symbolic links make that 
a bit easier, too. Put the following function definition in your .bash_profile
or .bash_rc file:

````bash
rmlt() {
    echo rm `readlink -n $1`
    rm `readlink -n $1`
}
````
You can delete the image file by executing the function/command
rmlt (remove link target) on the link:

````bash
$ cd ~/image/matches/m0
$ rmlt anteater.jpg
````

You may prefer to use the GNU version of readlink, available from Homebrew or
MacPorts in the GNU coreutils package. It gets renamed to greadlink
to avoid conflict with the native macOS readlink. If so, replace the function
in .bash_profile with the following:

````bash
rmlt() {
    echo rm `greadlink -f $1`
    rm `greadlink -f $1`
}
````

The -f flag isn't necssary for links generated by imgmatch, since they are
absolute links to canonical file paths, but this version will work with any 
symbolic links.

### Command syntax

Each option has a long name and a short (single-letter) abbreviation, which 
is typically the first letter of the long name. A long-name option is preceeded 
by two hypens on the command line; abbreviations are preceeded by a single 
hyphen.

The general command syntax is:

**imgfile** \[ *option*... \] \[ *search_path*... \]

where *options* are described below.

### Options
#### Set Target
**--target** *path* <br/>
**-t** *file*

If *path* is a JPEG image file, it will be compared with image files in the 
search paths. If *path* is a directory, all images in that directory will be 
compared with images in the search directories. If no target is specified, each 
search path will be searched for matches within the search path.

#### Set Results Location
**--results** *path* <br/>
**-r** *path*

The *path* argument determines where the results of the matching process will 
be put. If *path* doesn't correspond to an existing file or directory, a 
directory with that name will be created. If *path* identifies an existing 
directory that is empty, that directory will be used. If *path* identifies an 
existing non-directory file or a directory that is not empty, imgmatch will 
attempt to create a directory with the same parent as *path*, by concatening a 
number (starting with zero) to the last element of *path*. The number will be 
incremented until a name is generated that has no corresponding existing file. 
The default value for *path* is './matches'.

#### Set Limit
**--limit** *num* <br/>
**-l** *num*

Sets the maximum number of images in each search directory that will be compared 
with the target. If a target directory is specified, limit also sets the maximum 
number of images in the target directory that will be compared with search 
directory contents. The default value is 1000. If the value is negative, no
limit will be imposed.

#### Set Match Threshold
**--match** *num* <br/>
**-m** *num*

Sets the match threshold, where *num* is a non-negative floating-point number. 
File comparisons generate a numeric distance value. If two images are identical, 
the distance will be 0. The match option sets threshold at which two compared 
files will be considered a match. The default value is 0.1. 

#### Set Verbosity Level
**--spew** \[*num*\] <br/>
**-s** \[*num*\]

Sets the level of verbosity for generated commentary. Valid values 
are 0, 1 and 2. Level 0 produces no output unless an error condition is 
occurred. Level 1 produces a moderate amount, announcing the target and search 
directories as they are being searched, and announcing matches when found. 
Level 2 produces voluminous output. For example, it announces every comparison 
as it is being made, along with the resulting distance from the comparison. The 
default level is 0. If the verbose option is specified but no level is given, 
the level will be set to 1.

#### Show version
**-v** <br/>
**--version**

Shows the version of imgmatch being executed.

### Build and install

#### Dependencies

##### Boost (v 1.63 or later)

Specifically, imgmatch uses boost program options and file system libraries.
Download from [http://boost.org], build and install according to the 
instructions in the download.

#### libjpeg (release 9b)

Imgmatch uses libjpeg to decode JPEG images. Download from 
[http://www.ijg.org/files/], build and install according to instructions in the 
download.

#### LodePNG

Imgmatch uses the LodePNG library to decode PNG images. No download is needed, 
as the necessary files are included in the imgmatch repository. LodePNG is
the property of Lode Vandevenne, and is used here in accordance with its 
licensing terms. The project site can be found at [http://lodev.org/lodepng],
and the repository at [https://github.com/lvandeve/lodepng].

#### C++ Bitmap Library

Imgmatch uses the C++ Bitmap Library to load BMP images, and as the internal
representation of all decoded images. No download is needed, as the
only file (bitmap_image.hpp) is included in the imgmatch repository. 
The C++ Bitmap Library is the property of Arash Partow, and is used here in 
accordance with its licensing terms. The project website for C++ Bitmap Library 
can be found at [http://www.partow.net/programming/bitmap/index.html], and 
the repository at [https://github.com/ArashPartow/bitmap].

#### Building imgmatch

In order to build on macOS, you will need to have Xcode and the Xcode
command line tools installed, as well as cmake (from MacPorts).

From the top-level project directory, go the the build sub-directory, execute
cmake and make:

````bash
$ cd build
$ cmake ..
$ make
````

If all is well, the executable named imgmatch will be created in build. To make
it generally available, put a copy of imgmatch (or a link to it) in a directory 
in your path.

### How it works

Imgmatch builds a histogram of color values for each image involved in a search. 
It compares image histograms pairwise, by calculating a Chi-Squared distance 
measure between the histograms.

The histograms have 4096 bins. Each RGB pixel is mapped to a bin as follows:

* Interpret the bins of the histogram as a three-dimensional cube, each
dimension having 16 bins. 

* Given a pixel (r, g, b) with channel values from 0 to 255, scale the values to
the cube dimensions, dividing by 16. 

* Interpret the scaled pixel values (r', g', b') as indices into the cube.

For a given image, each bin contains the count of pixels in the image that
map to that bin.

When calculating the distance, the bins are interpreted as a vector of 4096 bins.
Given images a and b, we have:

* N<sub>a</sub> - the number of pixels in a

* N<sub>b</sub> - the number of pixels in b

* A - the histogram for a, with bins A</sup><sub>i</sub>

* B -  the histogram for b, with bins B</sup><sub>i</sub>

* &#956;<sub>i</sub> = ((A<sub>i</sub> / N<sub>a</sub>) + 
(B<sub>i</sub> / N<sub>b</sub>)) / 2

* dist = &#931;<sub>i</sub> (((A<sub>i</sub>/N<sub>a</sub>) - 
(B<sub>i</sub>/N<sub>b</sub>))<sup>2</sup> / &#956;<sub>i</sub>)
for i = 0 .. 4095, where  &#956;<sub>i</sub> &ne; 0

As a distance measure, smaller values indicate greater similarity between
the histograms. A distance of zero indicates identical histograms.

Imgmatch's default threshold for a "match"&mdash;considering the images to 
be probable duplicates&mdash;is 0.1. If you find that an excessive number of
false positives (matches containing dissimilar images) are being generated,
try decreasing the match value with the -m option. Conversely, if images that
you consider to be duplicates are not being recognized by imgmatch, try 
increasing the threshold.

If the verbosity level is set to maximum with option -s2, imgmatch will
show the resulting distance for all comparisons being made. Knowing the distance
measures for a set of images (particularly when there are false negatives) can 
be useful when adjusting the match threshold.

