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

# R profiler to make profiles of the model space


add_profile <- function (ras, iteration, rownum, file_prefix) {
    # function to cut a profile of the model space in the ew direction
    # and add the profile to the list of profiles. There are two files
    # that are progressively updated: one is the actual profiles, which
    # is the raster values along the profile. The other is the key, which
    # lists the files and iterations associated.
    
    # Arguments:
    # ras = the raster to be profiled
    # iteration = the iteration of the profile
    # rownum = the rownumber of the profile
    # file_prefix = a prefix to label the profile log files
    
    keyfile <- paste (file_prefix, rownum, 'key.csv', sep = '_')
    profilefile <- paste (file_prefix, rownum, 'prof.csv', sep = '_')
       
    # obtain a vector at the rownum and get the length required
    vec <- ras[rownum, ]
    vec_len <- length (vec)

    # check to see if we have files already, otherwise read existing files
    if (!file.exists (keyfile)) {
        key <- data.frame (id = NA, iteration = NA)
        key <- key [-1,]
        write.table (key, keyfile, row.names = F, col.names = T, sep = ',')
    } else {
        key <- read.table (keyfile, header = T, sep = ',')
        key$iteration <- as.character (key$iteration)
    }
    
    if (!file.exists (profilefile)) {
        prof <- data.frame (matrix (nrow = 0, ncol = vec_len))
        write.table (prof, profilefile, row.names = F, col.names = F, sep = ',')
    } else {
        prof <- read.table (profilefile, sep = ',')
    }
    
    # append the record to the key
    key [nrow(key) + 1, ] <- c(nrow(key) + 1, iteration)
    
    # append the profile to the profilefile
    prof[nrow(prof) + 1, ] <- vec
    
    # write the files
    write.table (key, keyfile, row.names = F, col.names = T, sep = ',')
    write.table (prof, profilefile, row.names = F, col.names = F, sep = ',')
}

plot_profiles <- function (rownum, file_prefix) {
    # function to make plots of the profiles
    
    # Arguments:
    # rownum = the rownumber of the profile
    # file_prefix = a prefix to label the profile log files
    
    # set the filenames and read in the files
    keyfile <- paste (file_prefix, rownum, 'key.csv', sep = '_')
    profilefile <- paste (file_prefix, rownum, 'prof.csv', sep = '_')
    plotfile <- paste (file_prefix, rownum, 'plot.jpeg', sep = '_')
    key <- read.table (keyfile, header = T, sep = ',')
    prof <- read.table (profilefile, sep = ',')

    # determine the maximum and minimum values to plot, and nrows
    minval <- min(prof, na.rm = T)
    maxval <- max(prof, na.rm = T)
    num_profiles <- nrow(key)
    
    # sort the key dataframe by iteration (which reads in numeric by default)
    key <- key[order(key$iteration), ]
    
    # plot the base and set up the plot space
    jpeg (filename = plotfile, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
    plot (as.vector(as.matrix(prof[1,])), cex = 0.0, ylim = c(minval, maxval), main = plotfile, 
                            ylab = 'values', xlab = 'cells')
    plot_colors <- colorRampPalette (c('blue', 'red')) (num_profiles)

    # loop through the profiles
    for (i in 1:num_profiles) {
        plot_id <- key$id[i]
        lines (as.vector(as.matrix(prof[plot_id,])), col = plot_colors[i])
    }
    
    legend ("topleft", lty = 1, col = plot_colors, legend = key$iteration, bty = 'n',
                    cex = 0.5, title = 'iterations')

    placeholder <- dev.off()
}

call_profiler <- function (ras, iteration, rownum, file_prefix) {
    # function to call the profiler functions. Note this depends on externally defined
    # variables to control what this function does

    # Arguments:
    # ras = the raster to be profiled
    # iteration = the iteration label
    # file_prefix = the file prefix for the key files and profile files (label of sorts)
    # rownum = the row number for the profile
    
    # add a profile to the profiler files
    add_profile (ras, iteration, rownum, file_prefix)
}

finalize_profiler <- function (rownum, file_prefix) {
    # function to call finalization code in the profiler
    # Arguments:
    # rownum = the row number for the profile
    # file_prefix = the file_prefix for the key files and profile files
    
    # call the plotting code
    plot_profiles (rownum, file_prefix)
}    

    