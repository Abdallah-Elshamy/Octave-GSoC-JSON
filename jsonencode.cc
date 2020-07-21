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
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

//! Encodes a scalar Octave value into a numerical JSON value.
//!
//! @param writer RapidJSON's writer that is responsible for generating json.
//! @param obj scalar Octave value.
//! @param ConvertInfAndNaN @c bool that converts @c Inf and @c NaN to @c null.
//!
//! @b Example:
//!
//! @code{.cc}
//! octave_value obj (7);
//! encode_numeric (writer, obj,true);
//! @endcode

template <typename T> void
encode_numeric (T& writer, const octave_value& obj, const bool& ConvertInfAndNaN)
{
  double value =  obj.scalar_value ();
  if (obj.is_bool_scalar ())
    writer.Bool (obj.bool_value ());
  // If value > 999999, MATLAB will encode it in scientific notation (double)
  else if (floor (value) == value && value <= 999999 && value >= -999999)
    writer.Int64 (value);
  // NA values doesn't exist in MATLAB, so I will decode it as null instead
  else if (((octave::math::isnan (value) || std::isinf (value)
            || std::isinf (-value)) && ConvertInfAndNaN)
           || obj.isna ().bool_value ())
    writer.Null ();
  else if (obj.is_double_type ())
  // FIXME: Print in scientific notation
    writer.Double (value);
  else
    error ("jsonencode: Unsupported type.");
}

//! Encodes any Octave object. This function only serves as an interface
//! by choosing which function to call from the previous functions.
//!
//! @param writer RapidJSON's writer that is responsible for generating json.
//! @param obj any @ref octave_value that is supported.
//! @param ConvertInfAndNaN @c bool that converts @c Inf and @c NaN to @c null.
//!
//! @b Example:
//!
//! @code{.cc}
//! octave_value obj (true);
//! encode (writer, obj,true);
//! @endcode

template <typename T> void
encode (T& writer, const octave_value& obj, const bool& ConvertInfAndNaN)
{
  if (obj.is_real_scalar ())
    encode_numeric (writer, obj, ConvertInfAndNaN);
  else
    error ("jsonencode: Unsupported type.");
}

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
  rapidjson::StringBuffer json;
  if (PrettyWriter){}
      // FIXME: fix the error comming from this statement
  /*{
      rapidjson::PrettyWriter<rapidjson::StringBuffer, rapidjson::UTF8<>,
                              rapidjson::UTF8<>, rapidjson::CrtAllocator,
                              rapidjson::kWriteNanAndInfFlag> writer (json);
      encode (writer, args(0));
    }*/
  else
    {
      rapidjson::Writer<rapidjson::StringBuffer, rapidjson::UTF8<>,
                        rapidjson::UTF8<>, rapidjson::CrtAllocator,
                        rapidjson::kWriteNanAndInfFlag> writer (json);
      encode (writer, args(0), ConvertInfAndNaN);
    }

  return octave_value (json.GetString ());
}
