% Copyright (C) Microsoft Corporation. All rights reserved.

%------------------------------------------------------------------------%

% Usage
% 1. In Matlab, run script by pasting the following in the Command Window:
% >> load_quadtiltrotor_simulink_physics_model
%
% 2. Open and start the simulation server.
% 3. In an activated virtual environment, run the following in the
% example_user_scripts directory:
% python simulink_physics_quadtiltrotor.py

%------------------------------------------------------------------------%

clear variables; clc

% Define Quadtiltrotor Parameters

disp("Loading quadtiltrotor parameters...")

mass = 10; % bumped up for better realism
% inertia taken from fast_physics.cpp
inertia = [14.8947001, 0, 0;0, 14.8947001, 0;0, 0, 17.8016682];
drag_coeff_linear = 0.2; % decreased to help tilt transition
drag_coeff_angular = 0.2; % decreased to help tilt transition
cross_section_area = 9.5; % happy medium computed between xy and xz frame
                          % areas from config
initial_xyz = [2, 3, -4]; % copied over from scene config file
initial_rpy = [0, 0, 0];
initial_vel_xyz = [0, 0, 0];
initial_ang_rot_xyz = [0, 0, 0];

% Enable fixed-wing forces in Simulink
enable_FW_forces = 1;

% Aero constants
span = 2.795;
chord = 0.351342;
elarm = 0.121; % was 1.21, decreased to smooth pitch
CL0 = 0.38;
CLa = 18.5;
CLa_dot = 2.64;
CLq = 7.4;
CLDe = 0; % was 0.24
CLDf = 0.4;
CD0 = 0.022;
A1 = 0.007;
Apolar = 0.057;
CYb = -1.098;
CYDr = 0.143;
Clb = 0; % was -0.296
Clp = 0; % was -1.96
Clr = 0; % was 0.103
ClDa = 0.1695;
ClDr = 0; % was 0.106
Cm0 = 0; % was 0.3
Cma = 0; % was -1.239
Cmq = 0; % was -2.4
CmDe = -3.2;
CmDf = 0; % was -0.021
Cnb = 0; % was 0.277
Cnp = -0.0889;
Cnr = -0.19997;
CnDa = -0.023;
CnDr = 0; % was 0.277

%------------------------------------------------------------------------%

disp("Loading 'matlab_physics_demo_model.slx' " + ...
    "model and configuring Matlab for Python API control...")

% Set Matlab working directory to the folder containing this M-file
cd(fileparts(which(mfilename)))

% Open the Simulink model so that it's ready for the client demo script to
% start/stop
open_system('matlab_physics_demo_model')

% Set the Matlab process to be available for the client demo script to use
% Python APIs to control
try
  matlab.engine.shareEngine('MATLABEngine')
catch
  disp("Matlab is already set as a shared engine.")
end

disp("Done. Ready for demo Python script to connect and start/stop " + ...
    "the Simulink model.")
