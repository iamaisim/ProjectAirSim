// Copyright (C) Microsoft Corporation.  All rights reserved.

#ifndef MULTIROTOR_API_INCLUDE_ILANDING_GEAR_API_HPP_ //TO-DO: This is not only for multirotor. We need to do some refactoring here.
#define MULTIROTOR_API_INCLUDE_ILANDING_GEAR_API_HPP_


namespace microsoft {
namespace projectairsim {

// Represents public landing gear APIs intended for client
class ILandingGearApi {
 public:
  // return value of these functions is true if command was completed without
  // interruption or timeouts

  virtual bool SetBrakes(float value) = 0;  //value beteween 0 and 1 for all wheels

};

}  // namespace projectairsim
}  // namespace microsoft

#endif  // MULTIROTOR_API_INCLUDE_ILANDING_GEAR_API_HPP_