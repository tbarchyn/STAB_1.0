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

import os
import shutil
import subprocess

def execute_ (simfile_in, stab_bin_dir, processing_dir_in = '', verbose = False):
    """ 
    This function is the main python wrapper to execute the model.
    
    This function has explicit directory arguments so that it can be portable out of the
    local batchfile program (e.g., use on westgrid)
    
    simfile_in = this is the name of the simfile which we are going to use (must be full path)
    stab_bin_dir = this is the path to the bin directory, which contains all the files needed locally
    processing_dir_in = the directory to base the processing (defaults to present dir)
    verbose = execute the model in verbose mode
    """
    
    if os.name == 'nt':
        stab_corename = 'stab.exe'            # windows corename
    elif os.name == 'posix':
        stab_corename = 'stab'
    else:
        print ('platform is undefined! platform = ' + os.name)
        sys.exit()
    
    # first thing we need to do is create a directory for the job, which will be the simfile
    # name minus the extension '.simfile'
    simfile_base = os.path.basename (simfile_in)               # get the base
    sim_name = simfile_base.replace ('.simfile', '')           # strip .simfile
    exec_dir = os.path.join (processing_dir_in, sim_name)      # set the execution directory

    # make the exec_dir, but allow it to previously exist (if re-running simulations)
    if not os.path.isdir (exec_dir):
        os.mkdir (exec_dir)
            
    exec_simfile = os.path.join (exec_dir, simfile_base)
    
    shutil.copy2 (simfile_in, exec_simfile)

    # next, we need to copy in the contents of the 'bin directory'
    bin_directory_list = os.listdir (stab_bin_dir)
    for f in bin_directory_list:
        source_file = os.path.join (stab_bin_dir, f)
        dest_file = os.path.join (exec_dir, f)
        shutil.copy2 (source_file, dest_file)
        
    # change the directories to the exec_dir in preparation for running the program
    present_dir = os.getcwd()
    os.chdir (exec_dir)
    
    # call the program, verbose and non-verbose modes
    if verbose:
        if os.name == 'nt':
            ret = subprocess.call ([stab_corename, exec_simfile, '-v'], shell = True)
        if os.name == 'posix':
            ret = subprocess.call ('./' + stab_corename + ' ' + exec_simfile + ' -v', shell = True)
    else:
        if os.name == 'nt':
            ret = subprocess.call ([stab_corename, exec_simfile], shell = True)
        if os.name == 'posix':
            ret = subprocess.call ('./' + stab_corename + ' ' + exec_simfile, shell = True)
            
    # back to the original directory
    os.chdir (present_dir)
    
    return (exec_dir, ret)
    




