% Copyright (C) Microsoft Corporation. All rights reserved.

%------------------------------------------------------------------------%

% Usage
% 1. In Matlab, run script by pasting the following in the Command Window: 
% >> load_quadrotor_simulink_control_model
%
% 2. Open and start the simulation server.
% 3. In an activated virtual environment, run the following in the
% example_user_scripts directory: 
% python simulink_controller_quadrotor.py

%------------------------------------------------------------------------%

clear variables; clc

disp("Loading 'matlab_control_demo_model_basic.slx' " + ...
    "model and configuring Matlab for Python API control...")

% Set Matlab working directory to the folder containing this M-file
cd(fileparts(which(mfilename)))

guidanceType = 1 % 1 to use waypoints from this script, 2 to use the dashboard located in the navigation subsystem
waypoints = [
            0 0 -7 0; 
            15 0 -7 0;
            15 15 -7 0;
            0 15 -7 0;
            0 0 -7 0;
            0 0 0 0;
            ]

% Open the Simulink model so that it's ready for the client demo script to 
% start/stop
open_system('matlab_control_demo_model_navigation')

% Set the Matlab process to be available for the client demo script to use 
% Python APIs to control
try
  matlab.engine.shareEngine('MATLABEngine')
catch
  disp("Matlab is already set as a shared engine.")
end

disp("Done. Ready for demo Python script to connect and start/stop " + ...
    "the Simulink model.")
