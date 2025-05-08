// Copyright (C) Microsoft Corporation.  All rights reserved.

#include "LightActorBase.h"

// Returns true if able to set intensity on all light components
// False if any item is nullptr
bool ALightActorBase::SetIntensity(float NewIntensity) {
  bool retresult = true;
  for(int i = 0; i < lightComponents.Num(); i++)
  {
    if (lightComponents[i] != nullptr) {
      lightComponents[i]->SetIntensity(NewIntensity);
    } else { retresult = false; }
  }

  return retresult;
}

// Returns true if able to set color on all light components
// False if any item is nullptr
bool ALightActorBase::SetLightFColor(FColor NewLightColor) {
  bool retresult = true;
  for(int i = 0; i < lightComponents.Num(); i++)
  {
    if (lightComponents[i] != nullptr) {
      lightComponents[i]->SetLightFColor(NewLightColor);
    } else { retresult = false; }
  }

  return retresult;
}

// Returns true if able to set radius on all light components
// SetAttenuationRadius is only valid for point and spot lights
// False if any item is nullptr (or directional light)
bool ALightActorBase::SetRadius(float NewRadius) {
  bool retresult = true;
  for(int i = 0; i < lightComponents.Num(); i++)
  {
    ULocalLightComponent* localLightComponent =
        dynamic_cast<ULocalLightComponent*>(lightComponents[i]);

    if (localLightComponent != nullptr) {
      localLightComponent->SetAttenuationRadius(NewRadius);
    } else { retresult = false; }
  }

  return retresult;
}
