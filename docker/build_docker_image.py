import argparse
import subprocess
import os

def main():
    parser = argparse.ArgumentParser(description='AirSim docker image builder')
    parser.add_argument('--target_image', type=str, help='base image name AND tag')
    parser.add_argument('--dockerfile', type=str, help='dockerfile to use for build', default='binary.Dockerfile' )
    parser.add_argument('--env_binary_dir', type=str, help='dockerfile to use for build', default='binary.Dockerfile' ) 
    
    args = parser.parse_args()
    build_docker_image(args)

def build_docker_image(args):

    dockerfile = os.path.join(os.path.abspath(os.path.dirname(__file__)), args.dockerfile)
    
    # Go to parent directory(projectairsim root)
    os.chdir( '..' )
    
    docker_command = ['docker', 'build', '--network=host', \
                        '-t', args.target_image, \
                        '-f',  dockerfile, \
                        '--build-arg', 'ENV_BINARY_DIR=' + args.env_binary_dir, \
                        '.']
    print(" ".join(docker_command))
        
    subprocess.call(docker_command)

if __name__=="__main__":
    main()
