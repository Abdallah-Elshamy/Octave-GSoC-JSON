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
#include <octave/parse.h>
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

octave_value
decode (const rapidjson::Value& val);

octave_value
decode_number (const rapidjson::Value& val)
{
  if (val.IsUint ())
    return octave_value (val.GetUint ());
  else if (val.IsInt ())
    return octave_value (val.GetInt ());
  else if (val.IsUint64 ())
    return octave_value (val.GetUint64 ());
  else if (val.IsInt64 ())
    return octave_value (val.GetInt64 ());
  else if (val.IsDouble ())
    return octave_value (val.GetDouble ());
}

octave_value
decode_object (const rapidjson::Value& val)
{
  octave_scalar_map retval;
  for (auto& pair : val.GetObject ())
    {
      std::string fcn_name = "matlab.lang.makeValidName";
      octave_value_list args = octave_value_list (pair.name.GetString ());
      std::string validName = octave::feval (fcn_name,args)(0).string_value ();
      retval.assign(validName, decode(pair.value));
    }
  return octave_value (retval);
}

octave_value
decode (const rapidjson::Value& val)
{
  if (val.IsBool ())
    return octave_value (val.GetBool ());
  else if (val.IsNumber ())
    return decode_number (val);
  else if (val.IsString ())
    return octave_value (val.GetString ());
  else if (val.IsObject ())
    return decode_object (val);
}

DEFUN_DLD (jsondecode, args,, "Decode JSON") // FIXME: Add proper documentation
{
  int nargin = args.length ();

  if (nargin != 1)
    print_usage ();

  if(! args(0).is_string ())
    error ("jsondecode: The input must be a character string");

  std::string json = args (0).string_value ();
  rapidjson::Document d;

  d.Parse <rapidjson::kParseNanAndInfFlag>(json.c_str ());

  if (d.HasParseError ())
    error("jsondecode: Parse error at offset %u: %s\n",
          (unsigned)d.GetErrorOffset (),
          rapidjson::GetParseError_En (d.GetParseError ()));
  return decode (d);
}
