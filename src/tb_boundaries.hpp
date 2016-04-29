// tb_boundaries: generic boundary class containing lookups for spatial models
// Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

/*
Copyright 2015 Thomas E. Barchyn
Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

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

class tb_boundaries {
    public:
        /* This class contains lookup arrays for boundaries. This is more or less an
        optimization that just allows looking up the the coordinates of the boundaries
        to the north and south of a given cell. The boundary lookups will of course
        change based on whether or not the boundaries are set to be periodic or not.
        */
        
        // Looking arrays: these are to evaluate the value of neighboring cells
        int * n1;               // 1 cell to the north
        int * s1;               // 1 cell to the south
        int * e1;               // 1 cell to the east
        int * w1;               // 1 cell to the west
        
        // Movement arrays: these are to actually change values and have a toxic_coord flag
        int * n1m;              // 1 cell to the north
        int * s1m;              // 1 cell to the south
        int * e1m;              // 1 cell to the east
        int * w1m;              // 1 cell to the west
        
        int * y_1cdw_n;         // 1 cell downflow in northerly
        int * x_1cdw_n;         // 1 cell downflow in northerly
        int * y_1cdw_s;         // 1 cell downflow in southerly
        int * x_1cdw_s;         // 1 cell downflow in southerly
        int * y_1cdw_e;         // 1 cell downflow in easterly
        int * x_1cdw_e;         // 1 cell downflow in easterly
        int * y_1cdw_w;         // 1 cell downflow in westerly
        int * x_1cdw_w;         // 1 cell downflow in westerly
        
        int toxic_coord;        // the toxic coordinate triggers removal of the slab
        int shadloops;          // number of loops for the shadow updater to run
        
        int ydim;               // y dimensions
        int xdim;               // x dimensions
        string boundaries_ns;   // boundaries north-south
        string boundaries_ew;   // boundaries east-west
        
        tb_boundaries () {
            // constructor is just a placeholder
        }
        
        void init (int ydim_in, int xdim_in, string boundaries_ns_in, string boundaries_ew_in) {
            /* initialize the boundary lookups
            
            Arguments:
            ydim_in = y dimensions (number of rows)
            xdim_in = x dimensions (number of cols)
            boundaries_ns_in = input boundaries north-south
            boundaries_ew_in = input boundaries east-west
            */
            
            if (verbose) {
                cout << "Initializing boundary lookups . . ";
            }
            
            ydim = ydim_in;                     // assign class values
            xdim = xdim_in;
            boundaries_ns = boundaries_ns_in;           
            boundaries_ew = boundaries_ew_in;
            
            // allocate some memory first
            try {
                n1 = new int [ydim];
                s1 = new int [ydim];
                e1 = new int [xdim];
                w1 = new int [xdim];
                n1m = new int [ydim];
                s1m = new int [ydim];
                e1m = new int [xdim];
                w1m = new int [xdim];
                y_1cdw_n = new int [ydim];
                x_1cdw_n = new int [xdim];
                y_1cdw_s = new int [ydim];
                x_1cdw_s = new int [xdim];
                y_1cdw_e = new int [ydim];
                x_1cdw_e = new int [xdim];
                y_1cdw_w = new int [ydim];
                x_1cdw_w = new int [xdim];
			} catch(...) {
				cout << "BOUNDARIES ERROR: cannot allocate sufficient memory!" << endl;
				exit (10);
			}

            toxic_coord = -1;          // set the toxic coordinate
            
            // determine what types of boundaries we need and call setup methods
            if (boundaries_ns == "periodic") {
                setup_periodic_ns();
            }
            if (boundaries_ew == "periodic") {
                setup_periodic_ew();
            }
            if (boundaries_ns == "nonperiodic") {
                setup_nonperiodic_ns();
            }
            if (boundaries_ew == "nonperiodic") {
                setup_nonperiodic_ew();
            }
            
            setup_shadloops ();             // setup number of shadloops
            setup_downflow_arrays ();       // setup the downflow arrays

            if (verbose) {
                cout << "complete" << endl;
            }
        }

        void setup_shadloops () {
            /* method to set up the number of shadloops. Relavant for Werner style CA models.
            The number of shadloops varies from 1 to 2 based on whether the boundaries
            are periodic along the axis that the wind direction is blowing. The shadow
            updater needs to travel 2 loops if the boundaries are periodic because the
            shadows from the downflow edge need to loop around upwind.
            */
            
            // start by setting default to 2 (to be safe), then set to 1 in special conditions
            shadloops = 2;
            
            if (boundaries_ns == "nonperiodic" && boundaries_ew == "nonperiodic") {
                shadloops = 1;
            }
            /*
            Update for MGD revision 1.7: since we are dynamically assigning wind direction every
            poll, we can never be fully sure that the shadloops can be backed off to 1. So
            we only set it to 1 in the special case where both the ns and ew boundaries are
            set to nonperiodic. If simulating nonperiodic simulations ew it just makes sense to
            set both boundaries to nonperiodic to take advantage of the speed up that occurs
            with just one shadloop.
            */
        }
        
        void setup_downflow_arrays () {
            /* method to set the downflow coordinates based on the wind direction.
            */
            
            for (int y = 0; y < ydim; y++) {
                y_1cdw_n[y] = s1m[y];
            }
            
            for (int x = 0; x < xdim; x++) {
                x_1cdw_n[x] = x;
            }
            
            for (int y = 0; y < ydim; y++) {
                y_1cdw_s[y] = n1m[y];
            }
            
            for (int x = 0; x < xdim; x++) {
                x_1cdw_s[x] = x;
            }
            
            for (int y = 0; y < ydim; y++) {
                y_1cdw_e[y] = y;
            }
            
            for (int x = 0; x < xdim; x++) {
                x_1cdw_e[x] = w1m[x];
            }
            
            for (int y = 0; y < ydim; y++) {
                y_1cdw_w[y] = y;
            }
            
            for (int x = 0; x < xdim; x++) {
                x_1cdw_w[x] = e1m[x];
            }
            
        }
        
        void setup_periodic_ns () {
            // setup lookup arrays for periodic boundaries
            for (int y = 0; y < ydim; y++) {
                n1[y] = y + 1;
                s1[y] = y - 1;
                n1m[y] = y + 1;
                s1m[y] = y - 1;
            }
            n1[ydim - 1] = 0;
            s1[0] = (ydim - 1);
            n1m[ydim - 1] = 0;            // movement is same as looking (move slabs periodically)
            s1m[0] = (ydim - 1);
        }
        
        void setup_periodic_ew () {
            /* setup lookup arrays for periodic boundaries
            */
            
            for (int x = 0; x < xdim; x++) {
                e1[x] = x + 1;
                w1[x] = x - 1;
                e1m[x] = x + 1;
                w1m[x] = x - 1;
            }
            e1[xdim - 1] = 0;
            w1[0] = (xdim - 1);
            e1m[xdim - 1] = 0;
            w1m[0] = (xdim - 1);
        }

        void setup_nonperiodic_ns () {
            /* setup lookup arrays for nonperiodic boundaries, which have mirrored edges for looking
            and toxic edges for movement. The toxic_coord is set when movement is off the raster
            and the boundaries are nonperiodic.
            */
            
            for (int y = 0; y < ydim; y++) {
                n1[y] = y + 1;
                s1[y] = y - 1;
                n1m[y] = y + 1;
                s1m[y] = y - 1;
            }
            n1[ydim - 1] = (ydim - 1);
            s1[0] = 0;
            n1m[ydim - 1] = toxic_coord;
            s1m[0] = toxic_coord;
        }
        
        void setup_nonperiodic_ew () {
            /* setup lookup arrays for nonperiodic boundaries, which have mirrored edges for looking
            and toxic edges for movement. The toxic_coord is set when movement is off the raster
            and the boundaries are nonperiodic.
            */
            
            for (int x = 0; x < xdim; x++) {
                e1[x] = x + 1;
                w1[x] = x - 1;
                e1m[x] = x + 1;
                w1m[x] = x - 1;
            }
            e1[xdim - 1] = (xdim - 1);
            w1[0] = 0;
            e1m[xdim - 1] = toxic_coord;
            w1m[0] = toxic_coord;
        }
};



