% Copyright (C) 2025 IAMAI CONSULTING CORP
%
% MIT License.

function desired = external_quadrotor_setpoint(input)
% Convert generic setpoints to the stock example's position/yaw reference.
kinematics = input(1:19);
command = input(20:31);
persistent target previous_time
time = command(12);
q = kinematics(4:7);
yaw = atan2(2*(q(4)*q(3)+q(1)*q(2)), 1-2*(q(2)^2+q(3)^2));
if isempty(target) || time < previous_time || command(9) == 0 || command(10) == 0
    target = [kinematics(1:3); yaw];
    previous_time = time;
end
dt = max(0, min(0.1, time-previous_time));
previous_time = time;
mode = command(1);
velocity = command(3:5);
if command(2) == 1
    velocity(1:2) = [cos(yaw), -sin(yaw); sin(yaw), cos(yaw)]*velocity(1:2);
end
if command(9) ~= 0 && command(10) ~= 0
    switch mode
        case 1
            target(1:3) = target(1:3) + dt*velocity;
        case 2
            target(1:2) = target(1:2) + dt*velocity(1:2);
            target(3) = command(5);
        case 3
            target(1:3) = command(3:5);
        case 4
            heading = command(3);
            target(1:3) = target(1:3) + dt*[cos(heading)*command(4); ...
                sin(heading)*command(4); command(5)];
    end
    if mode ~= 0
        if command(6) ~= 0
            target(4) = target(4) + dt*command(7);
        else
            target(4) = command(7);
        end
    end
end
% Demonstration simplification: the example always reports ready and infers
% landed from the absolute NED altitude, assuming the ground is at z = 0. A
% real controller reports ready only after its own initialization and health
% checks pass, and reports landed from ground contact or its landing logic.
landed = abs(kinematics(3)) < 0.05 && norm(kinematics(8:10)) < 0.05;
desired = [target; 1; landed];
end
