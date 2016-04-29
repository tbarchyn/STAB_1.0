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

# R imager to make images of the model space

source ('R_color_ramps.R')          # get some color ramp definitions
                                    # replace this file with specific definitions
                                    # if desired. Whatever is local is read.

plot_sim <- function (hshade, overlay, label, colvector, bsmt_exposure, block_basement) {
    # function to push an image to active plotter device of the surface with some overlay
    # this is called repeatedly for the different possible overlays.
    
    # Arguments:
    # hshade = hillshade pre-calculated
    # overlay = the overlay raster
    # label = the label to put at the top of the plot
    # colvector = the color ramp to use to plot the overlay
    # bsmt exposure = a raster to denote basement exposure
    # block_basement = a boolean to control whether to block overtop basement

    # plot the hillshade
    suppressPackageStartupMessages (
        plot (hshade, col = grey(0:100/100), legend = F, main = label, axes = F, box = F)
    )
    
    # now we need to plot the overlay
    suppressPackageStartupMessages (
        plot (overlay, col = colvector, add = T, axes = F, box = F, legend = T)  
    )
    
    if (block_basement) {
        # plot the basement exposure overlay
        suppressPackageStartupMessages (
            plot (bsmt_exposure, add = T, axes = F, box = F, legend = F)
        )
    }
}

plot_images <- function (iteration, surf, basal_pres, ice_cavity, ice_load, sed_depth) {
    # function to make images in a function call. Note that this relies upon a bunch of
    # accessible variables to run. These variables need to be set in the R plot
    # progress file.
    
    # Arguments:
    # iteration = the iteration used to construct the filenames to make the images (character)
    # surf = the raster brick of surface, slope, aspect
    # basal_pres = the basal pres raster object
    # ice_cavity = the ice cavity raster
    # ice_load = the ice load raster
    # sed_depth = the sediment depth raster 
    
    # set the plotting parameters
    alpha_transparency <- 0.55                     # set transparency of overlays
    
    # calculate hillshade
    hshade <- hillShade (surf[[2]], surf[[3]], angle = 30.0, direction = 315.0)
    
    # make basement exposure layer to overlay
    bsmt_exposure <- sed_depth
    bsmt_exposure <- bsmt_exposure * NA
    if (add_bsmt_exposure) {
        bsmt_exposure[sed_depth < 0.0000000001] <- alpha ('brown', 0.8)
    }
    
    # make the images!
    if (make_basal_pres_image) {
        title_label <- paste ('iteration =', iteration, ', time =', as.numeric(iteration) * timestep_len, 
                              'a (timestep_len = ', timestep_len, 'a) , parameter = basal_pres')
        oput_filename <- '_basal_pres_current.jpeg'
        colvector <- set_color_ramp ('pres', alpha_transparency)

        jpeg (filename = oput_filename, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot_sim (hshade, basal_pres, title_label, colvector, bsmt_exposure, FALSE)
        placeholder <- dev.off()
        
        if (archive_basal_pres_image) {
            archive_filename <- paste (file_output_prefix, '_basal_pres_', iteration, '.jpeg', sep = '') 
            placeholder <- file.copy (oput_filename, archive_filename)
        }
    }
    
    if (make_ice_cavity_image) {
        title_label <- paste ('iteration =', iteration, ', time =', as.numeric(iteration) * timestep_len, 
                              'a (timestep_len = ', timestep_len, 'a) , parameter = ice_cavity')
        oput_filename <- '_ice_cavity_current.jpeg'
        colvector <- set_color_ramp ('ice_cavity', alpha_transparency)

        jpeg (filename = oput_filename, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot_sim (hshade, ice_cavity, title_label, colvector, bsmt_exposure, FALSE)
        placeholder <- dev.off()
        
        if (archive_ice_cavity_image) {
            archive_filename <- paste (file_output_prefix, '_ice_cavity_', iteration, '.jpeg', sep = '') 
            placeholder <- file.copy (oput_filename, archive_filename)
        }
    }
    
    if (make_ice_load_image) {
        title_label <- paste ('iteration =', iteration, ', time =', as.numeric(iteration) * timestep_len, 
                              'a (timestep_len = ', timestep_len, 'a) , parameter = ice_load')
        oput_filename <- '_ice_load_current.jpeg'
        colvector <- set_color_ramp ('ice_load', alpha_transparency)

        jpeg (filename = oput_filename, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot_sim (hshade, ice_load, title_label, colvector, bsmt_exposure, FALSE)
        placeholder <- dev.off()
        
        if (archive_ice_load_image) {
            archive_filename <- paste (file_output_prefix, '_ice_load_', iteration, '.jpeg', sep = '') 
            placeholder <- file.copy (oput_filename, archive_filename)
        }
    }
    
    if (make_surf_image) {
        title_label <- paste ('iteration =', iteration, ', time =', as.numeric(iteration) * timestep_len, 
                              'a (timestep_len = ', timestep_len, 'a) , parameter = surface')
        oput_filename <- '_surface_current.jpeg'
        colvector <- set_color_ramp ('surf', alpha_transparency)

        jpeg (filename = oput_filename, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot_sim (hshade, surf[[1]], title_label, colvector, bsmt_exposure, FALSE)
        placeholder <- dev.off()
        
        if (archive_surf_image) {
            archive_filename <- paste (file_output_prefix, '_surface_', iteration, '.jpeg', sep = '') 
            placeholder <- file.copy (oput_filename, archive_filename)
        }
    }
    
    if (make_sed_depth_image) {
        title_label <- paste ('iteration =', iteration, ', time =', as.numeric(iteration) * timestep_len, 
                              'a (timestep_len = ', timestep_len, 'a) , parameter = sed_depth')
        oput_filename <- '_sed_depth_current.jpeg'
        colvector <- set_color_ramp ('sed_depth', alpha_transparency)

        jpeg (filename = oput_filename, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot_sim (hshade, sed_depth, title_label, colvector, bsmt_exposure, TRUE)
        placeholder <- dev.off()
        
        if (archive_sed_depth_image) {
            archive_filename <- paste (file_output_prefix, '_sed_depth_', iteration, '.jpeg', sep = '') 
            placeholder <- file.copy (oput_filename, archive_filename)
        }
    }    
}




