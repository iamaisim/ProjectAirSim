"""Exercise host-variable and interactive Bash recovery without an Unreal build."""
import os
from pathlib import Path
import subprocess
import tempfile

HELPER = Path(__file__).with_name('select_unreal_root.sh').resolve()


def main():
    with tempfile.TemporaryDirectory(prefix='pas-root-check-') as temporary:
        root = Path(temporary)
        engine = root / 'Unreal Engine'
        build = engine / 'Engine/Build/BatchFiles/Linux/Build.sh'
        build.parent.mkdir(parents=True)
        build.write_text('#!/bin/sh\nexit 0\n')
        build.chmod(0o755)
        home = root / 'home'
        home.mkdir()
        output = root / 'github-env'
        env = dict(os.environ, HOME=str(home), GITHUB_ENV=str(output))
        for key in ('UE_ROOT_5_2', 'UE_ROOT_5_7', 'BASH_ENV', 'ENV'):
            env.pop(key, None)
        for version, variable in (('5.2', 'UE_ROOT_5_2'), ('5.7', 'UE_ROOT_5_7')):
            for inherited in (False, True):
                # The guard would skip exports under ordinary noninteractive source.
                rc = 'case $- in *i*) ;; *) return ;; esac\necho startup-noise\n'
                rc += f'export {variable}="{engine if not inherited else root / "wrong"}"\n'
                (home / '.bashrc').write_text(rc)
                current = dict(env)
                if inherited:
                    current[variable] = str(engine)
                output.write_text('')
                result = subprocess.run(['bash', str(HELPER), version], env=current,
                                        capture_output=True, text=True)
                assert result.returncode == 0, result.stderr
                assert output.read_text() == f'{variable}={engine}\nUE_ROOT={engine}\n'
        for rc in ('', f'export UE_ROOT_5_2="{root / "missing"}"\n'):
            (home / '.bashrc').write_text(rc)
            output.write_text('')
            result = subprocess.run(['bash', str(HELPER), '5.2'], env=env,
                                    capture_output=True, text=True)
            assert result.returncode != 0
            assert output.read_text() == ''
    print('PASS: both UE versions, host precedence, guarded/noisy bashrc, spaces, missing/invalid roots')


if __name__ == '__main__':
    main()
