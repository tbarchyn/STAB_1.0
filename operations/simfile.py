# STAB: subglacial till advection and bedforms
# Thomas E. Barchyn - University of Calgary, Calgary, AB, Canada

# Copyright 2014-2016 Thomas E. Barchyn
# Contact: Thomas E. Barchyn [tbarchyn@gmail.com]

# This project was developed with input from Thomas P.F. Dowling,
# Chris R. Stokes, and Chris H. Hugenholtz. We would appreciate
# citation of the relavent publications.

# Barchyn, T. E., T. P. F. Dowling, C. R. Stokes, and C. H. Hugenholtz (2016), 
# Subglacial bed form morphology controlled by ice speed and sediment thickness,
# Geophys. Res. Lett., 43, doi:10.1002/2016GL069558

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

import sys


class simfile:
    """
    Generic echo class of the simfile, that can be used to create batches of simfiles
    """
    
    def __init__ (self, reference_simfilename):
        """
        This initializes with the reference.simfile to find all the elements that is
        in the present version of the program. The elements are denoted in the reference
        simfile by a '>' char leading each line.
        
        Argument is path to reference.simfile
        """
        
        # open the file
        try:
            f = open (reference_simfilename, 'r')
        except:
            print ('ERROR: cannot find ' + reference_simfilename)
            sys.exit()
        
        # read through the file, looking for the elements
        self.sim_elements = [[]]
        eof = False
        while not eof:
            line = f.readline()
            if line == '':
                eof = True
            line = line.split()         # this calls a 'whitespace split'            

            if len(line) > 2:
                if line[0] == '>':
                    line[2] = line[2].replace('\n', '')
                    line[2] = line[2].replace('\cr', '')
                    
                    self.sim_elements.append ([line[1], line[2]])
                
        f.close()
        
        self.sim_elements = self.sim_elements[1:]   # remove the first blank

        return
    
    def assign_element (self, element, value):
        """
        Method to assign a value to an element
        element = the element tagname
        value = the value to assign
        """
        
        # find the element in the list and assign it
        found_it = False
        for i in range(0, len(self.sim_elements)):
            if self.sim_elements[i][0] == element:
                self.sim_elements[i][1] = value
                found_it = True
        
        if not found_it:
            print ('ERROR: cannot find ' + element + ', no assignment made!')
                
        return
    
    def get_element (self, element):
        """
        Method to get a certain element from the list
        element = the element desired
        """
        return_value = ''
        
        # find the element in the list and return it
        for i in range(0, len(self.sim_elements)):
            if self.sim_elements[i][0] == element:
                return_value = self.sim_elements[i][1]
        
        if return_value == '':
            print ('ERROR: cannot find simfile element: ' + element)
            sys.exit()
            
        return return_value
        
    def read_simfile (self, simfilename):
        """
        Method to read an existing simfile and assign the elements
        simfilename = filename of simfile
        """
        
        f.open ('simfilename', 'r')
        
        eof = False
        while not eof:
            line = f.readline()
            line = line.split (' ')
            if line[0] == '>':
                line[2] = line[2].replace('\n', '')
                line[2] = line[2].replace('\cr', '')
                
                self.assign_element (line[1], line[2])
            
            if line[0] == '':
                eof = True
                
        f.close()

    def write_simfile (self, simfilename):
        """
        Method to write a simfile for ingestion to the model
        simfilename = filename of the simfile to write
        """
        
        f = open (simfilename, 'w')
        
        for i in range(0, len(self.sim_elements)):
            f.write ('> ' + self.sim_elements[i][0] + ' ' + self.sim_elements[i][1] + '\n')
        
        f.close()
        
        return
        
        
        
        
        
        
        
        
        