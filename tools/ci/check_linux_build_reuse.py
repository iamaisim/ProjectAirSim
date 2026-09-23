#!/usr/bin/env python3
"""Check real GNU Make dependencies without compiling or allocating CI runners."""
from pathlib import Path
import shlex
import shutil
import subprocess
import tempfile

import yaml

ROOT = Path(__file__).resolve().parents[2]


def build_arguments(step):
    commands = [line.strip() for line in step['run'].replace('\\\n', ' ').splitlines()
                if line.strip().startswith('./build.sh ')]
    assert len(commands) == 1, 'Each phase must use exactly one build invocation'
    return shlex.split(commands[0])[1:]


def main():
    assert shutil.which('make'), 'GNU Make is required; run this check on Linux/WSL'
    workflow = yaml.safe_load((ROOT / '.github/workflows/pr_linux_ci.yml').read_text())
    steps = workflow['jobs']['linux']['steps']
    initial = yaml.safe_load((ROOT / '.github/workflows/ci_linux_build.yml').read_text())['jobs']['linux']['steps']
    build = next(s for s in initial if s.get('id') == 'simlibs')
    regression = next(s for s in steps if s.get('id') == 'simlibs')
    unreal = next(s for s in steps if s.get('name') == 'Build Blocks Editor for integration')
    for step in initial + steps:
        if step.get('shell') == 'bash':
            subprocess.run(['bash', '-n'], input=step['run'], text=True, check=True)
    print('PASS: bash syntax for every Linux shell step')
    with tempfile.TemporaryDirectory(prefix='pas-ci-build-graph-') as temporary:
        engine = Path(temporary) / 'UE-5.2'
        engine.mkdir()
        base = ['make', '--dry-run', '--no-print-directory', '-f', str(ROOT / 'build_linux.mk'),
                f'UE_ROOT={engine}', 'CMAKE_DBG_BUILD_CMD=echo CI_SIMLIBS_DEBUG',
                'CMAKE_REL_BUILD_CMD=echo CI_SIMLIBS_RELEASE']
        for step, expected in ((build, (1, 0)), (regression, (1, 1))):
            result = subprocess.run(base + build_arguments(step), cwd=temporary,
                                    check=True, text=True, capture_output=True)
            lines = result.stdout.splitlines()
            measured = (lines.count('echo CI_SIMLIBS_DEBUG'), lines.count('echo CI_SIMLIBS_RELEASE'))
            assert measured == expected, (step['name'], measured, expected)
            assert '--target simlibs_unit_tests' in result.stdout
            assert 'ctest' in result.stdout
            print(f"PASS: {step['name']}: Debug builds={measured[0]}, Release builds={measured[1]}")
    # Direct UBT invocations consume the Release libraries already built by
    # the regression step; they do not invoke Make or rebuild SimLibs.
    assert './build.sh' not in unreal['run'] and 'cmake' not in unreal['run']
    assert 'BlocksEditor Linux Development' in unreal['run']
    assert 'ShaderCompileWorker Linux Development' in unreal['run']
    assert '--assume-old' not in unreal['run']
    print('PASS: Blocks Editor reuses existing Release SimLibs')


if __name__ == '__main__':
    main()
