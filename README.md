fiqrogue
========

FIQRogue is intended to be an experimental roguelike for learning purposes and to gain the experience of writing a C project entirely from scratch, rather than just working on already existing projects.

Dependecies for building
========================

You will need Perl and a C compiler to be able to build the code


Building
========

General
-------

You can try out the application by doing the following.

Linux/Mac
---------

Run the following commands in a shell

    $ git clone https://github.com/FredrIQ/fiqrogue.git # clone the repository
    $ cd fiqrogue
    $ mkdir build # create a build repository
    $ cd build
    $ ../aimake -i /path/to/installdir

Where installdir is where you want to install the application.


Windows
-------

I don't own a Windows desktop. Sorry (TODO).


Running
=======

To run the application, either do the following:

Terminal
--------

Open a terminal and run the following commands

    $ cd /path/to/installdir
    $ ./fiqrogue # terminal (curses) version
    OR
    $ ./fiqrogue-sdl # graphical version


GUI
---

Open the installation directory in your file explorer of choice, and run fiqrogue-sdl.
