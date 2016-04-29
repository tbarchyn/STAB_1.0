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

# This is the lead file for running STAB with the operations code. Take
# this file, modify the repository variable below, and put it somewhere
# on your computer. Then, execute this file, it should copy over a local
# version of the model, and get things set up for running the model.

# Notes: this architecture is legacy from several previous models, and
# works well - but I am looking to replace the simulation management
# code with something generic and better, like dispy, or similar. The trick
# is ensuring fault tolerance and allowing multiple computers to access
# the same simulation queue, which is easy with a queue represented by
# files, and less easy with some other inter-worker communication approach
# like dispy. Files turn out to be relatively robust inter-worker
# communication method!

import os
import shutil
import sys
import time

############################################################
# SET THE PATH TO THE REPOSITORY HERE!
# edit the text between the two single quotation marks!
# to the location of the repository folder, which will be somewhere
# on your computer, similar to this . . yes, use two slashes (//)!
repository = 'C://Users//tom//Dropbox//STAB//1.0//'
############################################################

# set the core name based on platform
if os.name == 'nt':
    mgd_core_basename = 'stab.exe'
if os.name == 'posix':
    mgd_core_basename = 'stab'

# check that the directory exits and throw message to user
# telling them to go edit this document if it doesn't exist
if not os.path.isdir (repository):
    print ('ERROR: I cannot find the repository directory!')
    print ('       the repository path must be set manually in the file')
    print ('       stab_*.py - where * is version number. Presently it is set to:')
    print ('       ' + repository)
    sys.exit()
   
# set repository directory
this_dir = os.getcwd()
operations_repository = os.path.join (repository, 'operations')

if __name__ == '__main__':
    # get a fresh, local copy of the control panel
    shutil.copy2 (os.path.join (operations_repository, 'control_panel.py'), 'control_panel.py')
    
    # execute the control panel
    batch_run = False
    execfile ('control_panel.py')
    
