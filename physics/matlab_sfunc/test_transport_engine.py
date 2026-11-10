"""Run real Simulink lifecycle checks (MATLAB Engine, pynng, msgpack required).

Usage: python test_transport_engine.py /absolute/path/to/rebuilt/mex
The models are temporary; no stock model is modified.
"""
import argparse
import time

import matlab.engine
import msgpack
import pynng


def make_model(engine, kind, port):
    model = 'test_transport_' + kind
    width, outputs = (19, 4) if kind == 'physics' else (16, 7)
    engine.eval(f"""
        m='{model}'; new_system(m);
        set_param(m,'SolverType','Fixed-step','Solver','FixedStepDiscrete', ...
            'FixedStep','0.01','StopTime','0.02');
        add_block('simulink/User-Defined Functions/S-Function',[m '/Transport'], ...
            'Parameters','''127.0.0.1'',''{port}'',0.01,5', ...
            'FunctionName','matlab_{kind}_model_sfcn');
        add_block('simulink/Sources/Constant',[m '/Input'],'Value','zeros(1,{width})');
        add_line(m,'Input/1','Transport/1');
        for p=1:{outputs}
            name=['Output' num2str(p)];
            add_block('simulink/Sinks/Terminator',[m '/' name]);
            add_line(m,['Transport/' num2str(p)],[name '/1']);
        end
    """, nargout=0)
    return model


def fast_restart(engine, model, address):
    engine.set_param(model, 'FastRestart', 'on', nargout=0)
    for run in range(3):
        engine.eval("lastwarn('')", nargout=0)
        simulation = engine.sim(model, nargout=0, background=True)
        replies = 0
        with pynng.Req0(send_timeout=500, recv_timeout=500) as peer:
            peer.dial(address, block=False)
            deadline = time.monotonic() + 30
            while not simulation.done():
                assert time.monotonic() < deadline, 'Simulation did not finish'
                try:
                    peer.send(msgpack.packb({'time_stamp': replies}))
                    response = msgpack.unpackb(peer.recv(), raw=False)
                    assert isinstance(response, dict) and response
                    replies += 1
                except pynng.Timeout:
                    continue
        simulation.result(timeout=10)
        assert replies > 0, 'No real request/reply exchange'
        assert engine.get_param(model, 'FastRestart') == 'on'
        assert engine.get_param(model, 'SimulationStatus') == 'compiled'
        warning = engine.eval('lastwarn')
        assert not warning, warning
        print(f'PASS: {model} Fast Restart run {run + 1}, {replies} replies', flush=True)
    engine.set_param(model, 'FastRestart', 'off', nargout=0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('mex_directory')
    parser.add_argument('--port', type=int, default=19199,
                        help='First of two free local TCP ports for the tests')
    args = parser.parse_args()
    engine = matlab.engine.start_matlab('-nodesktop -nosplash')
    try:
        engine.addpath(args.mex_directory, nargout=0)
        for index, kind in enumerate(('physics', 'control')):
            port = args.port + index
            model = make_model(engine, kind, port)
            try:
                fast_restart(engine, model, f'tcp://127.0.0.1:{port}')
            finally:
                engine.close_system(model, 0, nargout=0)
    finally:
        engine.quit()


if __name__ == '__main__':
    main()
