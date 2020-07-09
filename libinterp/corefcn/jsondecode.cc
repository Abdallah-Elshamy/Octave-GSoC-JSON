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

bool
equals (const string_vector& a, const string_vector& b)
{
  if (a.numel () != b.numel ())
    return false;
  octave_idx_type n = a.numel ();
  for (octave_idx_type i = 0; i < n; ++i)
    if (a(i) != b(i))
      return false;

  return true;
}

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
  else
    error ("jsondecode.cc: Unidentified type.");
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
decode_numeric_array (const rapidjson::Value& val)
{
  NDArray retval (dim_vector (val.Size (), 1));
  octave_idx_type index = 0;
  for (auto& elem : val.GetArray ())
    {
      if(elem.IsNull ())
        retval(index) = octave::feval ("NaN")(0).double_value ();
      else
        retval(index) = decode_number (elem).double_value ();
      index++;
    }
  return retval;
}

octave_value
decode_boolean_array (const rapidjson::Value& val)
{
  boolNDArray retval (dim_vector (val.Size (), 1));
  octave_idx_type index = 0;
  for (auto& elem : val.GetArray ())
    retval(index++) = elem.GetBool ();
  return retval;
}

octave_value
decode_string_and_mixed_array (const rapidjson::Value& val)
{
  Cell retval (dim_vector (val.Size (), 1));
  octave_idx_type index = 0;
  for (auto& elem : val.GetArray ())
    retval(index++) = decode (elem);
  return retval;
}

octave_value
decode_object_array (const rapidjson::Value& val)
{
  Cell struct_cell = decode_string_and_mixed_array (val).cell_value ();
  string_vector field_names = struct_cell(0).scalar_map_value ().fieldnames ();
  bool same_field_names = 1;
  for (octave_idx_type i = 1; i < struct_cell.numel (); ++i)
      if (! equals (field_names, struct_cell(i).scalar_map_value ().fieldnames ()))
        same_field_names = 0;
  if (same_field_names)
  {
    octave_map struct_array;
    Cell value (dim_vector (struct_cell.numel (), 1));
    for (octave_idx_type i = 0; i < field_names.numel (); ++i)
      {
        for (octave_idx_type k = 0; k < struct_cell.numel (); ++k)
          value(k) = struct_cell(k).scalar_map_value ().getfield (field_names(i));
        struct_array.assign (field_names(i), value);
      }
    return octave_value (struct_array);
  }
  else
    return struct_cell;
}


octave_value
decode_array_of_arrays (const rapidjson::Value& val)
{
  // Some arrays should be decoded as NDArrays and others as cell arrays
  Cell cell = decode_string_and_mixed_array(val).cell_value ();
  // Only arrays with sub arrays of booleans and numericals will return NDArray
  bool is_bool = cell(0).is_bool_matrix ();
  dim_vector sub_array_dims = cell(0).dims ();
  octave_idx_type sub_array_ndims = cell(0).ndims ();
  octave_idx_type cell_numel = cell.numel ();
  for (octave_idx_type i = 0; i < cell_numel; ++i)
    {
      // If one element is cell return the cell array as at least one of
      // the sub arrays area either an array of: strings , objects or mixed array.
      if (cell(i).iscell ())
        return cell;
      // If not the same dim of elements or dim = 0 return cell array
      if (cell(i).dims () != sub_array_dims || sub_array_dims == dim_vector ())
        return cell;
      // If not numeric sub arrays only or bool sub arrays only return cell array
      if(cell(i).is_bool_matrix () != is_bool)
        return cell;
    }
  // Calculate the dims of the output array
  dim_vector array_dims;
	array_dims.resize (sub_array_ndims + 1);
	array_dims(0) = cell_numel;
	for (int i = 1; i < sub_array_ndims + 1; i++)
  		array_dims(i) = sub_array_dims(i-1);
  NDArray array (array_dims);

  // Populate the array with specific order to generate MATLAB-identical output
  octave_idx_type array_index = 0;
  for (octave_idx_type i = 0; i < array.numel () / cell_numel; ++i)
    for (octave_idx_type k = 0; k < cell_numel; ++k)
        array(array_index++) = cell(k).array_value()(i);
  return array;
}

octave_value
decode_array (const rapidjson::Value& val)
{
  // Handle empty arrays
  if (val.Empty ())
    return NDArray (dim_vector (0,0));

  // Compare with other elements to know if the array has multible types
  rapidjson::Type array_type = val[0].GetType ();
  // Check if the array is numeric and if it has multible types
  bool same_type = 1, is_numeric = 1;
  for (auto& elem : val.GetArray ())
    {
      rapidjson::Type current_elem_type = elem.GetType ();
      if (! (current_elem_type == rapidjson::kNullType
          || current_elem_type == rapidjson::kNumberType))
        is_numeric = 0;
      if (current_elem_type != array_type)
        // RapidJSON doesn't have kBoolean Type it has kTrueType and kFalseType
        if (! ((current_elem_type == rapidjson::kTrueType
                && array_type == rapidjson::kFalseType)
            || (current_elem_type == rapidjson::kFalseType
                && array_type == rapidjson::kTrueType)))
          same_type = 0;
    }
  if (is_numeric)
    return decode_numeric_array (val);
  if (same_type)
    {
      if (array_type == rapidjson::kTrueType
          || array_type == rapidjson::kFalseType)
        return decode_boolean_array (val);
      else if (array_type == rapidjson::kStringType)
        return decode_string_and_mixed_array (val);
      else if (array_type == rapidjson::kObjectType)
        return decode_object_array (val);
      else if (array_type == rapidjson::kArrayType)
        return decode_array_of_arrays (val);
      else
        error ("jsondecode.cc: Unidentified type.");
    }
  else
    return decode_string_and_mixed_array (val);
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
  else if (val.IsNull ())
    return NDArray (dim_vector (0,0));
  else if (val.IsArray ())
    return decode_array (val);
  else
    error ("jsondecode.cc: Unidentified type.");
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
