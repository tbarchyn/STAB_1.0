STAB - the subglacial till advection and bedforms model

Copyright 2014-2106 Thomas E. Barchyn (tbarchyn@gmail.com)

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
totally necessary. There is a free version of GlobalMapper out there that would work for this I believe
as we are only dealing with a few files (please let me know if this is successful if you try).

Quickstart operation:
This program is intended to operate relatively seamlessly with a 'repository - operations' sort of
architecture. The repository is meant to stay un-altered, while you go ahead and run simulations in
the operations directory, which is somewhere else on your computer.

Copy the file 'stab_10.py', where the 10 is the version number, and may change. Put this file somewhere
where you wish to run your simulations. Edit the file where it mentions the repository absolute
file path so that it points to the repository location. Execute the file, and it should download a
copy of the program from the repository, and set up 3 empty directories: '1_pending', '2_processing',
and '3_finished'. Edit the simfiles for each simulation, and put them in the '1_pending' folder. Follow the 
instructions when executing 'stab_10.py' folder to go ahead and 'run batch file'. This will run
the simulations associated with the simfiles, and eventually put the results into '3_finished'.
More than one worker can be started with separate shells, or even on separate computers if the operations
directory is shared. Or on a cluster computer, the shared simulation queue can be accessed by tens
or hundreds of workers.

To compile the program with the python makefile, press the option for 'Recompile and get fresh code'.
This executes 'make.py' in the 'src' directory. Please review this file for more details and discussion
about compiling.

Please let me know your successes/failures with this - I require user numbers or actual problem
reports to justify the time improving the useability of this program. As noted in the readme.md file I
am happy working with the program in a raw form, but I wrote the thing and know what it does. If
you would like something that is easier to use, I need your encouragement and reports about the
'difficult to use' parts. Heck, I'll even buy you a drink of your choice if I see you at a conference!
Thanks!









