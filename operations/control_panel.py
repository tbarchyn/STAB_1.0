# STAB: subglacial till advection and bedforms
# Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

# Copyright 2014-2016 Thomas E. Barchyn
# Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

# This project was developed with input from Thomas P.F. Dowling,
# Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate
# citation of the relavent publications.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# Please familiarize yourself with the license of this tool, available
# in the distribution with the filename: /docs/license.txt
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Generic setup and operations console for the STAB model
# This code is called from a file called 'stab_*.py', which is placed where the model
# runs are to take place. The * represents the version number.

# set directories
this_dir = os.getcwd()
operations_repository = os.path.join (repository, 'operations')
core_repository = os.path.join (repository, 'bin')
imager_repository = os.path.join (repository, 'imager')
program_localdir = os.path.join (this_dir, 'stab')
operations_localdir = os.path.join (program_localdir, 'operations')
core_localdir = os.path.join (program_localdir, 'bin')
imager_localdir = os.path.join (program_localdir, 'imager')

# processing directories
pending_dir = os.path.join (this_dir, '1_pending')
processing_dir = os.path.join (this_dir, '2_processing')
finished_dir = os.path.join (this_dir, '3_finished')
static_dir = os.path.join (this_dir, 'static')

# set the imager lockfile name
imager_lockfile_name = os.path.join (imager_localdir, 'LOCKED')

# set the viewer to hires (or not)
view_hires = True

# set the globalmappe viewer
make_gm_images = False

def recompile ():
    """
    Function to recompile using the python makefile in the repository
    """
    python_makefile = os.path.join(repository, 'src', 'make.py')
    current_working_dir = os.getcwd()
    os.chdir (os.path.join (repository, 'src'))
    
    # try to recompile the program
    try:
        execfile(python_makefile)
    except:
        print ('ERROR: compile failure')
        pass
        
    os.chdir (current_working_dir)
    return

def get_fresh_code ():
    """
    Function to copy down fresh code from the repository to run locally
    """
    print ('Downloading a fresh version of the program . . '),
    
    # make any directories that are missing
    if not os.path.isdir (program_localdir):
        os.mkdir (program_localdir)
        
    if not os.path.isdir (operations_localdir):
        os.mkdir (operations_localdir)
        
    if not os.path.isdir (core_localdir):
        os.mkdir (core_localdir)
        
    if not os.path.isdir (imager_localdir):
        os.mkdir (imager_localdir)
                
    if not os.path.isdir (pending_dir):
        os.mkdir (pending_dir)

    if not os.path.isdir (processing_dir):
        os.mkdir (processing_dir)
        
    if not os.path.isdir (finished_dir):
        os.mkdir (finished_dir)
        
    if not os.path.isdir (static_dir):
        os.mkdir (static_dir)

        
    # copy down the operations code    
    for files in os.listdir (operations_repository):
        shutil.copy2 (os.path.join(operations_repository, files), os.path.join(operations_localdir, files))
    
    # copy down the program code
    for files in os.listdir (core_repository):
        shutil.copy2 (os.path.join(core_repository, files), os.path.join(core_localdir, files))
    
    # copy down the imager code
    for files in os.listdir (imager_repository):
        shutil.copy2 (os.path.join(imager_repository, files), os.path.join(imager_localdir, files))
    
    print ('complete')
    
    return
    
# MAIN
if __name__ == '__main__':
    try:
        execfile (os.path.join (operations_localdir, 'operations.py'))
    except:
        get_fresh_code()
        execfile (os.path.join (operations_localdir, 'operations.py'))
        
    # load the other functions
    execfile (os.path.join (operations_localdir, 'execute_.py'))
    execfile (os.path.join (operations_localdir, 'simfile.py'))
    
    # see if we are going to batch run
    if batch_run:
        # initialize a batchfile woker silently. There is a problem occasionally
        # where two workers simultaneously grab a file, and the simfile copy fails
        # this attempts to try again 0.1 second later if there is a failure.
        tries = 0
        while tries < 3:
            try:
                # try to init_batchfile worker, but sometimes this fails
                init_batchfile_worker (run_gm_imager = make_gm_images)
                break
            except:
                time.sleep (0.1)            # sleep 0.1 of a second
                tries = tries + 1           # try again
            
    
    else:
        # push the user prompt
        while True:
            print ('------------------------------------------')
            print ('Hi! Welcome to the STAB model operations control panel!')
            print ('Settings:')
            if view_hires:
                print ('  > globalmapper viewer mode: high resolution')
            else:
                print ('  > globalmapper viewer mode: low resolution')
            if make_gm_images:
                print ('  > globalmapper auto image: yes')
            else:
                print ('  > globalmapper auto image: no')
            print ('What would you like me to do?')
            print ('  1: Start a batchfile worker')
            print ('  2: Execute a specific simfile')
            print ('  3: Execute a specific simfile in verbose mode')
            print ('  4: Create gm images for finished simulations (requires globalmapper)')
            print ('  5: Interactively view specific simulation (requires globalmapper)')
            print ('  6: Houseclean')
            print ('  7: Get fresh code')
            print ('  8: Recompile and get fresh code (requires g++)')
            print ('  9: Toggle globalmapper viewer resolution')
            print ('  10: Toggle globalmapper auto imager')
            print ('  anything else: Quit')
            print ('------------------------------------------')
            
            # get the response from the user
            res = raw_input('Enter number:\n')
            try:
                res = int(res)
            except:
                print ('I do not understand . . goodbye for now')
                sys.exit()

            # execute the simfiles pending
            if int(res) == 1:
                init_batchfile_worker(run_gm_imager = make_gm_images)
            
            # execute specific simfile
            elif int(res) == 2:
                res = raw_input ('Enter simfile name:\n')
                execute_simfile (res, verbose = False, run_gm_imager = make_gm_images)

            # execute specific simfile in verbose mode
            elif int(res) == 3:
                res = raw_input ('Enter simfile name:\n')
                execute_simfile (res, verbose = True, run_gm_imager = make_gm_images)

            # run imaging script
            elif int(res) == 4:
                gm_image_finalized_sims()
                
            # interactively image specific simulation
            elif int(res) == 5:
                gm_image_interactive_sim()
            
            # houseclean
            elif int(res) == 6:
                houseclean()
                
            # get fresh code
            elif int(res) == 7:
                shutil.rmtree (program_localdir)
                get_fresh_code()
                
            # recompile and get fresh code
            elif int(res) == 8:
                shutil.rmtree (program_localdir)
                recompile()
                get_fresh_code()
                
            # toggle gm viewer resolution
            elif int(res) == 9:
                if view_hires:
                    view_hires = False
                else:
                    view_hires = True
                    
            # toggle gm viewer auto imager
            elif int(res) == 10:
                if make_gm_images:
                    make_gm_images = False
                else:
                    make_gm_images = True
                
            else:
                break
        

        
        
        
        