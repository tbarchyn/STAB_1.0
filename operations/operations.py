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

# Basic repository for the operations functions

import os
import shutil
import subprocess
import datetime
import sys
import glob
import time


def grabjob ():
    """
    Grab a job from the pending directory. This function immediately moves the
    simfile from the pending directory to the processing directory and returns 
    the full path to the simfile (now located in the processing directory)
    
    Global variables required:
    pending_dir
    processing_dir
    """
    
    present_dir = os.getcwd()
    
    # move to the processing directory and try to grab a job
    os.chdir (pending_dir)
    
    if len(glob.glob ('*.simfile')) > 0:
        source_simfile_base = glob.glob('*.simfile')[0]     # grab a simfile
        source_simfile = os.path.join (pending_dir, source_simfile_base)
        dest_simfile = os.path.join (processing_dir, source_simfile_base)
        shutil.move (source_simfile, dest_simfile)
        exec_simfile = dest_simfile
        print ('============================================================================')
        print ('Executing ' + exec_simfile)
        
    else:
        exec_simfile = ''
        print ('Cannot find pending job in the queue!')
     
    os.chdir (present_dir) 
    return (exec_simfile)

def finishjob (processing_simfile):
    """
    Once the job is finished, this function moves the simfile from the processing
    directory to finished directory. This function also moves the processed tree
    to the finished directory
    
    Argument: processing_simfile, full pathname of the simfile that has been executed
              and is ready to be moved out of the processing directory
              
    Global variables required:
    processing_dir: the processing directory
    finished_dir: the finished directory
    """
    
    # move the simfile
    simfile_base = os.path.basename (processing_simfile)
    dest_simfile = os.path.join (finished_dir, simfile_base)
    shutil.move (processing_simfile, dest_simfile)
    
    # move the directory
    simfile_base = os.path.basename (processing_simfile) # get the base
    sim_name = simfile_base.replace ('.simfile', '')     # strip .simfile
    exec_dir = os.path.join (processing_dir, sim_name)   # get the execution directory
    dest_dir = os.path.join (finished_dir, sim_name)     # set the directory we are moving to
    
    # try to remove the finished directory if it previously exists
    if os.path.isdir (dest_dir):
        shutil.rmtree (dest_dir)
    
    try:
        shutil.copytree (exec_dir, dest_dir)
        shutil.rmtree (exec_dir)
    except:
        print ('ERROR with copying finished directory from the processing directory for ' + processing_simfile)
    
    
    print ('Finished job: ' + simfile_base)
    return

def gm_view (surf_filename, output_jpeg_filename = '', view = False):
    """
    This function copies the surf raster to the imager directory,
    runs the globalmapper imager, cleans up, and then copies the output jpeg to wherever it is
    specified.
    
    Arguments:
    surf_filename: the full path to the surf_filename
    output_jpeg_filename: the full path to the outputfilename
    view: optional boolean to launch globalmapper and manually inspect the output
   
    Global variables required:
    imager_localdir: the directory to the imager
    imager_lockfile_name: the full imager lockfile name
    
    Notes:
    Globalmapper must be set up for this script to work. Global settings like the vertical 
    exaggeration will affect the outputs. For production image creation, make sure outputs
    look good. This program places an imager lock when it is working.
    """
    
    present_dir = os.getcwd()
    os.chdir (imager_localdir)
    
    # wait for the lock to clear up, print waiting message
    printed_waiting_message = False
    while os.path.isfile (imager_lockfile_name):
        if not printed_waiting_message:
            print ('Waiting for imager '),
            printed_waiting_message = True
            
        time.sleep(1.0)
        print ('.'),        # print dots to show we are still waiting
    
    # once we are here, quickly set the lockfile (there is a remote possibility that another
    # process could set a lockfile between the ending of the while loop and when we write
    # the file . . but this possibility is quite rare (and the error will be obvious)
    f = open (imager_lockfile_name, 'w')
    f.write ('LOCKED: ' + datetime.datetime.now().strftime ("%Y-%m-%d %H:%M"))
    f.close()
    
    # copy the surface and topslabcode rasters into the local directory
    imager_surf_filename = os.path.join (imager_localdir, 'surf.asc')
    #imager_topslabcode_filename = os.path.join (imager_localdir, 'topslabcode.asc')
    imager_output_jpeg = os.path.join (imager_localdir, 'output.jpg')
    
    shutil.copy2 (surf_filename, imager_surf_filename)
    #shutil.copy2 (topslabcode_filename, imager_topslabcode_filename)
    
    # run the imager
    if view:
        print ('Viewing: ' + surf_filename)
        if view_hires:
            os.system (os.path.join (imager_localdir, 'view_hires.gmw'))
        else:
            os.system (os.path.join (imager_localdir, 'view.gmw'))
    else:
        if view_hires:
            os.system (os.path.join (imager_localdir, 'export_jpeg_hires.gms'))
        else:
            os.system (os.path.join (imager_localdir, 'export_jpeg.gms'))
            
        shutil.copy2 (imager_output_jpeg, output_jpeg_filename)              # copy the jpeg
        os.unlink (imager_output_jpeg)                                       # clean up
        
    # clean up the other files
    os.unlink (imager_surf_filename)
    #os.unlink (imager_topslabcode_filename)
    #os.unlink (os.path.join (imager_localdir, 'topslabcode.tif'))
    #os.unlink (os.path.join (imager_localdir, 'topslabcode.tfw'))
    os.unlink (imager_lockfile_name)            # clear the lockfile
    os.chdir(present_dir)
    
    if not view:
        print ('GM_imager output: ' + output_jpeg_filename)
    
    return

def gm_batchimage (target_dir, overwrite = False):
    """
    Function to make images for all the surf and topslabcode files within a given directory
    with the globalmapper imager.
    
    Arguments:
    target_dir: target directory
    overwrite: boolean to control whether we are to overwrite an existing image
    """

    output_image_prefix = '_gm_img_'
    
    present_dir = os.getcwd()
    os.chdir (target_dir)
    
    surf_filenames = glob.glob ('*_surf_*.asc')
    
    for surfname in surf_filenames:
        # get the iteration
        surfname_raw = surfname.split('.')[0]
        surfname_split = surfname_raw.split('_')
        iteration = surfname_split[len(surfname_split) - 1]

        imager_surf_filename = os.path.join (target_dir, surfname)
        #topslabcode_glob = glob.glob ('*topslabcode_' + iteration + '.asc')
        
        # double check that length
        #if len(topslabcode_glob) !=1:
        #    print ('ERROR: cannot find the corresponding topslabcode file to: ' + surfname)
        #    sys.exit()
        
        #imager_topslabcode_filename = os.path.join (target_dir, topslabcode_glob[0])
        output_imagename = os.path.join (target_dir, output_image_prefix + iteration + '.jpg') 
        
        if not os.path.isfile (output_imagename) or overwrite:
            # run the viewer script
            gm_view (imager_surf_filename, output_imagename, view = False)
        
    os.chdir (present_dir)
    return

def init_batchfile_worker (verbose = False, run_gm_imager = False):
    """
    Function to start a batchfile worker. This function first calls the grabjob
    function to get a job from the pending_dir. It then executes the job, tries
    to run the batchimage function (to make a movie) and then when, all is done
    the function then moves the simfile to the finished directory.
    
    verbose = execute the model in verbose mode
    run_gm_imager = run the global mapper imager
    
    Global variables required:
    processing_dir: the processing directory
    core_localdir: the full path to the mgd core
    cubeconfig_dir: the directory containing cubeconfig files
    slabfile_dir: the directory containing the slabfiles
    """

    print ('Creating a new batchfile worker . . ')
       
    files_pending = True      # flag to keep processing
   
    while files_pending:
    
        pending_simfile = grabjob ()        # grab a job
        
        if pending_simfile == '':
            files_pending = False           # set the flag low if no jobs left
        else:
            # we have a job to process, let's run the execute_ function
            exec_dir, ret = execute_ (pending_simfile, core_localdir, processing_dir, verbose)
            
            # we can try to run the batch image function
            if run_gm_imager:
                gm_batchimage (exec_dir, overwrite = True)
            
            # and . . if we have a good return code, finalize this job up
            if ret == 0:
                finishjob (pending_simfile)
            else:
                print ('ERROR: received return code of :' + str(ret) + ' with sim ' + pending_simfile)
        
    return
    
def execute_simfile (simfile, verbose = False, run_gm_imager = False):
    """
    Function to execute a specific simfile from the mgd operations center.
    
    Arguments:
    simfile: the file to be executed
    verbose: execute in verbose mode
    run_gm_imager: run the global mapper batchimage code
    
    Global variables required:
    processing_dir: the processing directory
    core_localdir: the full path to the bin directory
    """
    
    simfile_basename = os.path.basename (simfile)
    pending_simfile = os.path.join (processing_dir, simfile_basename)
    shutil.copy2 (simfile, pending_simfile)
    
    # execute the model
    exec_dir, ret = execute_ (pending_simfile, core_localdir, processing_dir, verbose)

    # try to run the batch image function
    if run_gm_imager:
        gm_batchimage (exec_dir)
    
    # and . . if we have a good return code, finalize this job up
    if ret == 0:
        finishjob (pending_simfile)
    else:
        print ('ERROR: received return code of :' + str(ret) + ' with sim ' + pending_simfile)
    
    return
    
def houseclean ():
    """
    Function to clean up the processing directory. Do not call this function when there
    are batch workers presently operating as you will delete the files underneath the
    batch workers and they will probably fail.
    
    Global variables required:
    pending_dir: pending simfile directory
    processing_dir: processing directory
    """
    present_dir = os.getcwd()
    
    # First, go into the processing directory and move all the simfiles back to the
    # pending directory, these files are probably orphaned.
    os.chdir (processing_dir)
    for simfile in glob.glob ('*.simfile'):
        dest_simfile = os.path.join (pending_dir, simfile)
        shutil.move (simfile, dest_simfile)
        print ('Housecleaning: I moved the following orphaned simfile back to pending: ' + simfile)
     
    # Next, back out of the present directory remove the processing directory (and
    # everything in it), and remake it. Try to do this at least, sometimes fails if
    # there are locks on the any files in the processing directory
    os.chdir (present_dir)
    try:
        shutil.rmtree (processing_dir)
    except:
        pass
    
    # try to remake the processing directory
    try:
        os.mkdir (processing_dir)
    except:
        # if we can't do it, we need fresh code as something is broken
        get_fresh_code()
        sys.exit()
    
    # Next, try to clear any orphaned globalmapper locks
    try:
        os.unlink (imager_lockfile_name)
        print ('Cleared orphaned imager lock')
    except:
        pass
    
    return
    
def gm_image_finalized_sims ():
    """
    Method to image finalized simulations, this doesn't overwrite existing images. The
    idea behind this is to not waste time making images for finalized simulations that
    have already been successfully through the imaging script.
    
    Global arguments required:
    finished_dir: the directory containing finalized sims
    """
    res = raw_input ('Overwrite existing files? (y or n)\n')
    if res == 'y':
        overwrite_existing = True
    else:
        if res == 'n':
            overwrite_existing = False
        else:
            print ('I did not understand your response!')
            return      # else exit
    
    
    sim_dirs = os.listdir (finished_dir)
    for sim_dir in sim_dirs:
        # exclude any files with extensions
        if not len(sim_dir.split('.')) == 2:
            gm_batchimage (os.path.join (finished_dir, sim_dir), overwrite = overwrite_existing)
    
    return
    
def gm_image_interactive_sim ():
    """
    Method to interactively image a certain simulation, image by image with the globalmapper imager
    """
    sim_dirs = os.listdir (finished_dir)
    print ('Choose a finished sim to visualize:')

    # list available sims
    list_numbers = 0
    
    for sim_dir in sim_dirs:
        # exclude any files with extensions
        if not len(sim_dir.split('.')) == 2:
            list_numbers = list_numbers + 1
            print (str(list_numbers) + ': ' + sim_dir)

    res = raw_input ('Enter the number of your choice:\n')
    
    # try to pick the directory with the response, otherwise just return out
    try:
        list_numbers = 0
        for sim_dir in sim_dirs:
            # exclude any files with extensions
            if not len(sim_dir.split('.')) == 2:
                list_numbers = list_numbers + 1
                if list_numbers == int(res):
                    chosen_dir = sim_dir
    
    except:
        print ('I cannot understand: ' + res)
        return
    
    # check the response and then batchimage
    target_dir = os.path.join (finished_dir, chosen_dir)
    if os.path.isdir (target_dir):
        
        present_dir = os.getcwd()
        os.chdir (target_dir)
    
        surf_filenames = glob.glob ('*_surf_*.asc')

        print ('Available files to view:')
        
        while True:
            for surfname in surf_filenames:
                print ('> ' + surfname)
            
            iteration = raw_input ('Enter iteration (e.g., 100) (or press enter to exit): ')
            
            try:
                surf_glob = glob.glob ('*surf_' + iteration + '.asc')
                #topslabcode_glob = glob.glob ('*topslabcode_' + iteration + '.asc')
                
                # double check we can find the files
                if len(surf_glob) !=1:
                    print ('ERROR: cannot find the requested surf file')
                
                #if len(topslabcode_glob) !=1:
                #    print ('ERROR: cannot find the requested topslabcode file')
                
                imager_surf_filename = os.path.join (target_dir, surf_glob[0])
                #imager_topslabcode_filename = os.path.join (target_dir, topslabcode_glob[0])

                output_imagename = ''           # placeholder for the view function call
                
                gm_view (imager_surf_filename, output_imagename, view = True)
            
            except:
                break
        
        os.chdir (present_dir)
        
    return

    
