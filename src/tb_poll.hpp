// tb_poll - generic poll class for model simulations
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

class tb_poll {
    /* This class creates poll sequences for sampling without replacement in iterations
    */
    
    public:
        int * ys;                       // list of ys
        int * xs;                       // list of xs
        int * vcoords;                  // vector coordinates
        int ydim;                       // ydim
        int xdim;                       // xdim
        int len;                        // length of vector coordinates
  
        tb_poll () {
            // constructor is just placeholder: must call init
        }
        
        void init (int ydim_in, int xdim_in) {
            /* initialize the polling object
            ydim_in: the assigned y dimensions
            xdim_in: the assigned x dimensions
            */
            
            ydim = ydim_in;
            xdim = xdim_in;
            len = ydim * xdim;
            
            // allocate memory
            try {
                ys = new int [len];
                xs = new int [len];
                vcoords = new int [len];
            } catch (...) {
				cout << "ERROR: cannot allocate sufficient memory!" << endl;
				exit (10);
			}
            
            // set up the vcoords vector as an ordered sequence
            for (int i = 0; i < len; i++) {
                vcoords[i] = i;
            }
            
            calc_new_sequence ();
        }
        
        void calc_new_sequence () {
            /* method to set out a new sequence of random samples, without replacement
            */
            int r;              // the coordinate to shuffle
            int rval;           // the value to shuffle
            
            for (int i = len - 1; i > -1; i--) {
                r = genrand_int32() % (i + 1);              // draw a random integer
                rval = vcoords[r];                          // get the value
                vcoords[r] = vcoords[i];                    // shuffle  
                vcoords[i] = rval;                          // shuffle
            }

            // apply the shuffled vcoords to the lookup arrays
            for (int i = 0; i < len; i++) {
                xs[i] = vcoords[i] / ydim;
                ys[i] = vcoords[i] % ydim;
            }
        }
};
