function testTransportLifecycle(mexDirectory)
% Exercise the real Simulink callbacks for both rebuilt stock S-functions.
% Build both modules first, then pass the directory containing their MEX files.
% Native wrapper termination tests do not exercise Simulink's scheduler.
addpath(mexDirectory);
for kind = {'physics', 'control'}
    model = ['test_matlab_' kind{1} '_transport'];
    if bdIsLoaded(model), close_system(model, 0); end
    new_system(model);
    cleanup = onCleanup(@() close_system(model, 0));
    set_param(model, 'SolverType', 'Fixed-step', 'Solver', ...
        'FixedStepDiscrete', 'FixedStep', '0.01', 'StopTime', '1');
    block = [model '/Transport'];
    base = '''127.0.0.1'',''0'',0.01';
    add_block('simulink/User-Defined Functions/S-Function', block, ...
        'Parameters', base, ...
        'FunctionName', ['matlab_' kind{1} '_model_sfcn']);
    if strcmp(kind{1}, 'physics'), width = 19; ports = 4;
    else, width = 16; ports = 7; end
    add_block('simulink/Sources/Constant', [model '/Input'], ...
        'Value', sprintf('zeros(1,%d)', width));
    add_line(model, 'Input/1', 'Transport/1');
    for p = 1:ports
        name = ['Output' num2str(p)];
        add_block('simulink/Sinks/Terminator', [model '/' name]);
        add_line(model, ['Transport/' num2str(p)], [name '/1']);
    end
    % Port 0 asks NNG to choose a free port; there is deliberately no peer.
    for suffix = {'', ',0.05'}
        set_param(block, 'Parameters', [base suffix{1}]);
        set_param(model, 'SimulationCommand', 'update');
    end
    for parameters = {'''127.0.0.1'',''0''', [base ',NaN'], [base ',[1 2]'], ...
            [base ',0'], [base ',-1'], [base ',Inf']}
        failed = false;
        try
            set_param(block, 'Parameters', parameters{1});
            set_param(model, 'SimulationCommand', 'update');
        catch
            failed = true;
        end
        assert(failed, 'Invalid block parameters were accepted.');
    end
    set_param(block, 'Parameters', [base ',0.05']);
    % A second run verifies cleanup allows the same block to start again.
    for run = 1:2
        failed = false;
        try
            sim(model);
        catch e
            failed = true;
            assert(contains(getReport(e), 'Timed out'), ...
                'Expected a transport timeout, got: %s', getReport(e));
        end
        assert(failed, 'Transport timeout did not become a model error.');
        assert(strcmp(get_param(model, 'SimulationStatus'), 'stopped'));
    end
    fprintf('PASS: %s parameter validation, timeout, and restart\n', kind{1});
    clear cleanup;
end
end
