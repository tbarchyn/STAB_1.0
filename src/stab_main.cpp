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

/*
Disclaimer: I've tried to make this program error free, but I cannot guarantee it.
I will not be responsible for any problems any errors may cause. As a result, you
are completely responsible for verifying the code and ensuring that it does what
you think it does. Additionally, you are fully responsible for understanding artefacts
in the model. I've tried to write comments in places where artefacts may creep up
but to fully understand artefacts you must perform many test simulations and evaluate
the outputs comprehensively.

Use: please let me know if you are using this program (tbarchyn@gmail.com). There
are two reasons: (i) if I find a bug I can let you know (I cannot let you know about
the bug if I don't have your contact information, this is in your interest), and 
(ii) I need some basic idea of user numbers to justify releasing things like this
program. If nobody is using the code, it is hard for me to justify any additional
effort to make the program easier to use for other people. A quick email would be
very much appreciated! I'll even buy you a drink of your choice if I see you at
a conference! Thanks!

Compiling notes: this program does not work well with 'fast math approximations'
which are common in optimizations. I've done very little optimization of the code
principally because the time involved in optimization may or may not pay back
in reduced run times, and I prefer to have code I can read and understand than
code that is less clear, but faster. I have compiled this program with g++ on Windows
and Linux. I have not tested Visual C++ or any other compilers or platforms. There
is an if statement on lines 224 in simulation.hpp that is a workaround to deal
with compiler differences between Linux and Windows for reading text strings.
This is a likely source of problems if using some other compiler. Let me know
your experiences and modifications if using some other compiler (tbarchyn@gmail.com).
*/


#define REVISION 1.0

// system includes
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <string>
#include <math.h>
#include <sys/time.h>

using namespace std;

// global time variables
int t = 0;                      // integer time for discrete time intervals

// other global variables
const double pi = 3.1415926535897932384626433;
bool verbose = false;           // toggle verbose outputs for those in need of lots of model feedback

// model headers
#include "mersenne_twister.h"   // random number generator
#include "timeprinter.hpp"      // time printer accessory function
#include "plot_progress.hpp"    // wrapper to call R imaging scripts
#include "tb_raster.hpp"        // model raster and boundaries objects
#include "tb_poll.hpp"          // random site poller
#include "simulation.hpp"       // simulation class which stores local simulation properties
#include "stab_log.hpp"         // logging engine
#include "stab.hpp"             // model engine

// MAIN
int main(int nArgs, char *pszArgs[]) {
    
    // print welcome message to the console
    cout << "------------------------------------------------------------------" << endl;
    cout << "WELCOME TO THE STAB: the 'Subglacial Till Advection and Bedform' model" << endl;
    cout << "Version number: " << REVISION << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "Copyright 2014-2016 Thomas E. Barchyn" << endl;
    cout << "Contact: Thomas E. Barchyn [tbarchyn@gmail.com]" << endl;
    cout << "This project was developed with input from Thomas P.F. Dowling," << endl;
    cout << "Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate" << endl;
    cout << "citation of the relavent publications." << endl;
    cout << "------------------------------------------------------------------" << endl;
    cout << "This program is free software: you can redistribute it and/or modify" << endl;
    cout << "it under the terms of the GNU General Public License as published by" << endl;
    cout << "the Free Software Foundation, either version 3 of the License, or" << endl;
    cout << "(at your option) any later version." << endl;
    cout << endl;
    cout << "This program is distributed in the hope that it will be useful," << endl;
    cout << "but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl;
    cout << "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl;
    cout << "GNU General Public License for more details." << endl;
    cout << endl;
    cout << "Please familiarize yourself with the license of this tool, available" << endl;
    cout << "in the distribution with the filename: /docs/license.txt" << endl;
    cout << "You should have received a copy of the GNU General Public License" << endl;
    cout << "along with this program.  If not, see <http://www.gnu.org/licenses/>." << endl;
    cout << "------------------------------------------------------------------" << endl;
    
    // sort out the arguments
    // Argument 1 = simfilename: this is the name of the simfile which stores simulation properties
    // Argument 2 (optional) = -v: this sets the verbose flag high and the program outputs additional info
    
    if (nArgs == 1) {
        cout << "ERROR: this program requires 1 argument, which is the simfile path" << endl;
        cout << "There is an optional second argument, which is '-v' to toggle verbose output" << endl;
        exit(2);
    }
    
    string simfilename = pszArgs[1];            // grab the first argument
    
    if (nArgs == 3) {
        string verbose_argument = pszArgs[2];
        if (verbose_argument == "-v") {
            verbose = true;                     // set verbose flag high
        } else {
            cout << "ERROR: cannot parse your second argument: " << verbose_argument << endl;
            exit (2);
        }
    }
    
    if (verbose) {
        for (int i = 0; i < nArgs; i++) {
            cout << "argument " << i << ": " << pszArgs[i] << endl;
        }
    }
    
    // initialize the simulation space and model engine
    cout << "------------------------------------------------------------------" << endl;
    cout << "INITIALIZING" << endl;
    stab stab;                                  // create the model engine
    stab.init (simfilename);                    // initialize model engine
    time_printer tp;                            // create the time printer
    tp.init (stab.sim.max_iterations);          // initialize the time printer
    
    // run time loop
    cout << "------------------------------------------------------------------" << endl;
    cout << "ENTERING TIME LOOP" << endl;
    while (t < stab.sim.max_iterations) {
        stab.run ();                            // run model engine forward 1 iteration
        tp.print ();                            // try to print the time
        t++;                                    // increment the integer time
    }
    
    // finalize the model space
    cout << "------------------------------------------------------------------" << endl;
    cout << "FINALIZING AND RUNNING POST-RUN ANALYSES" << endl;
    stab.finalize ();                            // finalize the model engine
    
    cout << "Simulation complete!" << endl;
    return (0);
}




