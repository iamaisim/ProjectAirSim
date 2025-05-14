% Copyright (C) Microsoft Corporation. 
% Copyright (C) IAMAI  Consulting.  

% MIT License.

%------------------------------------------------------------------------%

% Usage
% 1. In Matlab, run script by pasting the following in the Command Window:
% >> load_quadrotor_simulink_combined_model
%
% 2. Open and start the simulation server.
% 3. In an activated virtual environment, run the following in the
% example_user_scripts directory:
% python simulink_combined_quadrotor.py

%------------------------------------------------------------------------%

clear variables; clc

disp("Loading quadrotor parameters...")

% Define Quadrotor Parameters

mass = 1;
inertia = [0.02, 0, 0;0, 0.02, 0;0, 0, 0.04];
drag_coeff_linear = -0.5;
drag_coeff_angular = -0.5;
cross_section_area = 1;
initial_xyz = [0, 0, -4];
initial_rpy = [0, 0, 0];
initial_vel_xyz = [0, 0, 0];
initial_ang_rot_xyz = [0, 0, 0];

% Disable fixed-wing forces in Simulink
enable_FW_forces = 0;

% Dummy fixed-wing aero constants. Set to zero because quadrotor does not
% have fixed-wing components.
span = 0;
chord = 1;
elarm = 0;
CL0 = 0;
CLa = 0;
CLa_dot = 0;
CLq = 0;
CLDe = 0;
CLDf = 0;
CD0 = 0;
A1 = 0;
Apolar = 0;
CYb = 0;
CYDr = 0;
Clb = 0;
Clp = 0;
Clr = 0;
ClDa = 0;
ClDr = 0;
Cm0 = 0;
Cma = 0;
Cmq = 0;
CmDe = 0;
CmDf = 0;
Cnb = 0;
Cnp = 0;
Cnr = 0;
CnDa = 0;
CnDr = 0;

%------------------------------------------------------------------------%

disp("Loading 'matlab_combined_demo_model.slx' " + ...
    "model and configuring Matlab for Python API control...")

% Set Matlab working directory to the folder containing this M-file
cd(fileparts(which(mfilename)))

% Open the Simulink model so that it's ready for the client demo script to
% start/stop
open_system('matlab_combined_demo_model')

% Set the Matlab process to be available for the client demo script to use
% Python APIs to control
try
  matlab.engine.shareEngine('MATLABEngine')
catch
  disp("Matlab is already set as a shared engine.")
end

disp("Done. Ready for demo Python script to connect and start/stop " + ...
    "the Simulink model.")
