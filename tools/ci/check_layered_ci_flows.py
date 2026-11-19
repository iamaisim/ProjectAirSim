"""Evaluate this workflow's actual job conditions locally (no Actions run).

This deliberately supports only the expression/glob subset used by our workflow;
unknown syntax fails instead of silently pretending to implement GitHub Actions.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
import re

import yaml

ROOT = Path(__file__).resolve().parents[2]
WORKFLOW_PATH = ROOT / '.github/workflows/pr_ci_orchestrator.yml'


def load_workflow(path):
    return yaml.safe_load(Path(path).read_text(encoding='utf-8'))


def triggers(workflow):
    return workflow.get('on', workflow.get(True, {}))


def path_filters(workflow):
    changes = load_workflow(ROOT / workflow['jobs']['changes']['uses'].removeprefix('./'))
    step = next(s for s in changes['jobs']['changes']['steps'] if s.get('id') == 'filter')
    return yaml.safe_load(step['with']['filters'])


def matches_pattern(path, pattern):
    # All filters currently use literal paths, *, and ** (picomatch dot=true).
    if any(c in pattern for c in '![]{}()?'):
        raise ValueError(f'Unsupported glob syntax: {pattern}')
    escaped = re.escape(pattern)
    escaped = escaped.replace(r'\*\*/', '(?:.*/)?').replace(r'\*\*', '.*').replace(r'\*', '[^/]*')
    return re.fullmatch(escaped, path.replace('\\', '/')) is not None


def expression_value(expression, context):
    expression = str(expression).strip().removeprefix('${{').removesuffix('}}').strip()
    # Tokenize first so text substituted from the context cannot become code.
    token = re.compile(r"\s+|'[^']*'|&&|\|\||==|!=|[!(),]|[A-Za-z_][A-Za-z0-9_.*-]*")
    pieces = []
    position = 0
    for match in token.finditer(expression):
        if match.start() != position:
            raise ValueError(f'Unsupported expression: {expression}')
        position = match.end()
        part = match.group()
        if part.isspace():
            continue
        if part in context:
            pieces.append(repr(context[part]))
        elif part.startswith("'") or part in ('(', ')', ',', '==', '!='):
            pieces.append(part)
        elif part in ('&&', '||', '!'):
            pieces.append({'&&': 'and', '||': 'or', '!': 'not'}[part])
        elif part in ('contains', 'cancelled', 'always', 'success', 'failure', 'fromJSON'):
            pieces.append(part)
        elif part in ('true', 'false'):
            pieces.append('True' if part == 'true' else 'False')
        else:
            raise ValueError(f'Unknown condition token: {part}')
    if position != len(expression):
        raise ValueError(f'Unsupported expression: {expression}')
    return eval(' '.join(pieces), {'__builtins__': {}}, {
        'contains': lambda collection, item: item in collection,
        'cancelled': lambda: context.get('_cancelled', False),
        'always': lambda: True,
        'success': lambda: context.get('_success', True),
        'failure': lambda: context.get('_failure', False),
        'fromJSON': json.loads,
    })


def condition_value(expression, context):
    return bool(expression_value(expression, context))


@dataclass
class Evaluation:
    workflow_runs: bool
    filters: dict
    results: dict

    @property
    def jobs(self):
        return frozenset(k for k, v in self.results.items() if v != 'skipped')


def evaluate(workflow, event, files, action='labeled', label='run-ci', labels=(),
             branch='main_public', failures=(), cancelled=False):
    event_config = triggers(workflow).get(event)
    if event_config is None or event != 'pull_request':
        return Evaluation(False, {}, {})
    if action not in event_config['types'] or branch not in event_config['branches']:
        return Evaluation(False, {}, {})
    filters = {name: any(matches_pattern(f, p) for f in files for p in patterns)
               for name, patterns in path_filters(workflow).items()}
    context = {
        'github.event_name': event,
        'github.event.action': action,
        'github.event.label.name': label,
        'github.event.pull_request.labels.*.name': list(labels),
        '_cancelled': cancelled,
    }
    results = {}
    for job_id, job in workflow['jobs'].items():
        needs = job.get('needs', [])
        if isinstance(needs, str):
            needs = [needs]
        for dependency in needs:
            if dependency not in results:
                raise ValueError(f'Job ordering/dependency unsupported: {dependency}')
            context[f'needs.{dependency}.result'] = results[dependency]
        for name, changed in filters.items():
            context[f'needs.changes.outputs.{name}'] = (
                str(changed).lower() if results.get('changes') == 'success' else ''
            )
        context['_success'] = not cancelled and all(results[n] == 'success' for n in needs)
        condition = str(job.get('if', 'success()'))
        has_status = re.search(r'\b(?:always|cancelled|failure|success)\(', condition)
        runs = (has_status or context['_success']) and condition_value(condition, context)
        result = 'skipped'
        if runs:
            result = 'failure' if job_id in failures else 'success'
            if job_id == 'python-cheap' and results['python-compatibility'] != 'success':
                result = 'failure'
        results[job_id] = result
    return Evaluation(True, filters, results)


def check_contract(workflow):
    assert set(triggers(workflow)) == {'pull_request'}, 'Unexpected trigger'
    assert triggers(workflow)['pull_request']['types'] == ['labeled']
    for caller in workflow['jobs'].values():
        callee = load_workflow(ROOT / caller['uses'].removeprefix('./'))
        contract = triggers(callee)['workflow_call'] or {}
        required = {name for name, spec in contract.get('inputs', {}).items() if spec.get('required')}
        assert required <= set(caller.get('with', {}))
    changes = load_workflow(WORKFLOW_PATH.parent / 'ci_changes.yml')
    assert set(triggers(changes)['workflow_call']['outputs']) == set(path_filters(workflow))
    for name in ('ci_changes.yml', 'ci_configs.yml', 'ci_python.yml', 'ci_python_result.yml', 'ci_windows.yml', 'ci_macos.yml', 'ci_results.yml', 'ci_linux_build.yml'):
        reusable = load_workflow(WORKFLOW_PATH.parent / name)
        assert set(triggers(reusable)) == {'workflow_call'}, name
        for job in reusable['jobs'].values():
            for label in ('other', 'run-regressions', 'run-ci'):
                actual = condition_value(job['if'], {
                    'github.event_name': 'pull_request', 'github.event.action': 'labeled',
                    'github.event.label.name': label,
                    'github.event.pull_request.labels.*.name': ['windows', 'macOS'],
                })
                assert actual == (label == 'run-ci'), (name, label)
    assert not (WORKFLOW_PATH.parent / 'test_macos_simlibs_release.yml').exists()
    macos = load_workflow(WORKFLOW_PATH.parent / 'ci_macos.yml')
    commands = '\n'.join(s.get('run', '') for s in macos['jobs']['macos-tests']['steps'])
    assert './build.sh simlibs_debug' in commands
    assert 'test_simlibs' not in commands and 'ctest' not in commands
    assert '-DBUILD_TESTING=OFF' in commands
    assert not re.search(r'(?i)release|shipping', commands)
    assert 'windows-cheap' in workflow['jobs']['macos-tests']['needs']
    # Cache every Python installation, including docs and integration.
    for current in (load_workflow(p) for p in WORKFLOW_PATH.parent.glob('*.yml')):
        for job in current['jobs'].values():
            for step in job.get('steps', []):
                if step.get('uses', '').startswith('actions/setup-python@'):
                    assert step['with']['cache'] == 'pip'
                    for path in step['with']['cache-dependency-path'].splitlines():
                        assert (ROOT / path).is_file(), path


def check_linux_steps(workflow):
    linux = load_workflow(WORKFLOW_PATH.parent / 'pr_linux_ci.yml')['jobs']['linux']
    assert linux['runs-on'] == ['self-hosted', 'Linux', 'X64']
    assert sum(s.get('uses', '').startswith('actions/checkout@') for s in linux['steps']) == 1
    for installed in ('true', 'false'):
        counts = [0, 0, 0, 0, 0]
        context = {'steps.ros-tools.outputs.installed': installed}
        toolchains = linux['strategy']['matrix']['toolchain']
        assert toolchains == ['5.8']
        for toolchain in toolchains:
            context.update({'matrix.toolchain': toolchain, 'steps.simlibs.outcome': 'skipped'})
            for step in linux['steps']:
                context.setdefault('steps.ros-tools.outputs.installed', 'false')
                if not condition_value(step.get('if', 'success()'), context):
                    continue
                if step.get('id') == 'simlibs':
                    context['steps.simlibs.outcome'] = 'success'
                    counts[0] += 1
                name = step.get('name', '')
                if name == 'Build and test standalone C++ client': counts[1] += 1
                if name == 'Build ROS 2 C++ node': counts[2] += 1
                if name == 'Build Blocks Editor for integration': counts[3] += 1
                if name == 'Run Python integration tests':
                    assert toolchain == '5.8'
                    counts[4] += 1
        assert tuple(counts) == (1, 1, 1, 1, 1), counts
    commands = '\n'.join(s.get('run', '') for s in linux['steps'])
    assert '--sim-host offline' in commands and '--sim-host unreal' in commands
    assert 'run_runtime_regressions.py' in commands
    names = [s.get('name') for s in linux['steps']]
    assert names.index('Run PAS Runtime regressions') < names.index('Build Blocks Editor for integration')
    ue = next(s for s in linux['steps'] if s.get('name') == 'Build Blocks Editor for integration')
    assert 'BlocksEditor Linux Development' in ue['run']
    assert 'ShaderCompileWorker Linux Development' in ue['run']
    assert 'package_blocks' not in commands and 'blocks_shipping' not in commands
    launch = next(s for s in linux['steps'] if s.get('name') == 'Launch Blocks in Unreal Editor')
    assert '/Engine/Binaries/Linux/UnrealEditor' in launch['run']
    assert 'Blocks.uproject' in launch['run'] and '-game' in launch['run']
    assert 'packaged.pid' not in commands and 'packaged.log' not in commands
    assert not condition_value(ue['if'], {'inputs.integration': True, 'steps.simlibs.outcome': 'failure'})
    assert {'linux-build', 'config-validation', 'python-cheap'} <= set(workflow['jobs']['linux-ci']['needs'])
    assert not {'windows-cheap', 'macos-tests'} & set(workflow['jobs']['linux-ci']['needs'])
    assert 'linux-ci' in workflow['jobs']['windows-cheap']['needs']
    assert {'linux-ci', 'windows-cheap'} <= set(workflow['jobs']['macos-tests']['needs'])
    assert 'linux-build' in workflow['jobs']['windows-cheap']['needs']
    assert all('uses' in job and 'steps' not in job for job in workflow['jobs'].values())
    independent = load_workflow(WORKFLOW_PATH.parent / 'pr_linux_ci.yml')
    assert triggers(independent) == {'workflow_call': {}}
    build = load_workflow(WORKFLOW_PATH.parent / 'ci_linux_build.yml')['jobs']['linux']
    assert build['strategy']['matrix']['toolchain'] == ['5.2']
    for job in (build, linux):
        checkout = next(s for s in job['steps'] if s.get('uses') == 'actions/checkout@v4')
        assert checkout['with']['clean'] is True
        assert not any('artifact@' in s.get('uses', '') for s in job['steps'])
        assert any(s.get('id') == 'simlibs' and './build.sh simlibs_debug' in s['run'] and 'test_simlibs_debug' in s['run'] for s in job['steps'])
    print('PASS: Linux toolchain, clean-build, label and build-once contracts')


def run_scenarios(workflow):
    baseline = {'changes', 'comment-layered-ci-results'}
    python = {'python-compatibility', 'python-cheap'}
    native = {'linux-build', 'windows-cheap', 'macos-tests'}
    full = baseline | python | native | {'config-validation', 'linux-ci'}
    allfiles = ('.github/workflows/pr_ci_orchestrator.yml',)
    count = 0

    def check(name, files, expected, **kwargs):
        nonlocal count
        kwargs.setdefault('labels', ('windows', 'macOS'))
        actual = evaluate(workflow, kwargs.pop('event', 'pull_request'), files, **kwargs)
        assert actual.jobs == frozenset(expected), f'{name}: {actual.jobs} != {expected}'
        count += 1
        print(f'PASS: {name}')

    for event in ('push', 'workflow_dispatch'):
        check(event, allfiles, set(), event=event)
    for action in ('opened', 'reopened', 'synchronize', 'unlabeled', 'ready_for_review'):
        check(action, allfiles, set(), action=action, labels=('run-ci', 'run-regressions'))
    for label in ('documentation', 'run-regressions'):
        check('other label ' + label, allfiles, set(), label=label, labels=('run-ci', 'run-regressions'))
    check('unsupported base', allfiles, set(), branch='feature')
    check('cancelled request', allfiles, set(), cancelled=True)
    check('workflow run-ci', allfiles, full - {'linux-ci'})
    check('workflow full request', allfiles, full, labels=('run-regressions', 'windows', 'macOS'))
    for name, path, jobs in (
        ('docs', 'docs/config.md', set()),
        ('README', 'README.md', set()),
        ('validator', 'tools/ci/validate_configs.py', {'config-validation'}),
        ('validator tests', 'tools/ci/test_validate_configs.py', {'config-validation'}),
        ('release utility', 'tools/release/build.py', set()),
        ('Python source', 'client/python/projectairsim/src/projectairsim/utils.py', python),
        ('Python example', 'client/python/example_user_scripts/drone.py', python),
        ('example config', 'client/python/example_user_scripts/sim_config/scene.jsonc', {'config-validation'}),
        ('native source', 'core_sim/src/a.cpp', native),
        ('Windows script', 'build_windows.mk', native),
        ('Linux script', 'build_linux.mk', native),
        ('macOS script', 'build_macos.mk', native),
        ('ROS source', 'ros/projectairsim_ros2_cpp/src/node.cpp', native),
        ('Unreal source', 'unreal/Blocks/Source/file.cpp', native),
        ('macOS workflow', '.github/workflows/ci_macos.yml', native),
        ('Linux build workflow', '.github/workflows/ci_linux_build.yml', native),
        ('Linux regression workflow', '.github/workflows/pr_linux_ci.yml', native),
        ('Python workflow', '.github/workflows/ci_python.yml', python),
        ('docs workflow', '.github/workflows/sphinx-docs.yml', set()),
    ):
        check(name, (path,), baseline | jobs)
    for failed, expected in (
        ('changes', baseline),
        ('config-validation', baseline | {'config-validation'}),
        ('python-compatibility', baseline | python | {'config-validation'}),
        ('linux-build', baseline | python | {'config-validation', 'linux-build'}),
        ('windows-cheap', full - {'macos-tests'}),
        ('macos-tests', full),
        ('linux-ci', full - {'windows-cheap', 'macos-tests'}),
    ):
        check(failed + ' failure', allfiles, expected, failures=(failed,), labels=('run-regressions', 'windows', 'macOS'))
    check('full request overrides docs-only filter', ('README.md',), baseline | native | {'linux-ci'}, labels=('run-regressions', 'windows', 'macOS'))
    check('no platform labels', allfiles, full - {'windows-cheap', 'macos-tests', 'linux-ci'}, labels=())
    check('Windows opt-in only', allfiles, full - {'macos-tests', 'linux-ci'}, labels=('windows',))
    check('macOS without Windows', allfiles, full - {'windows-cheap', 'linux-ci'}, labels=('macOS',))
    check('regression without platform labels', allfiles, full - {'windows-cheap', 'macos-tests'}, labels=('run-regressions',))
    check('old resume label does not skip stages', allfiles, full - {'linux-ci'}, labels=('ci-start-windows', 'windows', 'macOS'))
    check_contract(workflow)
    check_linux_steps(workflow)
    print(f'\n{count} scenarios and trigger/cache contracts passed.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--files', nargs='*')
    parser.add_argument('--event', default='pull_request', choices=('pull_request', 'push', 'workflow_dispatch'))
    parser.add_argument('--action', default='labeled')
    parser.add_argument('--label', default='run-ci')
    parser.add_argument('--self-hosted', action='store_true')
    args = parser.parse_args()
    workflow = load_workflow(WORKFLOW_PATH)
    if args.files is None:
        run_scenarios(workflow)
    else:
        result = evaluate(workflow, args.event, args.files, action=args.action,
                          label=args.label, labels=('run-regressions', 'windows', 'macOS') if args.self_hosted else ())
        print(result)


if __name__ == '__main__':
    main()
