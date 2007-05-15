//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcn (Free TTCN) library.

// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/**
 * @file   ttcnWrappers.h
 * @author Mateusz Pusz
 * @date   Fri May 11 08:03:56 2007
 * 
 * @brief  
 * 
 * 
 */

#ifndef __TTCNWRAPPERS_H__
#define __TTCNWRAPPERS_H__


extern "C" {
#include <freettcn/ttcn3/tri.h>
}

namespace freettcn {
  
  namespace TE {
    
    class CTriComponentId {
      TriComponentId _id;
    public:
      CTriComponentId(const TriComponentId &id);
      ~CTriComponentId();
      const TriComponentId &Id() const;
    };
        

  } // namespace TE
  
} // namespace freettcn


#endif /* __TTCNWRAPPERS_H__ */
