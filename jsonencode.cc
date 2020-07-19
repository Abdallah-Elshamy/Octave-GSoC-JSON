////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2020 The Octave Project Developers
//
// See the file COPYRIGHT.md in the top-level directory of this
// distribution or <https://octave.org/copyright/>.
//
// This file is part of Octave.
//
// Octave is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Octave is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Octave; see the file COPYING.  If not, see
// <https://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////

#include <octave/oct.h>

DEFUN_DLD (jsonencode, args,, "Encode JSON") // FIXME: Add proper documentation
{
  int nargin = args.length ();
  // jsonencode has two options 'ConvertInfAndNaN' and 'PrettyWriter'
  if (! (nargin == 1 || nargin == 3 || nargin == 5))
    print_usage ();

  // Initialize options with their default values
  bool ConvertInfAndNaN = true;
  bool PrettyWriter = false;

  for (octave_idx_type i = 1; i < nargin; ++i)
    {
      if (! args(i).is_string ())
        error ("jsonencode: Option must be character vector");
      if (! args(i+1).is_bool_scalar ())
        error ("jsonencode: Value for options must be logical scalar");

      std::string option_name = args(i++).string_value ();
      if (option_name == "ConvertInfAndNaN")
        ConvertInfAndNaN = args(i).bool_value ();
      else if (option_name == "PrettyWriter")
        PrettyWriter = args(i).bool_value ();
      else
        error ("jsonencode: Valid options are \'ConvertInfAndNaN\'"
               " and \'PrettyWriter\'");
    }
}
