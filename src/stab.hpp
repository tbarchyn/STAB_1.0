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

class stab {
    public:
        /* STAB class: this class contains the model engine and state variables. Methods defined
        below execute the model operations. Note that the model engine depends on the global
        variable t, which is the integer timestep.
        */
        
        simulation sim;                                     // simulation parameters object
        
        tb_raster surf;                                     // surface elevation
        tb_raster bsmt;                                     // basement elevation
        tb_raster ice;                                      // ice elevation
        tb_raster n_ice;                                    // new ice elevation
        
        tb_raster basal_def;                                // ice base deformation
        tb_raster basal_pres;                               // basal pres
        tb_raster zero_elev;                                // elevation of zero basal pres
        tb_raster contact;                                  // contact (0 = cavity, 1 = contact)
        tb_raster iceload;                                  // ice sediment load
        tb_raster n_iceload;                                // new ice sediment load
        
        tb_raster dsurf;                                    // pending surface changes
        tb_raster diceload;                                 // pending iceload changes
        
        tb_raster erodibility;                              // local erodibility
        
        tb_poll p;                                          // polling engine
        stab_log sl;                                        // logging engine
        
        double cell_avg_global_bf;                          // the global basal pres for present iteration
        
        double basal_pres_fudge;                            // the fudge in equality tests, this is best as approx
                                                            // 1e-12 * global basal force to allow for minor math errors
                                                            // inherent in calculations
        
        double global_yll_corner;                           // yll corner for all rasters
        double global_xll_corner;                           // xll corner for all rasters
        
        stab () {
            // constructor is just a placeholder, must call init to initialize the engine
        }
        
        void init (string simfilename) {
            /* method to initialize the model engine

            simfilename = string of the simfile location
            */
            
            cout << "Initializing STAB model engine . . ";
            
            // seed the twister
            timeval tm;                                     // create a timeval to seed the twister
            gettimeofday(&tm, NULL);                        // get the time right now
            init_genrand (tm.tv_usec);                      // and . . seed with milliseconds

            // read the simfile by initializing the sim object
            sim.init (simfilename);
            
            // check the ice_advection rate
            if ((sim.ice_advection * sim.len_timestep) > sim.cellsize) {
                cout << "ERROR: ice_advection * len_timestep is greater than one cellsize" << endl;
                exit (10);
            }
            
            // initialize the rasters
            surf.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            bsmt.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            ice.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            n_ice.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            basal_def.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            basal_pres.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            zero_elev.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            contact.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            iceload.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            n_iceload.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            dsurf.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            diceload.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            erodibility.init (sim.ydim, sim.xdim, sim.yll_corner, sim.xll_corner, sim.cellsize, sim.boundaries_ns, sim.boundaries_ew);
            
            // assign initial values to the rasters
            if (sim.init_type == "flat") {
                init_flat ();
            } else if (sim.init_type == "existing") {
                init_existing ();
            } else {
                cout << "ERROR: undefined initialization!" << endl;
                exit (10);
            }
            
            // initialize the polling engine
            p.init (sim.ydim, sim.xdim);
            
            // set the basal pres
            cell_avg_global_bf = sim.global_basal_pres;
            
            // set basal_pres_fudge
            basal_pres_fudge = 1.0e-12 * sim.global_basal_pres; 
            
            // initialize the logging engine and create the status report
            sl.init ();
            sl.create_status_report ("stab_kinematics.csv");

            if (verbose) {
                print_raster_summaries ();
            }
            
            cout << "complete" << endl;
        }

        void run () {
            /* method to push model forward one iteration. This method moves the ice, then potentially
            pushes the model state outputs, then goes ahead and squishes the sediment, and advects
            and entrains the sediment, and finally applies changes to the surface raster.
            */
            
            move_ice ();                            // move the ice downflow

            if (t % sim.interim_file_output_interval == 0) {
                push_model_state ();                // push file outputs to disk
            }
            
            squish_sediment ();                     // squish sediment laterally based on pressure differences            
            advect_entrainment ();                  // perform advection and entrainment
            erode_basement ();                      // erode basement
            apply_dsurf ();                         // apply the pending changes to surf
            surf_bleed ();                          // apply surface bleed to the model space
            iceload_bleed ();                       // apply changes to the iceload
        }
        
        void finalize () {
            /* method to finalize the model space and shut down model engine.
            */

            push_model_state ();
            
            // if we weren't making images on the fly, we can call the image script and make them now
            if (!sim.on_the_fly_progress_updates) {
                // call with -1 flag to make all images at the end
                plot_progress (sim.Rscript_path, sim.progress_utility_name, sim.file_output_prefix, sim.ydim, sim.xdim, -1);            
            }
            
            // finally, call with -2 to run any final plotting or analyses
            plot_progress (sim.Rscript_path, sim.progress_utility_name, sim.file_output_prefix, sim.ydim, sim.xdim, -2); 
        }
        
        void push_model_state () {
            /* method to push the model state to disk and perform any analysis
            */
            
            // write out a surface raster
            ostringstream surf_output_filename;
            surf_output_filename << sim.file_output_prefix << "_surf_" << t << ".asc";
            surf.write_ascii_raster (surf_output_filename.str());
            
            // write out a pres raster
            ostringstream pres_output_filename;
            pres_output_filename << sim.file_output_prefix << "_pres_" << t << ".asc";
            basal_pres.write_ascii_raster (pres_output_filename.str());
            
            // push out ice raster
            ostringstream ice_output_filename;
            ice_output_filename << sim.file_output_prefix << "_ice_" << t << ".asc";
            ice.write_ascii_raster (ice_output_filename.str());
            
            // push out iceload raster
            ostringstream iceload_output_filename;
            iceload_output_filename << sim.file_output_prefix << "_iceload_" << t << ".asc";
            iceload.write_ascii_raster (iceload_output_filename.str());
            
            // push out basement raster
            ostringstream bsmt_output_filename;
            bsmt_output_filename << sim.file_output_prefix << "_bsmt_" << t << ".asc";
            bsmt.write_ascii_raster (bsmt_output_filename.str());
            
            // create a status report
            sl.surf_mean = surf.mean ();
            sl.surf_min = surf.min ();
            sl.surf_max = surf.max ();
            sl.bsmt_mean = bsmt.mean ();
            sl.bsmt_min = bsmt.min ();
            sl.bsmt_max = bsmt.max ();
            sl.basal_def_mean = basal_def.mean ();
            sl.basal_def_min = basal_def.min ();
            sl.basal_def_max = basal_def.max ();
            sl.contact_mean = contact.mean ();
            sl.total_iceload = iceload.sum () * sim.cellsize * sim.cellsize;
            
            // calculate total bed sediment
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    sl.total_bedsed = sl.total_bedsed + ((surf.ras[y][x] - bsmt.ras[y][x]) * sim.cellsize * sim.cellsize);
                }
            }
            
            sl.total_bleed = sl.surf_bleed + sl.iceload_bleed;          // calculate total bleed
            sl.push_status_report ();                                   // push the report!
            
            // try to run the progress utility to make a plot of the present progress
            if (sim.on_the_fly_progress_updates) {
                plot_progress (sim.Rscript_path, sim.progress_utility_name, sim.file_output_prefix, sim.ydim, sim.xdim, t);
            }
            
            if (verbose) {
                print_raster_summaries ();
            }
        }    
        
        void print_raster_summaries () {
            /* method to print raster summaries to the console
            */
            
            cout << "surf: ";
            surf.print_summary ();
            cout << "dsurf: ";
            dsurf.print_summary ();
            cout << "ice: ";
            ice.print_summary ();
            cout << "basal_def: ";
            basal_def.print_summary ();
            cout << "basal_pres: ";
            basal_pres.print_summary ();
            cout << "contact: ";
            contact.print_summary ();
            cout << "iceload: ";
            iceload.print_summary ();
        }
        
        void init_flat () {
            /* method to initialize the model space with a flat surface
            */
            surf.setvalue (sim.flat_init_sedfill_elev);
            bsmt.setvalue (sim.flat_init_basement_elev);
            erodibility.setvalue (0.0);
            ice.copy_rastercells (surf);
            iceload.setvalue (sim.init_iceload);
            contact.setvalue (1.0);
        }
        
        void init_existing () {
            /* method to initialize the model space with existing surface and basement files
            */
            surf.read_ascii_raster (sim.existing_surf_file);
            bsmt.read_ascii_raster (sim.existing_bsmt_file);
            erodibility.read_ascii_raster (sim.existing_erodibility_file);
            ice.copy_rastercells (surf);
            iceload.setvalue (sim.init_iceload);
            contact.setvalue (1.0);
        }    
                       
        void move_ice () {
            /* method to move the ice downflow 1 timestep and set pres rasters
            */
            double t_wgt;               // target cell weight
            double w_wgt;               // west cell weight
            double ice_temploc;         // ice temporary location
            
            t_wgt = 1.0 - ((sim.ice_advection * sim.len_timestep) / sim.cellsize);
            w_wgt = (sim.ice_advection * sim.len_timestep) / sim.cellsize;
            
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    
                    // calculate temporary height of the ice based on shift
                    ice_temploc = (w_wgt * ice.ras[y][ice.b.w1[x]]) + (t_wgt * ice.ras[y][x]);
                    zero_elev.ras[y][x] = ice_temploc - ((cell_avg_global_bf * sim.len_timestep) / sim.viscosity);
                    
                    // assign basal deformation and basal pres
                    basal_def.ras[y][x] = surf.ras[y][x] - ice_temploc;             // deformation this timestep
                    calc_basal_pres (y, x);
                    
                    // if there is no contact (e.g., a cavity), we need to reassign everything
                    if (contact.ras[y][x] == 0.0) {
                        basal_def.ras[y][x] = zero_elev.ras[y][x] - ice_temploc;  // set negative deformation
                        n_ice.ras[y][x] = zero_elev.ras[y][x];                    // set new ice elevation
                    } else {
                        n_ice.ras[y][x] = surf.ras[y][x];                         // set new ice elevation
                    }
                    
                    // calculate the new ice load at posting points
                    n_iceload.ras[y][x] = (w_wgt * iceload.ras[y][iceload.b.w1[x]]) + (t_wgt * iceload.ras[y][x]);
                }
            }
            
            // change the ice and ice_load rasters after we have calculated new values
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    ice.ras[y][x] = n_ice.ras[y][x];
                    iceload.ras[y][x] = n_iceload.ras[y][x];            
                }
            }
        }
        
        void calc_basal_pres (int y, int x) {
            /* method to calculate the basal pres at a specific site and assign contact raster
            Note that the basal deformation must be properly assigned. 
            y = the target y coordinate
            x = the target x coordinate
            */
            
            basal_pres.ras[y][x] = cell_avg_global_bf + ((basal_def.ras[y][x] / sim.len_timestep) * sim.viscosity);
            if (basal_pres.ras[y][x] - basal_pres_fudge < 0.0) {
                basal_pres.ras[y][x] = 0.0;                 // cavity
                contact.ras[y][x] = 0.0;                    // cavity
            } else {
                contact.ras[y][x] = 1.0;                    // contact with bed
            }
        }    

        void squish_sediment () {
            /* method to squish sediment based on the pressure gradients set up by the move_ice call. This
            is done by visiting each cell and advecting sediment from areas of high pressure to areas of low
            pressure. This is done randomly without replacement, ensuring every cell is visited. This
            must be done randomly to avoid the intractible infinite dependencies that can occur where the advection
            has to be limited by nonlinearities in the constraints (e.g., basement or hit ice situations).
            */
            
            double Q_sq_n;                  // squish to the north
            double Q_sq_s;                  // squish to the south
            double Q_sq_e;                  // squish to the east
            double Q_sq_w;                  // squish to the west
            
            double req_ero;                 // requested erosion
            double av_sed;                  // available sediment
            double reduce_frac;             // reduce fraction

            int y;                          // the y coordinate
            int x;                          // the x coordinate

            p.calc_new_sequence ();         // calculate new random sequence of polls

            for (int i = 0; i < p.len; i++) {
                y = p.ys[i];                // get target y
                x = p.xs[i];                // get target x

                // check for contact of the target cell, no contact no basal pres and no squish
                if (contact.ras[y][x] == 1.0) {
                    
                    // calculate the squish potential
                    Q_sq_n = calc_sq_potential (y, x, surf.b.n1[y], x);
                    Q_sq_s = calc_sq_potential (y, x, surf.b.s1[y], x);
                    Q_sq_e = calc_sq_potential (y, x, y, surf.b.e1[x]);
                    Q_sq_w = calc_sq_potential (y, x, y, surf.b.w1[x]);
                    
                    // check for basement erosion, and adjust the erosion if necessary
                    req_ero = Q_sq_n + Q_sq_s + Q_sq_e + Q_sq_w;            // calculate requested erosion
                    if (((surf.ras[y][x] - req_ero) < bsmt.ras[y][x]) && (req_ero != 0.0)) {
                        av_sed = surf.ras[y][x] - bsmt.ras[y][x];
                        reduce_frac = av_sed / req_ero;
                        Q_sq_n = Q_sq_n * reduce_frac;
                        Q_sq_s = Q_sq_s * reduce_frac;
                        Q_sq_e = Q_sq_e * reduce_frac;
                        Q_sq_w = Q_sq_w * reduce_frac;
                        req_ero = Q_sq_n + Q_sq_s + Q_sq_e + Q_sq_w;        // recalculate requested erosion
                    }
                    
                    // check for possibility of squish beyond ice elevation
                    if (((surf.ras[y][x] - req_ero) < zero_elev.ras[y][x]) && (req_ero != 0.0)) {
                        // we are going to erode too deeply, we must reduce sed transfer
                        reduce_frac = (surf.ras[y][x] - zero_elev.ras[y][x]) / req_ero;
                        
                        // address the problem whereby the surf raster is slightly below the zero elev
                        // and the reduce frac becomes very slightly negative, which screws up the mass balance
                        if (reduce_frac < 0.0) {
                            reduce_frac = reduce_frac * -1.0;
                        }
                        // also check for errors with reduce fracs that are greater than one
                        // this should not occur, but has happened with amplification of math errors.
                        if (reduce_frac > 1.0) {
                            cout << "ERROR: reduce frac > 1.0" << endl;
                            exit (10);
                        }
                        
                        Q_sq_n = Q_sq_n * reduce_frac;
                        Q_sq_s = Q_sq_s * reduce_frac;
                        Q_sq_e = Q_sq_e * reduce_frac;
                        Q_sq_w = Q_sq_w * reduce_frac;
                        req_ero = Q_sq_n + Q_sq_s + Q_sq_e + Q_sq_w;        // recalculate requested erosion
                    }

                    surf.ras[y][x] = surf.ras[y][x] - req_ero;              // lower target cell
                    basal_def.ras[y][x] = basal_def.ras[y][x] - req_ero;    // reduce basal deformation
                    ice.ras[y][x] = surf.ras[y][x];                         // ice is re-assigned to maintain contact
                    calc_basal_pres (y, x);                                 // re-calculate basal pres
                    
                    deposit_sq_sed (surf.b.n1m[y], x, Q_sq_n);              // deposit to the n
                    deposit_sq_sed (surf.b.s1m[y], x, Q_sq_s);              // deposit to the s
                    deposit_sq_sed (y, surf.b.e1m[x], Q_sq_e);              // deposit to the e
                    deposit_sq_sed (y, surf.b.w1m[x], Q_sq_w);              // deposit to the w
                    
                    sl.Q_sq_n = sl.Q_sq_n + Q_sq_n;                         // log the advection to the n
                    sl.Q_sq_s = sl.Q_sq_s + Q_sq_s;                         // log the advection to the s
                    sl.Q_sq_e = sl.Q_sq_e + Q_sq_e;                         // log the advection to the e
                    sl.Q_sq_w = sl.Q_sq_w + Q_sq_w;                         // log the advection to the w
                }
            }
        }
        
        void deposit_sq_sed (int y, int x, double Q) {
            /* deposit sediment at a site
            Arguments:
            y = the deposition site y coordinate
            x = the deposition site x coordinate
            Q = the increase in raster cell (deposition)
            */
            
            if (Q > 0.0) {
                if (y != surf.b.toxic_coord && x != surf.b.toxic_coord) {
                    surf.ras[y][x] = surf.ras[y][x] + Q;

                    // check if depositing into a cavity
                    if (contact.ras[y][x] == 0.0) {
                        // check to see if we are depositing up to the ice level
                        if ((surf.ras[y][x] + 0.0000000001) > ice.ras[y][x]) {
                            surf.ras[y][x] = ice.ras[y][x];         // address minor rounding error
                            contact.ras[y][x] = 1.0;                // set contact
                        } else {
                            // we are not depositing up to ice level, just infilling cavity a bit
                            contact.ras[y][x] = 0.0;
                        }
                    } else {
                        // we are depositing into an area in contact with the ice
                        ice.ras[y][x] = surf.ras[y][x];                     // ice is pushed upwards
                        basal_def.ras[y][x] = basal_def.ras[y][x] + Q;      // basal deformation increased
                    }
                    // recalculate the basal pres
                    calc_basal_pres (y, x);
                } else {
                    sl.total_bleed = sl.total_bleed + Q;
                }
            }
        }    

        double calc_sq_potential (int y, int x, int y_t, int x_t) {
            /* method to calculate the maximum potential squish potential to an adjacent cell
            Arguments:
            y = the target y coordinate
            x = the target x coordinate
            y_t = the test y coordinate
            x_t = the test x coordinate
            */
            
            double df_dx;                               // pres gradient
            double Q_sq;                                // Q_squish
            double max_Q_sq;                            // maximum Q squish
            
            // calculate basal pres gradient
            df_dx = (basal_pres.ras[y][x] - basal_pres.ras[y_t][x_t]) / basal_pres.cellsize;
            
            // calculate the prospective flux
            if (df_dx > 0.0) {
                // assign the maximum desired flux
                Q_sq = df_dx * sim.len_timestep * sim.Q_squish_coef;        // prospective flux

                // check for contact with the target cell
                if (contact.ras[y_t][x_t] == 1.0) {
                    // assign limitation based on pressure equalization
                    max_Q_sq = 0.125 * (basal_def.ras[y][x] - basal_def.ras[y_t][x_t]);
                } else {
                    // assign limitation based on cavity size
                    max_Q_sq = ice.ras[y_t][x_t] - surf.ras[y_t][x_t];
                }
                
                if (Q_sq > max_Q_sq) {
                    Q_sq = max_Q_sq;            // assign limitation
                }
            } else {
                Q_sq = 0.0;
            }
            return (Q_sq);
        }

        void advect_entrainment () {
            /* method to advect and entrain sediment from the raster. This function
            calculates the requested entrainment and advection from a site and determines
            if this will erode all the sediment at a site. If so, the erosion is limited.
            */
            
            dsurf.setvalue (0.0);               // reset dsurf raster
            diceload.setvalue (0.0);            // set the iceload value
            double Q_ad;                        // advection flux
            double Q_en;                        // entrainment flux
            double req_ero;                     // set total requested erosion
            double av_sed;                      // available sediment
            double reduce_frac;                 // reduce fraction
            double overdig;                     // potential overdig
            
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    
                    Q_ad = calc_advection (y, x);               // calc requested advection
                    Q_en = calc_entrainment (y, x);             // calc requested entrainment

                    // set the requested erosion, noting that positive distrainment
                    // will increase the amount of possible flux.
                    req_ero = Q_ad + Q_en;
                    
                    // check for basement incursion and reduce
                    if ((surf.ras[y][x] - req_ero) < bsmt.ras[y][x] && req_ero != 0.0) {
                        av_sed = surf.ras[y][x] - bsmt.ras[y][x];           // available sediment
                        if (Q_en > 0.0) {
                            // if we are entraining sediment, reduce both entrainment and advection
                            reduce_frac = av_sed / req_ero;
                            Q_ad = reduce_frac * Q_ad;
                            Q_en = reduce_frac * Q_en;
                        } else {
                            // else, we are distraining sediment, only reduce advection
                            overdig = req_ero - av_sed;             // the requested overdig
                            Q_ad = Q_ad - overdig;                  // reduce just advection
                        }
                    }
 
                    // set the dsurf changes
                    dsurf.ras[y][x] = dsurf.ras[y][x] - Q_ad - Q_en;
                    diceload.ras[y][x] = diceload.ras[y][x] + Q_en;
                    
                    // deposit sediment downflow
                    if (dsurf.b.e1m[x] != dsurf.b.toxic_coord) {
                        dsurf.ras[y][dsurf.b.e1m[x]] = dsurf.ras[y][dsurf.b.e1m[x]] + Q_ad;
                    } else {
                        sl.total_bleed = sl.total_bleed + Q_ad;
                    }
                    
                    // log fluxes
                    sl.Q_ad = sl.Q_ad + Q_ad;
                    
                    if (Q_en > 0.0) {
                        sl.Q_entrain = sl.Q_entrain + Q_en;
                    } else {
                        sl.Q_distrain = sl.Q_distrain + (-1.0 * Q_en);
                    }
                }
            }
        }
        
        double calc_advection (int y, int x) {
            /* method to calculate the potential advection for a given site
            Arguments:
            y = the target y coordinate
            x = the target x coordinate
            */
            
            double rep_basal_pres;                 // the representative basal pres
            double Q_ad;                            // advection flux
            
            // get a representative basal pres for the boundary between this cell and the cell
            // immediately downflow - this is a boundary average basal pres for the couplet (this
            // could be replaced with a basal pres for the cell, but I leave as is for now).
            rep_basal_pres = (basal_pres.ras[y][x] + basal_pres.ras[y][basal_pres.b.e1[x]]) / 2.0;
            
            // set the sediment advection for the site
            if (rep_basal_pres > 0.0) {
                // calculate sediment flux
                Q_ad = (rep_basal_pres * sim.Q_advection_global * sim.len_timestep) / sim.cellsize;
                Q_ad = Q_ad + ((genrand_real1() - 0.5) * Q_ad * sim.Q_advection_stochasticity);
                
                if (Q_ad < 0.0) {
                    Q_ad = 0.0;             // ensure stochasticity doesnt make flux negative
                }
            } else {
                Q_ad = 0.0;
            }
            return (Q_ad);
        }    

        double calc_entrainment (int y, int x) {
            /* method to calculate entrainment at a given site. This is updated for version 0.5 where
            I have implemented a nonlinear entrainment function.
            Arguments:
            y = the target y coordinate
            x = the target x coordinate
            */
            
            double entrainment;             // the rate of entrainment at the site
            
            // calculate entrainment as a function of basal pres
            if (contact.ras[y][x] == 0.0) {
                // cavity entrainment rate is a predefined constant
                entrainment = sim.entrainment_cavity;
            } else {
                // check to see if we are below vtx_2 point
                if (basal_pres.ras[y][x] < sim.entrainment_vtx_2) {
                    entrainment = (basal_pres.ras[y][x] * sim.entrainment_slp_1) + sim.entrainment_zero;
                } else {
                    entrainment = (((basal_pres.ras[y][x] - sim.entrainment_vtx_2) * sim.entrainment_slp_2) +
                                    (sim.entrainment_vtx_2 * sim.entrainment_slp_1) + sim.entrainment_zero);
                }
            }
            
            // adjust for the length of the timestep
            entrainment = entrainment * sim.len_timestep;
            
            // check to make sure there is enough iceload (noting the distrainment is negative)
            if (iceload.ras[y][x] + entrainment < 0.0) {
                entrainment = (-1.0 * iceload.ras[y][x]);           // only take what is available
            }
            return (entrainment);
        }    
        
        void apply_dsurf () {
            /* method to apply dsurf and modify the surface and iceload rasters
            */
            
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    surf.ras[y][x] = surf.ras[y][x] + dsurf.ras[y][x];
                    iceload.ras[y][x] = iceload.ras[y][x] + diceload.ras[y][x];
                }
            }
        }    
            
        void iceload_bleed () {
            /* method to add or remove a given amount of sediment from the iceload. In cases
            the specified amount of iceload bleed will not be possible because the local iceload
            will be 0.0. Thus there is a global iceload_bleed log variable to keep track of things.
            
            Updated for 0.7 the iceload bleed is a multiplier (as such emulating diffusion). This
            makes the iceload bleed more adaptive, realistic, and hopefully stable.
            
            Updated for 1.0, the iceload bleed can either be diffusive (e.g., a property of the
            local iceload), or not diffusive, in which the straight amount of specified bleed is
            subtracted from the cell.
            */
            
            bool diffusive = false;                             // set whether to bleed diffusively
            double cell_bleed;                                  // the amount to modify each cell
            
            // check to make sure we have a bleed assigned, note there is no fudge as this is straight param read
            if (sim.iceload_bleed != 0.0) {
            
                for (int y = 0; y < sim.ydim; y++) {
                    for (int x = 0; x < sim.xdim; x++) {
                        
                        // calculate cell bleed
                        if (diffusive) {
                            cell_bleed = sim.iceload_bleed * sim.len_timestep * iceload.ras[y][x];
                        } else {
                            cell_bleed = sim.iceload_bleed * sim.len_timestep;
                        }
                        
                        // try to change the iceload at the cell
                        if (iceload.ras[y][x] - cell_bleed < 0.0) {
                            cell_bleed = iceload.ras[y][x];
                            iceload.ras[y][x] = 0.0;
                        } else {
                            iceload.ras[y][x] = iceload.ras[y][x] - cell_bleed;
                        }
                        
                        // log the bleed to the iceload_bleed logger
                        sl.iceload_bleed = sl.iceload_bleed + cell_bleed;
                    }
                }
            }
        }
        
        void surf_bleed () {
            /* method to add or remove a given amount of sediment to the surface of the model
            space, and log the bleed. Note that this algorithm only removes sediment that is
            on the surface, if there is no sediment there, it is not removed.
            */
            
            double cell_bleed;                                  // the amount to modify each cell
            
            // check to make sure we have a bleed assigned, note there is no fudge as this is straight param read
            if (sim.surf_bleed != 0.0) {
            
                for (int y = 0; y < sim.ydim; y++) {
                    for (int x = 0; x < sim.xdim; x++) {
                
                        // calculate cell bleed
                        cell_bleed = sim.surf_bleed * sim.len_timestep;
                        
                        // try to remove the sediment from each cell
                        if (surf.ras[y][x] - cell_bleed < bsmt.ras[y][x]) {
                            cell_bleed = surf.ras[y][x] - bsmt.ras[y][x];
                            surf.ras[y][x] = bsmt.ras[y][x];
                        } else {
                            surf.ras[y][x] = surf.ras[y][x] - cell_bleed;
                        }
                        
                        // log the bleed to the surf bleed logger
                        sl.surf_bleed = sl.surf_bleed + cell_bleed;
                    }
                }
            }
        }
        
        void erode_basement () {
            /* method to erode the basement in exposed regions. Updated for 1.0 with many changes (see changelog).
            Also note that this doesn't re-calculate the basal pressure or deformation or anything, it is just
            straight modification. This will be re-calculated at the beginning of next timestep to be current for
            the next set of squish, advection, and entrainment calculations.
            */
            
            double av_sed;                  // available sediment at a site
            double abrasion;                // abrasion at the cell
            double req_abrasion;            // requested abrasion at a site
            double N_abrasion;              // abrasion from N
            double iceload_abrasion;        // abrasion from iceload
            
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    
                    // check to see if the basement is exposed and we have contact
                    if (contact.ras[y][x] == 1.0) {
                        av_sed = surf.ras[y][x] - bsmt.ras[y][x];
                        if (av_sed < 0.0000000001 && av_sed > -0.0000000001) {
                            // here ice is in direct contact with the basement and we need to evaluate the amount of basement
                            // to erode. This is evaluated as an addition of erosion from both N, and from the iceload (eg,
                            // the Eyles, Krabbendam et al erodent layer theory). The eroded sediment is delivered to both
                            // the iceload and the surface sediment as defined in the parameter file.
                            
                            // calculate the pressure abrasion, and the iceload abrasion, to sum with total requested abrasion
                            N_abrasion = sim.len_timestep * (sim.abrasion_from_N_zero + (basal_pres.ras[y][x] * sim.abrasion_from_N_slope));
                            iceload_abrasion = sim.len_timestep * iceload.ras[y][x] * sim.abrasion_from_iceload;
                            req_abrasion = N_abrasion + iceload_abrasion;
                            
                            // multiply the requested abrasion by the local erodibilty to determine the volume of sediment eroded
                            abrasion = req_abrasion * (sim.global_bsmt_erodibility + erodibility.ras[y][x]);
                            
                            // erode the basement
                            bsmt.ras[y][x] = bsmt.ras[y][x] - abrasion;             // erode basement
                            surf.ras[y][x] = bsmt.ras[y][x];                        // reset surf raster (drops with bsmt)
                            
                            // add sediment to the iceload or surface, and log the abrasion
                            iceload.ras[y][x] = iceload.ras[y][x] + (sim.iceload_surf_return_fraction * abrasion);
                            surf.ras[y][x] = surf.ras[y][x] + ((1.0 - sim.iceload_surf_return_fraction) * abrasion);
                            sl.abrasion = sl.abrasion + abrasion;                   // log the abrasion
                        }
                    }
                }
            }    
        }    
        
        bool check_state () {
            /* method to check the state of the rasters and exit if there is an issue, returns
            true when there is a problem. Not called normally, used in debugging.
            */
            
            bool error = false;
            if (isnan(surf.sum()) || isnan(dsurf.sum()) || isnan(ice.sum()) || isnan(basal_def.sum()) || isnan(basal_pres.sum()) ||
                  isnan(iceload.sum())) {
                cout << "NAN ERROR FOUND!!" << endl;
                print_raster_summaries();
                error = true;
            }
            
            // check for an ice error
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    if (ice.ras[y][x] - surf.ras[y][x] < -0.000000001) {
                        cout << "ice error found: " << ice.ras[y][x] << " surf: " << surf.ras[y][x] << endl;
                        error = true;
                    }
                }
            }
            
            // check for a basement incursion
            for (int y = 0; y < sim.ydim; y++) {
                for (int x = 0; x < sim.xdim; x++) {
                    if (surf.ras[y][x] - bsmt.ras[y][x] < -0.00000001) {
                        cout << "basement error:" << endl;
                        print_raster_summaries();
                        error = true;
                    }
                }
            }
            
            return (error);
        }    
};        
        

