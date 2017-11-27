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
directory; it does't compare images between search directories when there 
is no target.

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
not the links generated by imgmatch. Delete the match links all at once when 
you're finished, to clean up).

##### With the viewer

Some image viewers allow you to delete images directly within the viewer. My 
favorite viewer&mdash;[Xee<sup>3</sup>](https://theunarchiver.com/xee) supports 
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
MacPorts in the GNU coreutils package. It's renamed to greadlink to avoid 
conflict with the native macOS readlink. If so, replace the function
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

The general command syntax is:

**imgfile** \[ *option*... \] \[ *search_path*... \]

where *options* are described below. Each option has a long name and a short 
(single-letter) abbreviation, which is typically the first letter of the long 
name. A long-name option is preceeded by two hypens on the command line; 
abbreviations are preceeded by a single hyphen.

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

Sets the directory where imgmatch puts the results of the matching process. 
If *path* doesn't identify an existing file or directory, a directory with that 
name will be created. If *path* identifies an existing directory that is empty, 
that directory will be used. If *path* identifies an existing non-directory 
file or a directory that is not empty, imgmatch will attempt to create a 
directory with the same parent as *path*, by concatening a number (starting 
with zero) to the last element of *path*. The number will be incremented until 
a name is generated that has no corresponding existing file or directory. 
The default value for *path* is './matches'.

#### Set Limit
**--limit** *num* <br/>
**-l** *num*

Sets the maximum number of images in each search directory that will be compared 
with the target to the value of *num*, which must be an integer. If a target 
directory is specified, limit also sets the maximum 
number of images in the target directory that will be compared with search 
directory contents to *num*. The default value is 1000. If *num* is negative, no
limit will be imposed.

#### Set Match Threshold
**--match** *num* <br/>
**-m** *num*

Sets the match threshold to *num*, which must be a non-negative floating-point 
number. File comparisons generate a numeric distance value. If two images are 
identical, the distance will be 0. Files are considered to match if their 
distance measure is less than or equal to the threshold. The default value 
is 0.1.

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

Shows the version of imgmatch.

### Build and install

#### Dependencies

#### Boost (v 1.63 or later)

Specifically, imgmatch uses boost program options and file system libraries.
Build and install according to the instructions in the 
[download](http://boost.org).

#### libjpeg (release 9b)

Imgmatch uses libjpeg to decode JPEG images. Build and install according to 
instructions in the [download](http://www.ijg.org/files/).

#### LodePNG

Imgmatch uses the LodePNG library to decode PNG images. No download is needed, 
as the necessary files are included in the imgmatch repository. LodePNG is
the property of Lode Vandevenne, and is used here in accordance with its 
licensing terms. Links: [project site](http://lodev.org/lodepng) and 
[repository](https://github.com/lvandeve/lodepng).

#### C++ Bitmap Library

Imgmatch uses the C++ Bitmap Library to load BMP images, and as the internal
representation of all decoded images. No download is needed, as the
only file (bitmap_image.hpp) is included in the imgmatch repository. 
The C++ Bitmap Library is the property of Arash Partow, and is used here in 
accordance with its licensing terms. Links: 
[project site](http://www.partow.net/programming/bitmap/index.html) and
[repository](https://github.com/ArashPartow/bitmap).

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
in your command search path.

### How it works

Imgmatch builds a histogram of color values for each image involved in a search. 
It compares image histograms pairwise, calculating a Chi-Squared distance 
measure between the histograms being compared.

A histogram has 4096 bins. Each RGB pixel is mapped to a bin as follows:

* Interpret the bins of the histogram as a three-dimensional cubical array, each
dimension having 16 bins. 

* Given a pixel (r, g, b) with channel values from 0 to 255, scale the values to
the array dimensions, dividing by 16. 

* Interpret the scaled pixel values (r', g', b') as indices into the array.

For a given image, each bin contains the count of pixels in the image that
map to that bin.

When calculating the distance, the bins are interpreted as a linear vector of 
4096 bins. Given images a and b, we have:

* N<sub>a</sub> - the number of pixels in a

* N<sub>b</sub> - the number of pixels in b

* A - the histogram for a, with bins A</sup><sub>i</sub>

* B -  the histogram for b, with bins B</sup><sub>i</sub>

* &#956;<sub>i</sub> = ((A<sub>i</sub> / N<sub>a</sub>) + 
(B<sub>i</sub> / N<sub>b</sub>)) / 2

* distance = &#931;<sub>i</sub> (((A<sub>i</sub>/N<sub>a</sub>) - 
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

### How well does it work?

If images are exact duplicates, it works 100% of the time. Those aren't the
interesting cases, though.

* **Resized images**&mdash;Unless the resizing is egregious, imgmatch works
quite well at finding copies of images that have been resized. Distance numbers
are often around 0.01 or less for JPEG images that have been resampled to 50%
of their original size, when compared to the original image.

* **Rotating or flipping**&mdash;Rotation has no effect on the histogram, so
rotated or flipped images will be considered to be exact matches of their 
originals.

* **Image encoding/format changes**&mdash;This depends, of course on how the
format change was performed. If a JPEG image is rendered into a buffer which is
then saved to a lossless format (e.g., PNG on uncompressed BMP), the distance
will be very close to zero. Going from PNG to jpeg, the result will depend on
how much compression is used to create the JPEG image. Generally, the distance
is low (<0.1) unless the compression is harsh enough to create obvious visible
artifacts.

* **Grayscale compared to RGB**&mdash;When building the histogram for a
grayscale image, imgmatch populates the histogram bins as if each grayscale
pixel were an RGB pixel with equal values in each channel, allowing grayscale
images to be compared with color images. If a color image is completely
de-saturated, saved, and then converted to a grayscale image, those two
images (the de-saturated RGB and the grayscale) will match exactly.

* **Cropping**&mdash;It depends on how much is cropped, and whether the 
histogram of the cropped part is similar to the overall image. An image 
subjected to minor cropping will usually match the original.

* **Watermarks or other artifacts**&mdash;Sometimes images have
been altered by the addition of watermarks or caption text. The extent to which
this affects matching depends on the size and nature of the artifact. Many 
watermarks are subtle&mdash;translucent effects don't effect matching as much
as solid colors.

Different image sets pose different challenges. For example, a sequence of 
images from a fashion modelling shoot will often have the same background, and 
subject material, where the only difference is in poses, or subtle changes to
lighting. Under these circumstances, it's usually best to set the threshold
lower, since the histograms of the images will be closer. If necessary, you can
set the threshold to 0, which will usually only match exact duplicates. 
Of course, it's theoretically possible to have to two significantly different
images with identical (or nearly identical) histograms, but, with images of 
significant size and resolution, the probability is very low.

### TO DO list

I don't know how much time and energy I'll have to commit to these, but here are
some things I'm considering:

* Multi-threading for better performance. The process of building histograms
can take a while for large numbers of large images, but most of that time 
is in image decoding (particularly for JPEG images). The actual cost of 
building histograms themselves is relatively low.

* Porting to Linux and Windows.

* Design a more human-friendly way of browsing and deleting duplicates. Despite
my whining about GUIs, this kind of application really needs one. The current
project started as an experiment using histograms for image comparison. I was
surprised at how well Chi-Squared distance worked for finding duplicates, and 
decided to try making a practical, usable (if less than ideal) solution. I'm
thinking about using node.js to drive a web-based front end, calling the native
C++ code from node to do the calculations. I don't know that world very well, so
it might be a while.