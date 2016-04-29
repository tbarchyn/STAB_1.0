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

# Color ramps for R imager - update this as desired!

set_color_ramp <- function (overlay_name, alpha_transparency) {
    # function to set the color ramp based on the overlay name for individual overlays
    # Arguments:
    # overlay_name = the name of the overlay (key for the different color ramps)
    # alpha_transparency = the alpha transparency
    
    if (overlay_name == 'pres') {
        colvector <- colorRampPalette (c('orange', 'forestgreen'))(1000)
    } else if (overlay_name == 'ice_cavity') {
        colvector <- colorRampPalette (c('red', 'blue'))(1000)
    } else if (overlay_name == 'ice_load') {
        colvector <- colorRampPalette (c('yellow', 'orange', 'red'))(1000)
    } else if (overlay_name == 'surf') {
        colvector <- colorRampPalette (c('blue', 'yellow', 'red'))(1000)
    } else if (overlay_name == 'sed_depth') {
        colvector <- colorRampPalette (c('blue', 'green', 'yellow'))(1000)
    } else {
        # default color vector for un-assigned variables
        colvector <- colorRampPalette (c('white', 'black'))(1000)
    }
    
    colvector <- alpha (colvector, alpha = alpha_transparency)
    return (colvector)
}




