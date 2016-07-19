/*
STAB: subglacial till advection and bedforms
Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

Copyright 2014-2016 Thomas E. Barchyn
Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

This project was developed with input from Thomas P.F. Dowling,
Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate
citation of the relavent publications.

Barchyn, T. E., T. P. F. Dowling, C. R. Stokes, and C. H. Hugenholtz (2016), 
Subglacial bed form morphology controlled by ice speed and sediment thickness,
Geophys. Res. Lett., 43, doi:10.1002/2016GL069558

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

class stab_log {
    public:
        /* This class contains the logging variables for the stab model and writing methods.
        For the most part, all logging variables are ammended externally. Note that when a
        report is filed, all the variables are reset (this is different than MGD!).
        */
        
        double Q_ad;                                        // total advection flux
        double Q_sq_n;                                      // total squish flux to north
        double Q_sq_s;                                      // total squish flux to south
        double Q_sq_e;                                      // total squish flux to east
        double Q_sq_w;                                      // total squish flux to west
        double Q_entrain;                                   // total entrainment into iceload
        double Q_distrain;                                  // total distrainment from iceload    
            
        int settle_count;                                   // count of settle calls

        double total_bleed;                                 // sum of total sediment bleed out of the model space
        double iceload_bleed;                               // iceload bleed (can be positive, loss of sediment up
                                                            // into the upper ice, or negative, distrainment from the
                                                            // upper ice.
        double surf_bleed;                                  // total bleed directly from the surface
        
        double abrasion;                                    // total abrasion

        double surf_mean;                                   // surf mean
        double surf_min;                                    // surf min
        double surf_max;                                    // surf max
        double bsmt_mean;                                   // bsmt mean    
        double bsmt_min;                                    // bsmt min
        double bsmt_max;                                    // bsmt max
        double basal_def_mean;                              // basal def mean
        double basal_def_min;                               // basal def min
        double basal_def_max;                               // basal def max
        double contact_mean;                                // contact mean
        
        double total_iceload;                               // total sediment in the iceload
        double total_bedsed;                                // total sediment in the bed

        ofstream ofile;                                     // output file object
        string ofile_name;                                  // output file name
        string delimeter;                                   // delimeter        
        string eol_char;                                    // end of line character
        
        stab_log () {
            // constructor is just a placeholder, must call init at runtime
        }
        
        void init () {
            /* method to initialize the stab logger and reset all the variables
            */
            reset_vars ();
        }
        
        void reset_vars () {
            /* method reset all the internal variables
            */
            Q_ad = 0.0;
            Q_sq_n = 0.0;
            Q_sq_s = 0.0;
            Q_sq_e = 0.0;
            Q_sq_w = 0.0;
            Q_entrain = 0.0;
            Q_distrain = 0.0;
            total_bleed = 0.0;
            settle_count = 0;
            iceload_bleed = 0.0;
            surf_bleed = 0.0;
            abrasion = 0.0;
            
            surf_mean = 0.0;
            surf_min = 0.0;
            surf_max = 0.0;
            bsmt_mean = 0.0;  
            bsmt_min = 0.0;
            bsmt_max = 0.0;
            basal_def_mean = 0.0;
            basal_def_min = 0.0;
            basal_def_max = 0.0;
            contact_mean = 0.0;
            
            total_iceload = 0.0;
            total_bedsed = 0.0;
        }    

        void create_status_report (string fname) {
            /* method to initialize the status file with the header row
            Argument:
            logfilename: the output filename
            
            Notes: we will subsequently open the file and append each row of statistics
            as the simulation proceeds. This function also sets the class variable holding
            the output filename, so it must be called first before pushing any statuses.
            */
            
            ofile_name = fname;                         // set the output filename
            
            delimeter = ",";
            eol_char = "\n";
            
            ofile.open (ofile_name.c_str());
            
            // push the headers to the file
            ofile << "t" << delimeter;
            ofile << "Q_ad" << delimeter;
            ofile << "Q_sq_n" << delimeter;
            ofile << "Q_sq_s" << delimeter;
            ofile << "Q_sq_e" << delimeter;
            ofile << "Q_sq_w" << delimeter;
            ofile << "Q_entrain" << delimeter;
            ofile << "Q_distrain" << delimeter;
            
            ofile << "total_bleed" << delimeter;
            ofile << "settle_count" << delimeter;   
            ofile << "iceload_bleed" << delimeter;
            ofile << "surf_bleed" << delimeter;
            ofile << "abrasion" << delimeter;
            
            ofile << "surf_mean" << delimeter;
            ofile << "surf_min" << delimeter;
            ofile << "surf_max" << delimeter;
            
            ofile << "bsmt_mean" << delimeter;
            ofile << "bsmt_min" << delimeter;
            ofile << "bsmt_max" << delimeter;
            
            ofile << "basal_def_mean" << delimeter;
            ofile << "basal_def_min" << delimeter;
            ofile << "basal_def_max" << delimeter;
            ofile << "contact_mean" << delimeter;
            
            ofile << "total_iceload" << delimeter;
            ofile << "total_bedsed" << eol_char;
            
            ofile.close();
        }
        
        void push_status_report () {
            /* method to push out a status report row to the status report
            Notes: the create_status_report method must have been called previously, otherwise
            there will be no filename set properly, and no headers. Note that this method resets
            the variables after outputting them, not the same as MGD! Also note that this will
            matter if the pushes are not regular and will need to be taken into account in analysis.
            */
            
            ofile.open (ofile_name.c_str(), ios::app);      // open in append mode
            
            // push the results to the file
            ofile << t << delimeter;
            ofile << Q_ad << delimeter;
            ofile << Q_sq_n << delimeter;
            ofile << Q_sq_s << delimeter;
            ofile << Q_sq_e << delimeter;
            ofile << Q_sq_w << delimeter;
            ofile << Q_entrain << delimeter;
            ofile << Q_distrain << delimeter;
            
            ofile << total_bleed << delimeter;
            ofile << settle_count << delimeter;   
            ofile << iceload_bleed << delimeter;
            ofile << surf_bleed << delimeter;
            ofile << abrasion << delimeter;
            
            ofile << surf_mean << delimeter;
            ofile << surf_min << delimeter;
            ofile << surf_max << delimeter;
            
            ofile << bsmt_mean << delimeter;
            ofile << bsmt_min << delimeter;
            ofile << bsmt_max << delimeter;
            
            ofile << basal_def_mean << delimeter;
            ofile << basal_def_min << delimeter;
            ofile << basal_def_max << delimeter;
            ofile << contact_mean << delimeter;
            
            ofile << total_iceload << delimeter;
            ofile << total_bedsed << eol_char;

            ofile.close();
            
            reset_vars ();                                  // reset variables
        }
            
};

