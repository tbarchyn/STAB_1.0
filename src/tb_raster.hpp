// tb_raster - generic raster class for model simulations
// Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

/*
Copyright 2015-2016 Thomas E. Barchyn
Contact: Thomas E. Barchyn [tbarchyn@gmail.com]
v1.0: updated 23 February 2016

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Please familiarize yourself with the license of this tool, available
in the distribution with the filename: license.txt
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "tb_boundaries.hpp"            // generic boundaries class

class tb_raster {
    // generic raster class for spatial models
    public:
        double ** ras;                      // raster values
        int ydim;                           // nrows
        int xdim;                           // ncols
        double yll_corner;                  // the lower left corner y coord
        double xll_corner;                  // the lower left corner x coord
        double cellsize;                    // cellsize
        double nodata_value;                // nodata value
        tb_boundaries b;                    // boundaries object (note this requires manual initialization)
        
        tb_raster () {
            // constructor is simply a placeholder 
        }
                
        void init (int ydim_in, int xdim_in, double yll_corner_in, double xll_corner_in, double cellsize_in,
                   string boundaries_ns_in, string boundaries_ew_in) {
            /* method to initialize the raster object directly
            
            ydim_in = the y dimensions of the raster
            xdim_in = the x dimensions of the raster
            yll_corner_in = the y lower left corner of the raster
            xll_corner_in = the x lower left corner of the raster
            cellsize_in = the cellsize of the raster
            boundaries_ns_in = the boundaries type for the north-south edges, either 'periodic' or 'nonperiodic'
            boundaries_ew_in = the boundaries type for the east-west edges, either 'periodic' or 'nonperiodic'
            */
            ydim = ydim_in;
            xdim = xdim_in;
            yll_corner = yll_corner_in;
            xll_corner = xll_corner_in;
            cellsize = cellsize_in;
            nodata_value = -9999.0;
            b.init (ydim, xdim, boundaries_ns_in, boundaries_ew_in);
            allocate_mem ();
            setnull ();
        }
            
        void allocate_mem () {
            // method to allocate internal memory for the array
            try {
                ras = new double * [ydim];
                for (int y = 0; y < ydim; y++) {
                    ras[y] = new double [xdim];
                }
            } catch(...) {
                cout << "ERROR: cannot allocate sufficient memory!" << endl;
                exit (10);
            }
        }
        
        void write_ascii_raster (string outfilename) {
            /* method to write a conventional ascii raster surface file as a raster of doubles
            
            outfilename = filename of the output file
            */
            if (verbose) {
                cout << "Writing ascii raster file: " << outfilename << endl;
            }
                        
            ofstream ascfile;
            ascfile.open (outfilename.c_str());
                        
            // Write the header
            ascfile << "ncols " << xdim << "\n";
            ascfile << "nrows " << ydim << "\n";
            ascfile << "xllcorner " << xll_corner << "\n";
            ascfile << "yllcorner " << yll_corner << "\n";
            ascfile << "cellsize " << cellsize << "\n";
            ascfile << "NODATA_value " << nodata_value << "\n";
            
            /* write the rest of the file out . . 
            note: we are working down the rows as the ascii raster looks from above, but is referenced
            to the lower left corner. Thus, we start at row (ydim - 1) and work down to row 0.
            */
            
            double out_value = -9999.0;
            
            for (int y = (ydim - 1); y > -1; y--) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] == nodata_value) {
                        out_value = nodata_value;
                    } else {
                        out_value = ras[y][x];
                    }
                    
                    ascfile << out_value;       // write it out to the stream

                    if (x < (xdim - 1)) {
                        ascfile << " ";         // follow by pushing space delimiter
                    } else {
                        ascfile << "\n";        // or, follow by pushing endline char
                    }
                }
            }
            ascfile.close();
        }

        void read_ascii_raster (string infilename) {
            /* method to read in an ascii raster
            
            infilename = the name of the file to read in
            */
            
            int file_read_errorcode = 12;
            int fail_cntr;                      // counter to trigger file read failure
        
            if (verbose) {
                cout << "Reading ascii raster file: " << infilename << endl;
            }
            
            ifstream ifile;
            ifile.open ( infilename.c_str() );

            if (!ifile.is_open()) { 
                cout << "ERROR: cannot find input file: " << infilename << endl;
                exit (file_read_errorcode);
            }

            // Search 1: look for the number of columns
            fail_cntr = 0;
            string read1;
            do {
                ifile >> read1;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'ncols'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read1 != "ncols" && read1 != "NCOLS");
            ifile >> xdim;                  // next integer should be xdim or ncols

            // Search 2: look for the number of rows
            ifile.seekg (0);      // rewind to the beginning again
            fail_cntr = 0;
            string read2;
            do {
                ifile >> read2;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'nrows'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read2 != "nrows" && read2 != "NROWS");
            ifile >> ydim;                  // next integer should be ydim or nrows

            // Search 3: look for the xllcorner
            ifile.seekg (0);      // rewind to the beginning again
            fail_cntr = 0;
            string read3;
            do {
                ifile >> read3;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'xllcorner'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read3 != "xllcorner" && read3 != "XLLCORNER");
            ifile >> xll_corner;             // next double should be xll_corner

            // Search 4: look for the yllcorner
            ifile.seekg (0);      // rewind to the beginning again
            fail_cntr = 0;
            string read4;
            do { 
                ifile >> read4;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'yllcorner'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read4 != "yllcorner" && read4 != "YLLCORNER");
            ifile >> yll_corner;            // next double should be yll_corner

            // Search 5: look for the cellsize
            ifile.seekg (0);      // rewind to the beginning again
            fail_cntr = 0;
            string read5;
            do {
                ifile >> read5;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'yllcorner'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read5 != "cellsize" && read5 != "CELLSIZE");
            ifile >> cellsize;              // next double should be cellsize    

            // Search 6: look for the nodata flag value
            ifile.seekg (0);      // rewind to the beginning again
            fail_cntr = 0;
            string read6;
            do {
                ifile >> read6;
                fail_cntr++;
                if (fail_cntr == 100) {
                    cout << "FILE READ FAILURE!, need 'nodata flag'" << endl;
                    exit (file_read_errorcode);
                }
            }
            while (read6 != "nodata_value" && read6 != "NODATA_value" && read6 != "NODATA_VALUE");
            ifile >> nodata_value;          // next double should be nodata_value
            
            // next, we need to allocate memory for the raster
            allocate_mem ();
            
            // now, we can read in the contents of the raster into memory
            for (int y = (ydim - 1); y > -1; y--) {
                for (int x = 0; x < xdim; x++) {
                    ifile >> ras[y][x];
                }
            }
            
            ifile.close ();
            
            if (verbose) {
                cout << "success!" << endl;
            }   
        }
        
        tb_raster calc_aspect_horn (tb_raster oput) {
            /* method to calculate the aspect of the raster with the Horn (1981) method and
            return a tb_raster object that is a copy of the present raster.
            
            oput = a raster to update the cells
            
            This will produce incorrect values with NAs in the raster!!
            
            Notes: this was tested against arcgis spatial analysis aspect and seems to work fine. The
            only differences are this method returns 360.0, instead of 0.0. This could be changed
            at the very end of the method if desired - but I haven't done this as it hasn't been a
            problem just quite yet. Also the edges are represented differently in Arcgis, which is
            understandable as Arcgis does not see periodic boundaries like this function does.
            */
            
            if (verbose) {
                cout << "Calculating aspect with horn method" << endl;
            }
            
            oput.setnull ();
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    
                    double dz_dx;               // rise/run in x direction
                    double dz_dy;               // rise/run in y direction
                    double aspect;              // aspect in degrees
                    double aspect_return;       // returned aspect in azimuth degrees
                    
                    // calculate the dz_dx and dz_dy values
                    dz_dx = ((ras[b.n1[y]][b.e1[x]] + (2.0 * ras[y][b.e1[x]]) + ras[b.s1[y]][b.e1[x]] -
                                ras[b.n1[y]][b.w1[x]] - (2.0 * ras[y][b.w1[x]]) - ras[b.s1[y]][b.w1[x]]) /
                                (8.0 * cellsize));
                    dz_dy = ((ras[b.s1[y]][b.w1[x]] + (2.0 * ras[b.s1[y]][x]) + ras[b.s1[y]][b.e1[x]] -
                                ras[b.n1[y]][b.w1[x]] - (2.0 * ras[b.n1[y]][x]) - ras[b.n1[y]][b.e1[x]]) / 
                                (8.0 * cellsize));
                    
                    // assign -1 code to aspect if the location is flat
                    if (dz_dx == 0.0 && dz_dy == 0.0) {
                        aspect_return = -1.0;
                    } else {
                        // else calculate aspect with atan2
                        aspect = atan2 (dz_dy, (-1.0 * dz_dx)) * (180.0 / pi);
                        
                        // now convert this value to a compass azimuth
                        if (aspect < 90.0) {
                            aspect_return = 90.0 - aspect;
                        }
                        else {
                            aspect_return = 360.0 - aspect + 90.0;
                        }
                    }
                    oput.ras[y][x] = aspect_return;
                }
            }
            return (oput);
        }
        
        tb_raster calc_slope_horn (tb_raster oput) {
            /* method to calculate the aspect of the raster with the Horn (1981) method and
            return a tb_raster object that is a copy of the present raster.
            
            oput = a raster to update the cells
            
            This will produce incorrect values with NAs in the raster!!
            
            Notes: this function was tested against the slope calculation in arcgis and it seems to
            produce the same values. So I am reasonably confident we are on the right track. The only
            differences are around the edges, where arcgis doesn't see periodic boundaries (and this
            function does. This is acceptable.
            */
            
            if (verbose) {
                cout << "Calculating slope with horn method" << endl;
            }

            oput.setnull ();
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    
                    double dz_dx;               // rise/run in x direction
                    double dz_dy;               // rise/run in y direction
                    double rise_run;            // real rise/run
                    double slope;               // returned slope in degrees
                    
                    // calculate the dz_dx and dz_dy values
                    dz_dx = ((ras[b.n1[y]][b.e1[x]] + (2.0 * ras[y][b.e1[x]]) + ras[b.s1[y]][b.e1[x]] -
                                ras[b.n1[y]][b.w1[x]] - (2.0 * ras[y][b.w1[x]]) - ras[b.s1[y]][b.w1[x]]) /
                                (8.0 * cellsize));
                    dz_dy = ((ras[b.s1[y]][b.w1[x]] + (2.0 * ras[b.s1[y]][x]) + ras[b.s1[y]][b.e1[x]] -
                                ras[b.n1[y]][b.w1[x]] - (2.0 * ras[b.n1[y]][x]) - ras[b.n1[y]][b.e1[x]]) / 
                                (8.0 * cellsize));
                    
                    // calculate rise/run
                    rise_run = sqrt ((dz_dx * dz_dx) + (dz_dy * dz_dy));
                    
                    // if the rise/run is not zero, calculate real slope in degrees
                    if (rise_run != 0.0) {
                        slope = atan (rise_run) * (180.0 / pi);
                    } else {
                        slope = 0.0;
                    }
                    oput.ras[y][x] = slope;
                }
            }    
            return (oput);        
        }
        
        tb_raster calc_aspect_d8 (tb_raster oput) {
            /* method to calculate downhill aspect based on the d8 algorithm. This algorithm
            determines the steepest slope from the target cell to one of the eight adjacent
            cells.
            
            oput = a raster to update the cells
            
            Missing data cells are not addressed in this algorithm!
            */
            
            if (verbose) {
                cout << "Calculating aspect with d8 method" << endl;
            }
            
            oput.setnull ();

            double aspect;                  // aspect to be assigned
            double diff;                    // rise / run diff
            double angle_dist;              // angle distance
            
            angle_dist = sqrt ((cellsize * cellsize) + (cellsize * cellsize));
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    diff = -99999999999.9;
                    aspect = -1.0;
                    
                    // check N
                    if ((ras[y][x] - ras[b.n1[y]][x]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][x]) / cellsize;
                        aspect = 360.0;
                    }
                    // check NE
                    if ((ras[y][x] - ras[b.n1[y]][b.e1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][b.e1[x]]) / angle_dist;
                        aspect = 45.0;
                    }
                    // check E
                    if ((ras[y][x] - ras[y][b.e1[x]]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[y][b.e1[x]]) / cellsize;
                        aspect = 90.0;
                    }
                    // check SE
                    if ((ras[y][x] - ras[b.s1[y]][b.e1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][b.e1[x]]) / angle_dist;
                        aspect = 135.0;
                    }
                    // check S
                    if ((ras[y][x] - ras[b.s1[y]][x]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][x]) / cellsize;
                        aspect = 180.0;
                    }
                    // check SW
                    if ((ras[y][x] - ras[b.s1[y]][b.w1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][b.w1[x]]) / angle_dist;
                        aspect = 225.0;
                    }
                    // check W
                    if ((ras[y][x] - ras[y][b.w1[x]]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[y][b.w1[x]]) / cellsize;
                        aspect = 270.0;
                    }
                    // check NW
                    if ((ras[y][x] - ras[b.n1[y]][b.w1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][b.w1[x]]) / angle_dist;
                        aspect = 315.0;
                    }

                    oput.ras[y][x] = aspect;       // assign the direction with steepest diff
                }
            }    
            return (oput);
        }
        
        tb_raster calc_slope_d8 (tb_raster oput) {
            /* method to calculate downhill slope based on the d8 algorithm. This algorithm
            determines the steepest slope from the target cell to one of the eight adjacent
            cells.
            
            oput = a raster to update the cells
            
            Missing data cells are not addressed in this algorithm!
            */
            
            if (verbose) {
                cout << "Calculating slope with d8 method" << endl;
            }
            
            oput.setnull ();
            
            double diff;                    // rise / run diff
            double angle_dist;              // angle distance
            
            angle_dist = sqrt ((cellsize * cellsize) + (cellsize * cellsize));
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    diff = -99999999999.9;
                    
                    // check N
                    if ((ras[y][x] - ras[b.n1[y]][x]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][x]) / cellsize;
                    }
                    // check NE
                    if ((ras[y][x] - ras[b.n1[y]][b.e1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][b.e1[x]]) / angle_dist;
                    }
                    // check E
                    if ((ras[y][x] - ras[y][b.e1[x]]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[y][b.e1[x]]) / cellsize;
                    }
                    // check SE
                    if ((ras[y][x] - ras[b.s1[y]][b.e1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][b.e1[x]]) / angle_dist;
                    }
                    // check S
                    if ((ras[y][x] - ras[b.s1[y]][x]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][x]) / cellsize;
                    }
                    // check SW
                    if ((ras[y][x] - ras[b.s1[y]][b.w1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.s1[y]][b.w1[x]]) / angle_dist;
                    }
                    // check W
                    if ((ras[y][x] - ras[y][b.w1[x]]) / cellsize > diff) {
                        diff = (ras[y][x] - ras[y][b.w1[x]]) / cellsize;
                    }
                    // check NW
                    if ((ras[y][x] - ras[b.n1[y]][b.w1[x]]) / angle_dist > diff) {
                        diff = (ras[y][x] - ras[b.n1[y]][b.w1[x]]) / angle_dist;
                    }
                    
                    // check the difference is not 0.0 and assign
                    if (diff != 0.0) {
                        oput.ras[y][x] = atan (diff) * (180.0 / pi);
                    } else {
                        oput.ras[y][x] = 0.0;
                    }
                }
            }    
            return (oput);
        }
        
        void setnull () {
            /* method to set the raster values to nodata_val, the idea is to call this
            when a fresh raster is desired. The nodata value should obviously mess up
            calculations with the raster and highlight the error
            */
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    ras[y][x] = nodata_value;
                }
            }
        }
            
        void setvalue (double value) {
            /* method to set the raster to one value
            value = the value to set
            */
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    ras[y][x] = value;
                }
            }
        }
        
        void bumpify (double multiplier) {
            /* method to randomly add or subtract small amounts with a uniform dist to
            the raster values to make the raster surface bumpy, note that
            the mersenne twister functions must be part of the global scope
            multiplier = value to multiply against random draws from -0.5 to 0.5 to add
            */
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] != nodata_value) {
                        ras[y][x] = ras[y][x] + (multiplier * (genrand_real1() - 0.5));
                    }
                }
            }
        }    
        
        void copy_rastercells (tb_raster in_raster) {
            /* method to copy all the cells from another raster (with identical dimensions).
            Note that this only copies raster cells, not any spatial metadata or anything
            else!
            
            in_raster = another instance of a tb_raster
            */
            if (in_raster.xdim != xdim || in_raster.ydim != ydim) {
                cout << "ERROR: copy_values method requires conformant rasters" << endl;
                exit (10);
            }
            // copy the values
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (in_raster.ras[y][x] == in_raster.nodata_value) {
                        ras[y][x] = nodata_value;
                    } else {
                        ras[y][x] = in_raster.ras[y][x];
                    }
                }
            }
        }

        double sum () {
            /* method to return the sum of all values in the raster
            */
            
            double sum;
            sum = 0.0;
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] != nodata_value) {
                        sum = sum + ras[y][x];
                    }
                }
            }
            return (sum);
        }
        
        double min () {
            /* method to return the minimum of all values in the raster
            */
            
            double min; 
            min = 9.9999e306;
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] != nodata_value) {
                        if (ras[y][x] < min) {
                            min = ras[y][x];
                        }
                    }
                }
            }
            return (min);
        }
        
        double max () {
            /* method to return the maximum of all values in the raster
            */
            
            double max; 
            max = -9.9999e306;
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] != nodata_value) {
                        if (ras[y][x] > max) {
                            max = ras[y][x];
                        }
                    }
                }
            }
            return (max);
        }
        
        int num_NAs () {
            /* method to count the number of NAs in the raster
            */
            
            int num_NAs;
            num_NAs = 0;
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    if (ras[y][x] == nodata_value) {
                        num_NAs++;
                    }
                }
            }
            return (num_NAs);
        }    
        
        double mean () {
            /* method to return the mean value of a raster
            */
            
            double meanval;
            double number_NAs;
            double sumval;
            
            number_NAs = (double)num_NAs ();
            sumval = sum ();
            
            if (number_NAs == (ydim * xdim)) {
                meanval = nodata_value;
            } else {
                meanval = sumval / ((ydim * xdim) - number_NAs);
            }
            return (meanval);
        }
        
        void print_summary () {
            /* method to print a short summary to the console
            */
            cout << "sum = " << sum() << ", min = " << min() << ", max = " << max() << ", mean = " << mean () << endl;
            cout << "==============================================" << endl;
        }

        tb_raster rooks_filter (tb_raster oput) {
            /* method to filter the raster and populate the oput raster
            with the results. This filter is a mean of the 4 nearest cells
            and target cell or 'rooks case'. This eventually may be replaced with
            defined filter windows, but for now this is a simple convenience
            filter
            
            oput = a raster with identical size and shape to fill results
            */
            double neigh_sum;               // the neighborhood sum
            int neigh_count;                // the count of neighbors
            
            oput.setnull ();
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    neigh_sum = 0.0;
                    neigh_count = 0;
                    
                    // target cell
                    if (ras[y][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][x];
                        neigh_count++;
                    }

                    // rooks case cells
                    if (ras[b.n1[y]][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.n1[y]][x];
                        neigh_count++;
                    }
                    if (ras[b.s1[y]][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.s1[y]][x];
                        neigh_count++;
                    }
                    if (ras[y][b.e1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][b.e1[x]];
                        neigh_count++;
                    }
                    if (ras[y][b.w1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][b.w1[x]];
                        neigh_count++;
                    }
                    
                    if (neigh_count > 0) {
                        oput.ras[y][x] = neigh_sum / (double)neigh_count;
                    } else {
                        oput.ras[y][x] = oput.nodata_value;
                    }
                }
            }
            return (oput);
        }
            
        tb_raster queens_filter (tb_raster oput) {
            /* method to filter the raster and populate the oput raster
            with the results. This filter is a mean of the 8 nearest cells
            and target cell or 'queens case'. This eventually may be replaced with
            defined filter windows, but for now this is a simple convenience
            filter
            
            oput = a raster with identical size and shape to fill results
            */
            double neigh_sum;               // the neighborhood sum
            int neigh_count;                // the count of neighbors
            
            oput.setnull ();
            
            for (int y = 0; y < ydim; y++) {
                for (int x = 0; x < xdim; x++) {
                    neigh_sum = 0.0;
                    neigh_count = 0;
                    
                    // target cell
                    if (ras[y][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][x];
                        neigh_count++;
                    }
                    
                    // rooks case cells
                    if (ras[b.n1[y]][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.n1[y]][x];
                        neigh_count++;
                    }
                    if (ras[b.s1[y]][x] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.s1[y]][x];
                        neigh_count++;
                    }
                    if (ras[y][b.e1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][b.e1[x]];
                        neigh_count++;
                    }
                    if (ras[y][b.w1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[y][b.w1[x]];
                        neigh_count++;
                    }
                    
                    // oblique corner cells
                    if (ras[b.n1[y]][b.e1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.n1[y]][b.e1[x]];
                        neigh_count++;
                    }
                    if (ras[b.n1[y]][b.w1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.n1[y]][b.w1[x]];
                        neigh_count++;
                    }
                    if (ras[b.s1[y]][b.e1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.s1[y]][b.e1[x]];
                        neigh_count++;
                    }
                    if (ras[b.s1[y]][b.w1[x]] != nodata_value) {
                        neigh_sum = neigh_sum + ras[b.s1[y]][b.w1[x]];
                        neigh_count++;
                    }
                    
                    if (neigh_count > 0) {
                        oput.ras[y][x] = neigh_sum / (double)neigh_count;
                    } else {
                        oput.ras[y][x] = oput.nodata_value;
                    }
                }
            }
            return (oput);
        }
};




