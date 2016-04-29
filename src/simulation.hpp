/*
STAB: subglacial till advection and bedforms
Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

Copyright 2014-2016 Thomas E. Barchyn
Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

This project was developed with input from Thomas P.F. Dowling,
Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate
citation of the relavent publications.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Please familiarize yourself with the license of this tool, available
in the distribution with the filename: /docs/license.txt
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

class simulation {
    public:
        /*
        The simulation class stores all the parameters necessary for the simulation by 
        reading a simfile, which is given as an argument to the main program. This is
        essentially just read verbatim from a simfile. This is more or less just an echo
        class storing the contents of the simfile in memory.
        */
        
        // Simulation parameters
        int ydim;                            // y dimensions
        int xdim;                            // x dimensions
        double cellsize;                     // cellsize of the raster
        double yll_corner;                   // the yll_corner
        double xll_corner;                   // the xll_corner
        int max_iterations;                  // max iterations for this model run
        double len_timestep;                 // length of timestep in years
        double global_basal_pres;            // global basal pres
        double total_basal_pres;             // total basal pres across model space
        double viscosity;                    // ice viscosity
        double Q_advection_global;           // sediment advection coefficient
        double Q_advection_stochasticity;    // advection stochasticity
        double Q_squish_coef;                // squish coefficient   
        double ice_advection;                // ice advection rate
        
        // entrainment properties
        double entrainment_cavity;           // cavity entrainment rate
        double entrainment_zero;             // entrainment rate at 0.0 pres
        double entrainment_slp_1;            // slope from 0.0 pres to vtx_2 pres
        double entrainment_vtx_2;            // vtx_2 pres
        double entrainment_slp_2;            // slope from vtx_2 pres to positive infinity
        
        // basement erosion properties
        double abrasion_from_N_slope;        // abrasion from basal_pres, the slope to increase with N
        double abrasion_from_N_zero;         // the abrasion rate at 0 Pa N
        double abrasion_from_iceload;        // the abrasion rate as a linear function of iceload
        double global_bsmt_erodibility;      // the global basement erodibility
        double iceload_surf_return_fraction; // the proportion of eroded sediment to place into the iceload,
                                             // the remainder of sediment is placed on the surface
        
        // bleed properties
        double iceload_bleed;                // the rate of en/distrainment from the upper glacier
        double surf_bleed;                   // the bleed directly from the surface

        // R properties
        string Rscript_path;                 // path to the Rscript installation on this computer
        string progress_utility_name;        // path to the progress utility script to create progress plots
        bool on_the_fly_progress_updates;    // boolean to update the progress on the fly, or just at the end
            
        // model space properties
        string boundaries_ns;                // boundaries type north-south
        string boundaries_ew;                // boundaries type east-west
        
        // interim file outputs
        int interim_file_output_interval;    // interval for interim file outputs
        string file_output_prefix;           // prefix for all output files
        
        // initialization parameters
        string init_type;                    // type of model space initialization
        double flat_init_basement_elev;      // elevation of basement
        double flat_init_sedfill_elev;       // elevation of sediment
        double init_iceload;                 // initial iceload
        double init_erodibility;             // initial erodibility of the basement
        string existing_surf_file;           // existing surface file
        string existing_bsmt_file;           // existing basement file
        string existing_erodibility_file;    // existing erodibility file
        
        ifstream cfile;                      // simfile file object
        
        simulation () {
            // constructor is a placeholder, must call init
        }

        void init (string simfilename) {
            /* intialization reads the simfile and populates the internal parameters
            simfilename = name of the simulation parameter file
            */
            
            cfile.open(simfilename.c_str());
            
            if (!cfile.is_open()) { 
				cout << "ERROR: cannot find simfile!" << endl;
				exit (10);
			}

            // populate the internal parameters
            string returnstring;                    // string that gets returned for conversion to numbers
            
            returnstring = find_header_element ("ydim", false);
            ydim = atoi (returnstring.c_str());
            
            returnstring = find_header_element ("xdim", false);
            xdim = atoi (returnstring.c_str());
            
            returnstring = find_header_element ("cellsize", false);
            cellsize = atof (returnstring.c_str());
            
            returnstring = find_header_element ("yll_corner", false);
            yll_corner = atof (returnstring.c_str());
            
            returnstring = find_header_element ("xll_corner", false);
            xll_corner = atof (returnstring.c_str());

            returnstring = find_header_element ("max_iterations", false);
            max_iterations = atoi (returnstring.c_str());
            
            returnstring = find_header_element ("len_timestep", false);
            len_timestep = atof (returnstring.c_str());
    
            returnstring = find_header_element ("global_basal_pres", false);
            global_basal_pres = atof (returnstring.c_str());

            returnstring = find_header_element ("viscosity", false);
            viscosity = atof (returnstring.c_str());

            returnstring = find_header_element ("Q_advection_global", false);
            Q_advection_global = atof (returnstring.c_str());
            
            returnstring = find_header_element ("Q_advection_stochasticity", false);
            Q_advection_stochasticity = atof (returnstring.c_str());

            returnstring = find_header_element ("Q_squish_coef", false);
            Q_squish_coef = atof (returnstring.c_str());

            returnstring = find_header_element ("ice_advection", false);
            ice_advection = atof (returnstring.c_str());

            // entrainment variables
            returnstring = find_header_element ("entrainment_cavity", false);
            entrainment_cavity = atof (returnstring.c_str());
            
            returnstring = find_header_element ("entrainment_zero", false);
            entrainment_zero = atof (returnstring.c_str());
            
            returnstring = find_header_element ("entrainment_slp_1", false);
            entrainment_slp_1 = atof (returnstring.c_str());
            
            returnstring = find_header_element ("entrainment_vtx_2", false);
            entrainment_vtx_2 = atof (returnstring.c_str());
            
            returnstring = find_header_element ("entrainment_slp_2", false);
            entrainment_slp_2 = atof (returnstring.c_str());
            
            // basement erosion properties
            returnstring = find_header_element ("abrasion_from_N_slope", false);
            abrasion_from_N_slope = atof (returnstring.c_str());
            
            returnstring = find_header_element ("abrasion_from_N_zero", false);
            abrasion_from_N_zero = atof (returnstring.c_str());
            
            returnstring = find_header_element ("abrasion_from_iceload", false);
            abrasion_from_iceload = atof (returnstring.c_str());
            
            returnstring = find_header_element ("global_bsmt_erodibility", false);
            global_bsmt_erodibility = atof (returnstring.c_str());
            
            returnstring = find_header_element ("iceload_surf_return_fraction", false);
            iceload_surf_return_fraction = atof (returnstring.c_str());
            
            // sediment bleed properties
            returnstring = find_header_element ("iceload_bleed", false);
            iceload_bleed = atof (returnstring.c_str());
            
            returnstring = find_header_element ("surf_bleed", false);
            surf_bleed = atof (returnstring.c_str());
            
            // R script parameters
            Rscript_path = find_header_element ("Rscript_path", true);
            progress_utility_name = find_header_element ("progress_utility_name", true);
            returnstring = find_header_element ("on_the_fly_progress_updates", false);
            if (returnstring == "yes") {
                on_the_fly_progress_updates = true;
            } else {
                on_the_fly_progress_updates = false;
            }
            
            // boundaries
            boundaries_ns = find_header_element ("boundaries_ns", false);
            boundaries_ew = find_header_element ("boundaries_ew", false);

            returnstring = find_header_element ("interim_file_output_interval", false);
            interim_file_output_interval = atoi (returnstring.c_str());
            
            file_output_prefix = find_header_element ("file_output_prefix", true);
            
            init_type = find_header_element ("init_type", false);
            
            returnstring = find_header_element ("flat_init_basement_elev", false);
            flat_init_basement_elev = atof (returnstring.c_str());

            returnstring = find_header_element ("flat_init_sedfill_elev", false);
            flat_init_sedfill_elev = atof (returnstring.c_str());
            
            returnstring = find_header_element ("init_iceload", false);
            init_iceload = atof (returnstring.c_str());
            
            returnstring = find_header_element ("init_erodibility", false);
            init_erodibility = atof (returnstring.c_str());
            
            existing_surf_file = find_header_element ("existing_surf_file", true);
            existing_bsmt_file = find_header_element ("existing_bsmt_file", true);
            existing_erodibility_file = find_header_element ("existing_erodibility_file", true);
            
            cfile.close();
        }
            
        string find_header_element (string element, bool is_path) {
            /* method to find a specific element from the simfile and return it
            
            Arguments:
            element: the element name we are searching for (tagname)
            is_path: boolean to trigger the function to use the code to read paths properly
            
            Notes: this method reads the next element from the filestream as the element we
            are searching for after identifying the tagname so the elements need to be separated
            by a whitespace. Returns a string representation of the value, which needs to be
            converted above.
            */
            
            string value;               // value to return
            int failure_counter;        // failure counter (in case no match found)
            int max_failures = 2000;    // maximum number of failures allowed
            string text_read;           // the string read in from the stream
            
            cfile.seekg(0);             // rewind to beginning
            failure_counter = 0;
            do {
                cfile >> text_read;
                failure_counter++;
                if (failure_counter == max_failures) {
                    cout << "ERROR: simfile file read error, could not find " << element << endl;
                    exit(2);
                }
            }
            while (text_read != element);
            
            /* Notes: here we need to use different read methods for Linux and Windows
            The read method for Windows allows spaces in the filenames (which is necessary
            for the Rscript path. Although it shouldn't be a problem for other elements
            as the user shouldn't have spaces in the pathnames (I hope they read the user
            guide that warns them not to use spaces in pathnames!). In Linux, you can just
            call Rscript with the name 'Rscript' and you don't need the full path.
            
            Alternately, and probably recommended is to add Rscript to your system path
            variable on windows.
            
            I have no idea how any of this works on Mac computers, sorry - so please email me
            if you are compiling on Mac computers.
            */
            
            #ifdef __MINGW32__
            if (is_path) {
                // get the rest of the line if we are searching for a path
                getline (cfile, value);
                
                // keep trimming spaces off the front of the string
                while (value.substr(0, 1) == " ") {
                    value = value.substr(1, value.npos);
                    if (value.npos == 0) {
                        break;                  // break if there is no string present
                    }
                }
            } else {
                cfile >> value;                 // assign next read to the value and return it
            }
            #endif
            
            #ifdef __linux__
            cfile >> value;
            #endif
            
            if (verbose) {
                cout << element << ": " << value << endl;;
            }
            return (value);
        }
};

