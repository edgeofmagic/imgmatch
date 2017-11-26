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
We'll stipulate that aardvark.jpg is the same image as anteater.jpg, and tortise.jpg 
is the same image as turtle.jpg. They need not be exact duplicates&mdash;aardvark.jpg 
could be a resized copy of anteater.jpg, or tortise.jpg could be a copy of turtle.jpg 
with increased JPEG compression, for example.

Run the imgmatch command:
```` bash
$ imgmatch
````
Imgmatch creates a subdirectory named 'matches':
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
These contain symbolic links to sets of files that appear to be duplicate images:
```` bash
$ ls matches/m0
aardvark.jpg
anteater.jpg
$ ls matches/m1
tortise.jpg
turtle.jpg
````
Note: these are not copies of the files in the 'images' directory; they are symbolic links to the actual image files.

Using your preferred image viewer, browse these links. This is typically done by 
clicking on the links in Finder (macOS). If they are, in your judgement, duplicates 
(imgmatch occasionally produces false positive results) decide which you want to 
keep and delete the rest.

Important: don't delete the symbolic links in 'm0' and 'm1'. Delete the original 
image files in the parent directory 'images'. Let's say we decided to keep 
aardvark.jpg and tortise.jpg:

```` bash
$ ls
aardvark.jpg
anteater.jpg
giraffe.jpg
matches
tortise.jpg
turtle.jpg
zebra.jpg
$ rm anteater.jpg turtle.jpg
````

Now delete 'matches' and its subdirectories to clean up the links:

```` bash
$ rm -rf matches
$ ls
aardvark.jpg
giraffe.jpg
tortise.jpg
zebra.jpg
````
Some viewers make the task of deleting selected files much easier, avoiding the need to delete 
them from the command line. For example, Xee (a popular viewer for macOS) allows 
you to delete the image currently being displayed (command+delete or 
File > Move to Trash). Even if you opened the image from a symbolic link, 
this will delete the actual image file (the target of the link).

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
$ imgmatch --target aardvark.jpg
````
or
````bash
$ imgmatch -t image_folder
````

\[Note that options have both a long form&mdash;two dashes followed by a 
name&mdash;and a short form&mdash;a single dash followed by an alphabetic 
character.\]

If the target is a JPEG, PNG, or BMP image, it will be compared with the 
contents of the search directories. If the target is a directory, all of the 
image files contained in the target directory are compared against images in 
the search directories.

If no target is specified, imgmatch finds matching images within each search 
directory&mdash;it does't compare images between search directories if no 
target is specified.

#### Example

Let's use a slightly different scenario from the example above. The same files 
are split into two directories&mdash;'images' and 'more_images':

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

Imgmatch will compare the files in 'images' with the files in 'more_images'. 
By default, the results will be put in directory named 'matches', which will 
be created in the current working directory when the imgmatch command was run:

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

> "There is no command line interface so bad 
> that it can't be made worse by adding a GUI"
&mdash;Me

I *really* didn't want to add a GUI. I've used image duplicate-finding
applications with GUIs, and their built-in image viewers are never as good as
stand-alone viewers that are freely available on most platforms. Nevertheless,
when potential matches are found, it's usually necessary to compare them visually
before deciding what to keep and what to delete. The first version of imgmatch
simply generated output text listing the matching files. The process of
navigating to those files in a Finder window so I could open them with my
favorite viewer (Xee) proved awkward and frustrating, particularly if the
files were in different directories, or the containing directories were large.

The solution I settled on is simple, and it provides a relatively smooth
workflow when de-duping large image sets. When a set of files (2 or more)
appear to be the same, imgmatch creates a directory and puts symbolic links to 
those files in the that directory. These directories (I'll call them 'match set'
directories) are created in the *results directory*. By default, imgmatch
will try to create a subdirectory of the current working directory called 
'matches'&mdash;this is the *results* directory.
Individual match set directories are created under the results directory, with 
names 'm0', 'm1', 'm2', and so on, depending on how many sets of matching images
are found. Now, symbolic links to matching files are all together. You simply
navigate to the results directory, open the match set directories and click on
links (which should open the linked images in your default image viewer). As
mentioned above, I reccommend using Xee. In addition to being an all-around 
good viewer, you can delete the linked image directly from the viewer. Other
viewers may support this, too. Preview (macOS) does not.

If you end up deleting from the command line, the symbolic links make that 
a bit easier, too. From the previous example, we have a set of links to matching
files in ~/images/matches/m0:

```` bash
$ cd ~/images/matches/m0
aardvark.jpg
anteater.jpg
````
and we want to delete the image ~/images/anteater.jpg. You can inspect the 
link to see its target with the **readlink** command:

```` bash
$ cd matches/m0
$ readlink anteater.jpg
/Users/my_user_name/images/anteater.jpg
````
\[Note: this version of readlink is macOS-specific; more on this 
in a minute.\]

Imgmatch creates a link to the canonical path of the file. You can incorporate
**readlink** into the **rm** command like this:

$ rm `readlink -n anteater.jpg`

and this will delete image file itself, not the link. ***But you really don't 
want to do it this way.*** Instead, add this function definition to your
.bash_profile or .bashrc file:

````bash
rmlt() {
    echo rm `readlink -n $1`
    rm `readlink -n $1`
}
````
Now you can delete the image file like this:

````bash
rmlt anteater.jpg
````

I called mine **rmlt** for *remove link target*. Name yours whatever you want.

You may prefer to use the GNU version of readlink, available from Homebrew or
MacPorts in the GNU coreutils package. It gets renamed to **greadlink**
to avoid hiding the native macOS readlink. If so, I recommend the following:

````bash
rmlt() {
    echo rm `greadlink -f $1`
    rm `greadlink -f $1`
}
````

This -f flag isn't necssary for links generated by imgmatch, since they are
absolute links to the canonical file path, but this version will work with any 
symbolic links, even if they are relative, or go through multiple layers of 
linking.

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

Examples:

Assume we start in our home directory with no 'matches' subdirectory:

```` bash
$ cd ~
$ ls
images
more_images
````

If we run imgmatch, it will create the directory './matches':

```` bash
$ imgmatch images more_images
$ ls
images
matches
more_images
````
Assuming that 'matches' is not empty, if we run imgmatch again, it will create 
a new subdirectory named 'matches0':

```` bash
$ imgmatch images more_images
$ ls
images
matches
matches0
more_images
````
If we keep doing this, imgmatch will create './matches1', './matches2', and so on.

We can explicity specify a different path:

```` bash
$ rm -rf matches*
$ imgmatch images more_images --results match_results
$ ls
images
match_results
more_images
````
If the results path already exists, and is not an empty directory, imgmatch 
will use the same name-generation strategy shown earlier with 'matches':

```` bash
$ imgmatch images more_images --results match_results
$ ls
images
match_results
match_results0
more_images
````
#### Set Limit
**--limit** *num* <br/>
**-l** *num*

Sets the maximum number of images in each search directory that will be compared 
with the target. If a target directory is specified, limit also sets the maximum number of images in the target directory that will be compared with search directory contents. The default value is 1000.

#### Set Match Threshold
**--match** *num* <br/>
**-m** *num*

Sets the match threshold, where *num* is a non-negative floating-point number. 
File comparisons generate a numeric distance value. If two images are identical, the distance will be 0. The match option sets threshold at which two compared files will be considered a match. The default value is 0.1. 

#### Set Verbosity Level
**--verbose** \[*num*\] <br/>
**-v** \[*num*\]

Sets the level of verbosity for generated commentary. Valid values are 0, 1 and 
2. Level 0 produces no output unless an error condition is occurred. Level 1 
produces a moderate amount, announcing the target and search directories as 
they are being searched, and announcing matches when found. Level 2 produces 
voluminous output. For example, it announces every comparison as it is being 
made, along with the resuling correlation coefficient from the comparison. The 
default level is 0. If the verbose option is specified but no level is given, 
the level will be set to 1.

### Build and install

