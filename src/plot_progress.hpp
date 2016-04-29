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


void plot_progress (string Rscript_path, string progress_utility_name, string file_output_prefix,
                    int ydim, int xdim, int t_loc) {
    /* Function to call the R script to make plots of the model space
    
    Arguments:
    Rscript_path: the path to the Rscript.exe program (or similar)
    progress_utility_name: the path to the progress utility script
    file_output_prefix: the file output prefix for the status files
    ydim: the y dimensions of the model space
    xdim: the x dimensions of the model space
    t_loc: the iteration fed to the R script
    
    If we don't have an Rscript path, or progress_utility name, the program will silently press forward,
    this is to just get on with things. There will of course be no images, so we will notice.
    
    Check R script for the meaning of various t_loc flags.
    */
    
    if (Rscript_path == "" || progress_utility_name == "") {
        if (verbose) {
            cout << "No Rscript_path or progress_utility name defined, silently pressing forward" << endl;
        }
    } else {
        // else, go ahead and run the R script
        int return_value;
        ostringstream system_call;
        system_call << Rscript_path << " " << progress_utility_name;
        system_call << " " << file_output_prefix << " " << ydim << " " << xdim << " " << t_loc;

        if (verbose) {
            cout << "trying the following system call: " << system_call.str() << endl;
        }
        
        return_value = system (system_call.str().c_str());
        
        if (return_value != 0) {
            cout << "ERROR calling the progress_utility R script" << endl;
            cout << "I tried the following system call: " << system_call.str() << endl;
            exit (10);
        }
        
        // try to remove Rplots.pdf, which some versions of R insist on making
        try {
            remove ("Rplots.pdf");
		}
		catch(...) {
            // pass
        }
    }
}



