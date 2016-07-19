# STAB: subglacial till advection and bedforms
# Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

# Copyright 2014-2016 Thomas E. Barchyn
# Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

# This project was developed with input from Thomas P.F. Dowling,
# Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate
# citation of the relavent publications.

# Barchyn, T. E., T. P. F. Dowling, C. R. Stokes, and C. H. Hugenholtz (2016), 
# Subglacial bed form morphology controlled by ice speed and sediment thickness,
# Geophys. Res. Lett., 43, doi:10.1002/2016GL069558

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

# R wrapper functions that execute specific functions

suppressPackageStartupMessages(library (raster))
suppressPackageStartupMessages(library (scales))
if (.Platform$OS.type == 'windows') {
    suppressPackageStartupMessages(library (rgdal))
}


source ('R_imager.R')               # imager functions
source ('R_profiler.R')             # profiler functions    
source ('R_analysis.R')             # analysis functions

process_file <- function (iter) {
    # function to process a specific file mid simulation
    # Argument:
    # iter = the character representation of the iteration to process
    
    # evaluate the options, read in the specified raster and call specified analyses
    if (make_images | make_surf_profiles | run_analysis) {
        # read in the surface file (we need this for almost every analysis)
        surf <- read_raster (iter, file_output_prefix, 'surf', slope_aspect = TRUE)
        
        # read in other ancillary files (if necessary)
        if (make_basal_pres_image) {
            basal_pres <- read_raster (iter, file_output_prefix, 'pres', slope_aspect = FALSE)
        } else {
            basal_pres <- NA
        }
        
        if (make_ice_cavity_image) {
            ice <- read_raster (iter, file_output_prefix, 'ice', slope_aspect = FALSE)
            ice_cavity <- ice - surf[[1]]          # calculate the cavity
        } else {
            ice_cavity <- NA
        }
        
        if (make_ice_load_image) {
            ice_load <- read_raster (iter, file_output_prefix, 'iceload', slope_aspect = FALSE)
        } else {
            ice_load <- NA
        }
        
        if (make_sed_depth_image | run_analysis) {
            bsmt <- read_raster (iter, file_output_prefix, 'bsmt', slope_aspect = FALSE)
            sed_depth = surf[[1]] - bsmt
        } else {
            bsmt <- NA
            sed_depth = NA
        }
        
        # call specific functions to perform analysis
        if (make_images) {
            plot_images (iter, surf, basal_pres, ice_cavity, ice_load, sed_depth) # plot images
        }
        if (make_surf_profiles) {
            call_profiler (surf[[1]], iter, rownum, 'surf')         # call profiler
        }
        if (run_analysis) {
            call_analyser (surf, bsmt, iter, 1.0)              # call topo analyser
        }
    }
}

process_all <- function () {
    # function to process all the files at the end of the simulation.
    # This function finds all the 'surf' files in the local directory
    # and processes each individually.

    surf_list <- list.files (pattern = '_surf_')
    surf_list_split <- strsplit (surf_list, '[.]')
    
    for (i in surf_list_split) {
        # first make sure that we only process ascii files
        if (i[2] == 'asc') {
            # get the iteration
            temp_split_1 <- strsplit (i[1], '_')
            temp_split_2 <- temp_split_1[[1]]
            iter <- temp_split_2[length(temp_split_2)]
            print (paste ('analysing iteration:', iter))
            process_file (iter)
        }
    }
}

finalize_all <- function () {
    # function to call all finalization code. This should be called after simulation is
    # complete, and whether the process code was called after each output, or all at
    # once with the process_all function. Generally, this coallates output files and
    # executes all plotting code.
    
    print ('finalizing analysis')
    if (make_surf_profiles) {
        finalize_profiler (rownum, 'surf')                # plot surf profiles
    }
    
    if (run_analysis) {
        finalize_analyser ()                              # finalize analysis code
    }
}

read_raster <- function (iter, file_output_prefix, name, slope_aspect) {
    # utility function to read a raster and return a raster object. There is an option to calculate
    # slope and aspect and make a brick of rasters where the surf is band 1, slope is band
    # 2, and aspect is band 3. Slope and aspect are returned in radians.
    
    # Arguments:
    # iter = the character iteration
    # file_output_prefix = the prefix on the file names
    # name = the name of the file to read
    # slope_aspect = boolean to also calculate slope and aspect
    
    # start by constructing the filenames
    rastername <- paste (file_output_prefix, '_', name, '_',  iter, '.asc', sep = '')
    
    # next see if a search finds the file
    if (length (list.files(pattern = rastername)) != 1) {
        ret_raster <- NA                # return NA
        print (paste ('ERROR: cannot find the rasterfile:', rastername))
    } else {
        # read in the file
        ret_raster <- raster (rastername)
        fake_utm_proj <- CRS('+proj=utm +zone=1 +north +ellps=WGS84 +datum=WGS84 +no_defs')
        projection (ret_raster) <- fake_utm_proj
        
        # calculate slope and aspect (in radians)
        if (slope_aspect) {
            slope <- terrain (ret_raster, opt = 'slope')
            aspect <- terrain (ret_raster, opt = 'aspect')
            ret_raster <- brick (ret_raster, slope, aspect)
        }
    }
    return (ret_raster)
}
 
main <- function (char_iter) {
    # main function to call all analysis and plotting code
    # Argument:
    # char_iter: the iteration flag provides a flag to perform specific analysis.
    # The stab program calls this script with the following flags:
    # any value > 0: process a specific iteration (used for 'on the fly' imaging)
    # -1: plot images for all available iterations
    # -2: plot any final images and conduct any final analysis

    if (char_iter == '-1') {
        process_all ()
    } else if (char_iter == '-2') {
        finalize_all ()
    } else {
        process_file (char_iter)
    }
}




