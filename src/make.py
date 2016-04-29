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

# make.py: this is my python version of my makefile which enables quite a bit more
# flexiblity when it comes to compiling. This will also be cross-platform, which
# cannot be said for makefiles. This only really works with simple make scripts,
# likely more complicated make scripts will need more complicated and proper
# makefiles.

import os
import subprocess
import shutil
import sys

# set the compiler flags
exe_compiler_flags = ['-Wall', '-pedantic']

# ancillary files
ancillary_files = [ 'pthreadGC2.dll',
                    'libgcc_s_dw2-1.dll',
                    'libstdc++-6.dll',
                    'R_main.R',
                    'R_main_noimages.R',
                    'R_color_ramps.R',
                    'R_imager.R',
                    'R_profiler.R',
                    'R_wrappers.R',
                    'R_analysis.R'
                    ]

# set names of 'main' source code files
main = 'stab_main.cpp'

# set the executable names based on platform
if os.name == 'nt':
    exe_name = 'stab.exe'
    local_bindir = 'bin'
elif os.name == 'posix':
    exe_name = 'stab'
    local_bindir = 'bin'
else:
    print ('platform is undefined! platform = ' + os.name)

# set the paths to the locations
bin_path = os.path.join(os.pardir, local_bindir)
exe_out_path = os.path.join (bin_path, exe_name)

# call the compilers
print ('-------------------------------------------------------------------')
print ('Compiling . . ')
if os.name == 'nt':
    exe_call = ['g++'] + exe_compiler_flags + [main] + ['-o'] + [exe_out_path]

if os.name == 'posix':
    exe_call = ['g++ -Wall -pedantic -O1 ' + main + ' -o ' + exe_out_path]
    

ret_1 = subprocess.call (exe_call, shell = True)

if ret_1 == 0:
    print ('Successfully compiled programs')
else:
    sys.exit()
print ('-------------------------------------------------------------------')

# now copy the ancillary files over
print ('Copying ancillary files . . ')
for i in ancillary_files:
    try:
        shutil.copy2 (i, os.path.join (bin_path, i))
        print ('Successfully copied: ' + i)
    except:
        print ('Could not copy: ' + i)

print ('Complete!')
print ('-------------------------------------------------------------------')


