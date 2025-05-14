"""
Copyright (C) Microsoft Corporation. 
Copyright (C) IAMAI Consulting Corporation.  

MIT License.

Demonstrates how to write fault injection modules with the 
Fault Injection feature in Airsim client. 

This can be injected into any other Airsim client script. 
This script injects Battery fault and a Camera fault at 
preset times.

See Mission_testing_example.py for usage in other mission scripts.
"""

from projectairsim.validate import FaultInjectionModule
from projectairsim import Drone, World
from projectairsim.types import ImageType

def inject_faults(drone: Drone, world: World):
    fault_injection_module = FaultInjectionModule(drone, world)

    # Define a fault using a simple function
    def camera_fault():
        drone.set_depth_of_field_focal_region(
                        camera_id="DownCamera",
                        image_type_id=ImageType.SCENE,
                        max_focal_distance=0.05)
    
    # Define a fault using a lambda function
    # battery_fault = lambda: drone.set_battery_remaining(10)

    # pass in the function (lambda or normal) to the fault injection method
    fault_injection_module.add_fault_injection_at_simtime(camera_fault, 2*(10**9))
    # fault_injection.add_fault_injection_at_simtime(battery_fault, 3*(10**9))

    fault_injection_module.start()