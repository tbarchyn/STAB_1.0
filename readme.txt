STAB - the subglacial till advection and bedforms model

Copyright 2014-2016 Thomas E. Barchyn (tbarchyn@gmail.com)

Please review the license file (COPYING) prior to using this program.

Dependencies:
STAB consists of 3 components. First, there is Python operations code, which manages the simulation
queue and assembles result files. Second, there is a C++ compiled model core, which does the actual
simulations. Third, there is R code, which makes images and does post processing analysis.

Thus you need the following on your computer:
1) Python 2.X, with the regular set of packages that come with a base installation (I believe
   I have not imported anything that needs to be installed separately). Refer to the import statements
   in the operations code for clarification. Python will need to be added to the system path if
   not already.

2) A C++ compiler. I use g++ on windows (with mingw), and linux. I have not used any other compiler,
   so let me know if you use another compiler and have success / failure. Note the 'compiling notes'
   in 'stab_main.cpp'. If using another compiler, you will need to edit 'make.py' in the 'src'
   directory.
   
3) R, with the following packages: 'rgdal', 'raster', 'scales', and the dependencies required for these
   packages. Note that on Windows you may need to add R to the system path, or set the Rscript_path
   variable in your simfiles to the absolute path to 'Rscript.exe' (note a need for quotes to deal
   with the spaces that inevitably come up with R installations that end up in the 'Program Files'
   directory).
   
If you have a version of GlobalMapper, you can use the viewer with GlobalMapper, but this is not
necessary or recommended. Contact me if you really want to use GlobalMapper - I've put GlobalMapper on
retirement notice here - I just haven't fully extricated it from my modeling workflows.

Quickstart operation:
This program is intended to operate relatively seamlessly with a 'repository - operations' sort of
architecture. The repository is meant to stay un-altered, while you go ahead and run simulations in
the operations directory, which is somewhere else on your computer.

Copy the file 'stab_10.py', where the 10 is the version number, and may change. Put this file somewhere
where you wish to run your simulations. Edit the file where it mentions the repository absolute
file path so that it points to the repository location. Execute the file, and it should download a
copy of the program from the repository, and set up 3 empty directories: '1_pending', '2_processing',
and '3_finished'.

The simfiles, which contain all the parameters for running the program, are placed into '1_pending'
and then executed sequentially until there are none left. During simulation, the operations code
assembles the requisite files, executes the simulation in'2_processing', then copies the simulation into the
'3_finished' directory. The worker then goes back and tries to find the next simfile. There can
be many workers all working on the same queue of simfiles, running in parallel. The model cannot
be parallelized that well because of all the sequential polling required - but you can run as many
simulations in parallel as you please (the 'poor man's parallelization'!). As communication about
the state of the queue is done with files, one can use many different computers (e.g., a cluster)
to work away at a queue. This will eventually be replaced with something better - but I will admit
that communicating between workers with files is rather robust and flexible.

Here are your options when executing 'stab_10.py'
1) Start batchfile worker -> this starts working on the simulation queue, simfile by simfile
2) Execute a specific simfile -> type the path to the simfile to execute
3) Execute a specific simfile in verbose mode -> same as 2, except verbose
4) Create gm images . . -> this is a legacy feature that I'm going to depreciate soon
5) Interactively view specific simulation -> this opens a specific iteration from a specific simulation
    that is present in '3_finished' for extra detailed visualization in Globalmapper
6) Houseclean -> this deletes anything present in '2_processing' directory and takes the simfiles back
    to '1_pending'. Useful if there are crashes
7) Get fresh code -> downloads fresh code from the repository
8) Recompile and get fresh code -> same as above, except calls 'make.py' in the 'src' directory
9) Toggle globalmapper viewer resolution -> legacy feature slated for retirement
10) Toggle globalmapper auto imager -> legacy feature slated for retirement
anything else: Quit -> self explanatory . .

To run simulations, you can use the test simfile and put it in the '1_pending' folder to run,
or contact me for some ensembles of simfiles. Or write your own scripts using the 'simfile.py'
simfile class to factory out ensembles -> read in a prototype, modify the parameter(s) (see methods
in the simfile class) -> write out an ensemble member -> repeat.

Please let me know your successes/failures with this - I require user numbers or actual problem
reports to justify the time improving the useability of this program. As noted in the readme.md file I
am happy working with the program in a raw form, but I wrote the thing and know what it does. If
you would like something that is easier to use, I need your encouragement and reports about the
'difficult to use' parts. Heck, I'll even buy you a drink of your choice if I see you at a conference!
Thanks!









