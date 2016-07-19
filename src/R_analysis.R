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

# R analysis code

calc_streamlining <- function (surf_ras, ras_slope, ras_aspect) {
    # function to evaluate the streamlining of the topography
    # Arguments:
    # surf_ras = the surf raster object
    # ras_slope = the raster slope (in radians)
    # ras_aspect = the raster aspect (in radians)
    
    # convert to degrees
    slope <- ras_slope * (180 / pi)
    aspect <- ras_aspect * (180 / pi)
    
    # calculate the flow parallel fraction of slopes, and the flow perpendicular fraction
    flowparallel_frac <- length (aspect[slope > 0.0 & ((aspect > 135.0 & aspect < 225.0) | aspect < 45.0 | aspect > 315.0)])
    flowperp_frac <- length (aspect[slope > 0.0 & ((aspect > 45.0 & aspect < 135.0) | (aspect > 215.0 & aspect < 315.0))])
    if (flowperp_frac != 0.0) {
        streamlining <- flowparallel_frac / (flowparallel_frac + flowperp_frac)
    } else {
        streamlining <- NA
    }
    return (streamlining)
}    

calc_streamlining2 <- function (surf_ras, ras_slope, ras_aspect) {
    # function to calculate streamlining 2, which is a new algorithm for 
    # evaluating the streamlining of the surface. Instead of comparing aspect
    # proportions, this algorithm calculates vector sums of the aspects.
   
    # Arguments:
    # surf_ras = the raster object
    # ras_slope = the raster slope (in radians)
    # ras_aspect = the raster aspect (in radians)

    # calculate flow parallel and flow perpendicular rasters
    ras_aspect [ras_slope == 0.0] <- NA
    flow_perp <- cos (ras_aspect)
    flow_parallel <- sin (ras_aspect)
    
    # calculate sums of absolute values
    flow_perp_sum <- sum (abs (as.matrix (flow_perp)), na.rm = T)
    flow_parallel_sum <- sum (abs (as.matrix (flow_parallel)), na.rm = T)
    
    # calculate streamlining 2 to range from 0-1 to be comparable to streamlining 1
    if (flow_perp_sum != 0.0) {
        streamlining2 <- flow_perp_sum / (flow_perp_sum + flow_parallel_sum)
    } else {
        streamlining2 <- NA
    }
    
    return (streamlining2)
}

calc_bsmt_exposure <- function (surf_ras, bsmt_ras) {
    # function to calculate the exposure of basement as a proportion of the raster
    # space. This should return NAs if there are any NAs in the raster (which there
    # shouldn't be).
    
    # Arguments:
    # surf_ras = the surf raster object to be analysed
    # bsmt_ras = the basement raster object to be analysed
    
    bsmt_bool <- calc_feat_mask (surf_ras, bsmt_ras, 0.0)
    exposed_proportion <- mean (as.matrix (bsmt_bool))
    return (exposed_proportion)
}

calc_feat_mask <- function (surf_ras, cut_ras, cut_elev) {
    # function to create a raster with 1's where there is a defined thickness
    # of sediment above a cut elevation raster, and 0.0 where there is no sediment.
    
    # Arguments:
    # surf_ras = the surf raster object to be analysed
    # cut_ras = the cut raster to cut offsets from (bsmt often)
    # cut_elev = the cut elevation
    
    # assign a rounding error guard if calculating basement exposure
    if (cut_elev == 0.0) {
        rounding_error_guard <- 0.00000000001
    } else {
        rounding_error_guard <- 0.0
    }
    
    feat_mask <- surf_ras * 0.0
    feat_mask [surf_ras >= cut_ras + cut_elev + rounding_error_guard] <- 1.0
    return (feat_mask)
}

calc_feat_dims <- function (surf_ras, feat_mask, cellsize, metric) {
    # function to calculate dimensions of positive feature detection on a row by
    # row basis down the raster. This returns a vector of lengths, which may
    # exceed the number of rows in the raster. Note this does not enumerate unique
    # objects, so there is some math required to relate classical ovoids to these
    # metrics. Note that this only works with periodic boundaries at present - due
    # to ambiguities related to what happens with objects detected that go off the
    # raster space!
    
    # Arguments:
    # surf_ras = the surf raster object to be analysed  
    # feat_mask = the feature mask to be analysed
    # cellsize = the cellsize required to calculate distances properly
    # metric = 'len' or 'width'
    
    # convert the rasters to matrices - subsetting raster is slow!
    surf_ras <- as.matrix (surf_ras)
    feat_mask <- as.matrix (feat_mask)
    
    lens <- NULL                        # set the lens vector
    assigned <- surf_ras * 0.0          # set a log for the length loop
    
    # assign a max_dimension value based on whether we are evaluating widths or lengths
    # here, we can also make sure we have a defined metric, previous splits based on metric
    # will just go ahead without this consideration, so don't remove this!
    if (metric == 'len') {
        max_dimension <- ncol (surf_ras)
    } else if (metric == 'width') {
        max_dimension <- nrow (surf_ras)
    } else {            
        print ('ERROR: undefined metric in calc_feat_dims function in R_analysis.R!')
        sys.exit ()
    }

    # loop across each row or column
    i <- 1
    j <- 1
    finished <- FALSE
    
    while (!finished) {
    
        # set the starting coordinate
        if (metric == 'len') {
            j <- 1                  # length start with j
        } else {
            i <- 1                  # width start with i
        }
        
        # loop down each row or column
        skip <- TRUE                    # skip and count on wrap-around
        feat_len <- 0.0                 # set regular feature length
        feat_on <- FALSE                # set a flag for the feature on
        skip_counter <- 0               # skip counter
        infinite_feature <- FALSE       # infinite feature flag
        
        while (assigned[i, j] == 0.0) {
            # if we begin and find a feature, we skip and cound the length upon
            # the wrap-around. The skip flag starts true, but as soon as it
            # is set to false, it remains false for the rest of the loop

            if (skip & feat_mask[i, j] == 1.0) {
                skip <- TRUE                         # maintain true state if feat present
                skip_counter <- skip_counter + 1     # count those skips to avoid infinite loop
                if (skip_counter == max_dimension) {
                    infinite_feature <- TRUE
                    break
                }
            } else {
                skip <- FALSE                        # else, enough skipping, we are on
            }
            
            # if the skip flag is false, we count the lengths normally
            if (!skip) {
                # if feat_mask is true, we have a feature, start or continue feature
                if (feat_mask[i, j] == 1.0) {
                    # start or continue new feature
                    feat_on <- TRUE
                    feat_len <- feat_len + 1.0
                } else {
                    # check to see if this is the end of a feature (feat_on is true)
                    if (feat_on) {
                        # append the present feature length to the lens vector
                        lens <- c(lens, feat_len)
                        feat_on <- FALSE            # reset
                        feat_len <- 0.0             # reset    
                    }
                }
                assigned[i, j] <- 1.0        # note that we have taken stock of this cell
            }
            
            # re-assign the coordinate
            if (metric == 'len') {
                j <- j + 1
                if (j == ncol(surf_ras) + 1) {
                    j <- 1                   # enpres periodic boundaries
                }
            } else {
                i <- i + 1
                if (i == nrow(surf_ras) + 1) {
                    i <- 1                   # enpres periodic boundaries
                }
            }    
        }

        # check to make see if we are counting a feature, and finish it up
        if (feat_on) {
            lens <- c(lens, feat_len)
        }
        
        # check for an infinite feature (fixed bug here that assigned ncols for widths)
        if (infinite_feature) {
            lens <- c(lens, max_dimension)
        }
        
        # now, re-assign the row or column coordinate
        if (metric == 'len') {
            i <- i + 1
            if (i == nrow(surf_ras) + 1) {
                finished <- TRUE
            }
        } else {
            j <- j + 1
            if (j == ncol(surf_ras) + 1) {
                finished <- TRUE
            }
        }
    }
    lens <- lens * cellsize             # scale properly to cellsize
    return (lens)
}

calc_Rmetrics <- function (surf_ras, bsmt_ras, iteration, cellsize) {
    # wrapper function to calc metrics for a given raster input and add
    # it to the metrics file.
    
    # Arguments:
    # surf_ras = the surf raster object
    # bsmt_ras = the basement raster object
    # iteration = the iteration (to assign properly in the file)
    # cellsize = the assigned cellsize
    
    # set the metrics file name and variables we are to fill to make a prototype dataframe
    metrics_file <- 'stab_Rmetrics.csv'
    metrics_vars <- c('iteration', 'streamlining', 'streamlining2', 'bsmt_exposure',
                      'feat_lens_mean', 'feat_lens_len', 'feat_lens_min', 'feat_lens_max',
                      'feat_widths_mean', 'feat_widths_len', 'feat_widths_min', 'feat_widths_max',
                      'feat_coverage', 'surf_q25', 'surf_q50', 'surf_q75')
    metrics_proto <- matrix (NA, nrow = 1, ncol = length (metrics_vars))
    metrics_proto <- data.frame (metrics_proto)
    names (metrics_proto) <- metrics_vars
                      
    # check to see if we have files already, otherwise read existing files
    if (!file.exists (metrics_file)) {
        metrics <- metrics_proto
        metrics <- metrics [-1,]
        write.table (metrics, metrics_file, row.names = F, col.names = T, sep = ',')
    } else {
        metrics <- read.csv (metrics_file)
        metrics$iteration <- as.character (metrics$iteration)
    }
    
    # calculate feature cut (presently mean sediment thickness)
    sed_thickness <- surf_ras[[1]] - bsmt_ras
    mean_sedthickness <- mean (as.matrix(sed_thickness))
    feat_mask <- calc_feat_mask (surf_ras[[1]], bsmt_ras, mean_sedthickness)
    
    # calculate feature lengths
    lens <- calc_feat_dims (surf_ras[[1]], feat_mask, cellsize, metric = 'len')
    widths <- calc_feat_dims (surf_ras[[1]], feat_mask, cellsize, metric = 'width')
    
    # calculate metrics and add them to the dataframe
    metrics_proto$iteration[1] <- iteration
    metrics_proto$streamlining[1] <- calc_streamlining (surf_ras[[1]], surf_ras[[2]], surf_ras[[3]])
    metrics_proto$streamlining2[1] <- calc_streamlining2 (surf_ras[[1]], surf_ras[[2]], surf_ras[[3]])
    metrics_proto$bsmt_exposure[1] <- calc_bsmt_exposure (surf_ras[[1]], bsmt_ras)
    metrics_proto$feat_lens_mean[1] <- mean (lens)
    metrics_proto$feat_lens_len[1] <- length (lens)
    metrics_proto$feat_lens_min[1] <- min (lens)
    metrics_proto$feat_lens_max[1] <- max (lens)
    metrics_proto$feat_widths_mean[1] <- mean (widths)
    metrics_proto$feat_widths_len[1] <- length (widths)
    metrics_proto$feat_widths_min[1] <- min (widths)
    metrics_proto$feat_widths_max[1] <- max (widths)
    metrics_proto$feat_coverage[1] <- mean (as.matrix (feat_mask))
    metrics_proto$surf_q25[1] <- quantile (as.matrix (surf_ras[[1]]), probs = 0.25)
    metrics_proto$surf_q50[1] <- quantile (as.matrix (surf_ras[[1]]), probs = 0.50)
    metrics_proto$surf_q75[1] <- quantile (as.matrix (surf_ras[[1]]), probs = 0.75)
    
    # now append the metrics_proto dataframe, presently populated, to the existing data frame
    metrics <- rbind (metrics, metrics_proto)

    # write the table
    write.table (metrics, metrics_file, row.names = F, col.names = T, sep = ',')
}

plot_all_metrics <- function () {
    # plot all the metrics in the coallated file in a loop, some of these plots
    # are pointless (e.g., iteration vs. iteration), but nevertheless, just ignore
    # them for now.
    
    coallated_filename <- 'stab_coallated_metrics.csv'
    mg <- read.csv (coallated_filename)
    mg <- mg [order (mg$t), ]
    
    # set any Infs or -Infs to NA
    mg[mg == Inf] <- NA
    mg[mg == -Inf] <- NA
    
    # loop through all the metrics
    for (i in names(mg)) {
        # set any special instructions
        if (i == 'streamlining' | i == 'streamlining2') {
            ylim_set <- c(0, 1)
        } else {
            ylim_set <- c(min (mg[, i], na.rm = T), max(mg[, i], na.rm = T))
        }
        
        # set the plotfile
        plotfile <- paste ('mg_', i, '.jpeg', sep = '')
        jpeg (filename = plotfile, res = jpeg_res_image, width = jpeg_width_image, height = jpeg_height_image, units = 'in')
        plot (mg$t, mg[, i], type = 'l', col = 'red', ylim = ylim_set,
                xlab = 'iteration', ylab = i,  main = i)
        placeholder <- dev.off()
    }
}

coallate_metricsfiles <- function () {
    # function to coallate the kinematics file and the metrics file, after everything is calculated
    # the idea is to make one file for each simulation. Legacy code can still call the existing
    # files, but I would like to move forward with one coallate file. Presently only implemented
    # with only one metrics file, but there is possibility of additional joins.
        
    kin_file <- 'stab_kinematics.csv'
    metrics_file_1 <- 'stab_Rmetrics.csv'
    coallated_filename <- 'stab_coallated_metrics.csv'
    
    # read the files in, and sort them (just to be safe here)
    kin <- read.csv (kin_file)
    metrics_1 <- read.csv (metrics_file_1)
    kin <- kin [order (kin$t), ]
    metrics_1 <- metrics_1 [order (metrics_1$iteration), ]
    
    # perform the join(s)
    mg <- merge (kin, metrics_1, by.x = 't', by.y = 'iteration', all = T, sort = T)
    
    # write the file
    write.table (mg, coallated_filename, col.names = T, row.names = F, sep = ',')
}

call_analyser <- function (surf_ras, bsmt_ras, iteration, cellsize) {
    # main head call for analysis
    
    # Arguments:
    # surf_ras = the surf raster object
    # bsmt_ras = the basement raster object
    # iteration = the iteration
    # cellsize = the assigned raster cellsize
    
    # calculate surf metrics and append them to the file
    calc_Rmetrics (surf_ras, bsmt_ras, iteration, cellsize)
}

finalize_analyser <- function () {
    # finalization call for analysis code
    
    # first, coallate the metrics and kinematics files. The kinematics files come directly
    # out of the stab model and have summaries from internal rasters that are not generally
    # output. Some of this might be useful, some of it might not.
    coallate_metricsfiles ()
    
    # now, we can plot metrics
    plot_all_metrics ()

    print ('complete')
}
