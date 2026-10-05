% Copyright (C) 2025 IAMAI CONSULTING CORP
%
% MIT License.

function model = load_external_quadrotor_simulink_control_model()
% Build a standard-command example using the supplied quadrotor controller.
root = fileparts(mfilename('fullpath'));
addpath(root);
model = 'matlab_external_quadrotor_demo';
template = 'matlab_control_demo_model_navigation';
load_system(fullfile(root, [template '.slx']));
new_system(model);
for name = {'CoordinateConversion', 'Original Controller Model', 'Output Conditioning'}
    add_block([template '/' name{1}], [model '/' name{1}]);
end
close_system(template, 0);

effectors = {'Prop_FR_actuator', 'rotor', 'normalized'; ...
            'Prop_RL_actuator', 'rotor', 'normalized'; ...
            'Prop_FL_actuator', 'rotor', 'normalized'; ...
            'Prop_RR_actuator', 'rotor', 'normalized'};
workspace = get_param(model, 'ModelWorkspace');
workspace.assignin('effectors', effectors);
add_block('simulink/User-Defined Functions/S-Function', [model '/External controller'], ...
    'FunctionName', 'matlab_vtol_control_model_sfcn', ...
    'Parameters', '''127.0.0.1'',''8998'',0.003,effectors', ...
    'Position', [30 100 230 400]);
add_block('simulink/Signal Routing/Mux', [model '/Control values'], ...
    'Inputs', '4', 'Position', [1450 200 1455 360]);
for port = 1:4
    add_line(model, ['Output Conditioning/' num2str(port)], ['Control values/' num2str(port)]);
end
add_line(model, 'Control values/1', 'External controller/1');
add_line(model, 'External controller/1', 'CoordinateConversion/1');
for port = 1:4
    state_ports = [1 2 4 5];
    add_line(model, ['CoordinateConversion/' num2str(state_ports(port))], ...
        ['Original Controller Model/' num2str(port)]);
end
add_line(model, 'Original Controller Model/1', 'Output Conditioning/1');
add_block('simulink/Signal Routing/Mux', [model '/Command state'], ...
    'Inputs', '2', 'Position', [300 500 305 580]);
add_line(model, 'External controller/1', 'Command state/1');
add_line(model, 'External controller/8', 'Command state/2');
add_block(sprintf('simulink/User-Defined\nFunctions/Interpreted MATLAB\nFunction'), ...
    [model '/Standard command guidance'], ...
    'MATLABFcn', 'external_quadrotor_setpoint(u)', 'OutputDimensions', '6', ...
    'Position', [400 500 620 580]);
add_line(model, 'Command state/1', 'Standard command guidance/1');
add_block('simulink/Signal Routing/Demux', [model '/Reference and status'], ...
    'Outputs', '[4 2]', 'Position', [700 500 705 580]);
add_line(model, 'Standard command guidance/1', 'Reference and status/1');
add_line(model, 'Reference and status/1', 'Original Controller Model/5');
add_line(model, 'Reference and status/2', 'External controller/2');
controller = [model '/Original Controller Model'];
delete_line(controller, 'Constant1/1', 'Position & Rate Controller1/6');
delete_block([controller '/Constant1']);
add_block('simulink/Sources/In1', [controller '/Armed'], 'Port', '6');
add_line(controller, 'Armed/1', 'Position & Rate Controller1/6');
add_block('simulink/Signal Routing/Selector', [model '/Armed'], ...
    'InputPortWidth', '12', 'Indices', '10', 'Position', [800 600 850 640]);
add_line(model, 'External controller/8', 'Armed/1');
add_line(model, 'Armed/1', 'Original Controller Model/6');
set_param(model, 'SolverType', 'Fixed-step', 'Solver', 'ode4', ...
    'FixedStep', '0.003', 'StopTime', 'inf');
clear external_quadrotor_setpoint;
matlab.engine.shareEngine('MATLABEngine');
end
