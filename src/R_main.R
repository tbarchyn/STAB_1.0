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

# R head wrapper to be called from the STAB engine. This main sets a number of
# local variables that are required in the subsequent programs.

source ('R_wrappers.R')                 # wrapper functions called from here

# get arguments from command line (as passed down from MGD/STAB)
args <- commandArgs(TRUE)
file_output_prefix <- args[1]           # file output prefix (so we can find the files)
ydim <- as.numeric (args[2])            # y dimensions of the model space
xdim <- as.numeric (args[3])            # x dimensions of the model space
char_iter <- args[4]                    # get iteration from the program as character string!

# set imaging actions
make_images <- TRUE                     # make images
make_basal_pres_image <- FALSE          # make a basal_pres image
archive_basal_pres_image <- FALSE       # also copy the image and relabel based on iteration
make_ice_cavity_image <- FALSE          # make a ice cavity image
archive_ice_cavity_image <- FALSE       # also copy the image and relabel based on iteration
make_ice_load_image <- FALSE            # make a ice load image
archive_ice_load_image <- FALSE         # also copy the image and relabel based on iteration
make_surf_image <- TRUE                 # make a surface image
archive_surf_image <- TRUE              # also copy the image and relabel based on iteration
make_sed_depth_image <- TRUE            # make a sediment depth image
archive_sed_depth_image <- TRUE         # also copy the image and relabel based on iteration

add_bsmt_exposure <- TRUE               # add a basement exposure to sed_depth images to highlight bsmt

# set analysis actions
make_surf_profiles <- TRUE              # make surface profiles
rownum <- floor (ydim / 2)              # rownumber for the profile
run_analysis <- TRUE                    # run analysis and topography description code

# hardcode timestep length (need to make this better at some point)
timestep_len <- 0.01

# set global large and small modelspace plotting details
if (ydim < 200 | xdim < 200) {
    jpeg_res_image <- 200.0             # modelspace resolution in pixels per inch
    jpeg_width_image <- 8.0             # modelspace plot size in inches
    jpeg_height_image <- 8.0            # modelspace plot size in inches
} else {
    jpeg_res_image <- 300.0             # modelspace resolution in pixels per inch
    jpeg_width_image <- xdim * 0.02     # modelspace plot size in inches
    jpeg_height_image <- ydim * 0.02    # modelspace plot size in inches
}

# CALL MAIN
main (char_iter)

    

