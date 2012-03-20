/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


%header %{
#include <sstream>
#include "pdb_remarks.hpp"
%}

namespace loos {
  class Remarks {
  public:
    int numberOf(void) const;
    int size(void) const;
    std::string get(const int i) const;
    void add(const std::string s);
    void add(const std::vector<std::string>& s);
    void erase(const int i);
    std::vector<std::string> allRemarks(void) const;

    %extend {
      std::string __getitiem__(const int i) {
        return((*$self)[i]);
      }

      void __setitem__(const int i, const std::string& s) {
        (*$self)[i] = s;
      }

      char* __str__() {
        std::ostringstream oss;
        oss << *$self;
        size_t n = oss.str().size();
        char* buf = new char[n+1];
        strncpy(buf, oss.str().c_str(), n+1);
        return(buf);
      }
    }

  };


}