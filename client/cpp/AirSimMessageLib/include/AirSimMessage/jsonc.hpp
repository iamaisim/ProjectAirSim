// Copyright (C) Microsoft Corporation. 
// Copyright (C) IAMAI  Consulting.  

// MIT License. All rights reserved.

#pragma once
#include "json.hpp"
#include <string>



namespace microsoft {
namespace projectairsim {
namespace client {


//Load JSON Commented file and return it as a json object
//
//Arguments:
//  str_file_path   File path to load
//  pjson_out       json object into which the file is loaded
//
//Returns:
//  (Return)        Errno status code
//  pjson_out       If return is 0, the loaded file contents
extern int LoadJSONCFromFile(const std::string &str_file_path, nlohmann::json *pjson_out);


} //namespace client
} //namespace projectairsim
} //namespace microsoft
