// timeprinter class - generic timeprinter for numerical models
// Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

/*
Copyright 2014-2015 Thomas E. Barchyn
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
in the distribution with the filename: /docs/STAB_license.txt
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

class time_printer {
    public:
        int max_iterations;                 // the maximum number of iterations
        bool short_sim;                     // boolean to trigger short simulation execution
        int t_loc;                          // internal iteration (advanced when print method called)
        
        time_printer () {
            // constructor is just a placeholder, must call init
        }
        
        void init (int max_iterations_in) {
            /* constructor sets up the max number of iterations
            max_iterations_in = the maximum number of iterations present
            */
            max_iterations = max_iterations_in;
            if (max_iterations <=100) {
                short_sim = true;
            } else {
                short_sim = false;
            }
            t_loc = -1;                      // reset the printer
        }    
    
        void print () {    
            /* prints percentage of time completed. Call every iteration to advance
            the internal counter. The counter prints every 5% along the simulation.
            */
            t_loc++;                        // advance the counter
            
            if (!short_sim) {
                if (t_loc % (max_iterations/20) == 0) {
                    time_t nowTime;
                    struct tm * timeString;
                    time (&nowTime);
                    timeString = localtime (&nowTime);
                    int percentDone = (int)(0.5 + (double)t_loc * 100/ max_iterations);
                    cout << percentDone << "% complete, Time: " << asctime(timeString);
                }
            }
        }
};
