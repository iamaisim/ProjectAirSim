#
#  Add AirSim to an existing base container (ie one with a ros2 distro)
# 
ARG BASE_IMAGE=hameritt.azurecr.io/project_airsim_pipeline_ros2_humble:22.04
FROM $BASE_IMAGE

ARG AIRSIM_SOURCE_DIR=ProjectAirSim
ARG AIRSIM_ENV_ROOT=ProjectAirSim/packages/Blocks/DebugGame/Linux
ARG AIRSIMENV_DIR=Blocks
ARG AIRSIM_ENV_SCRIPT=Blocks
ARG ROS2_DISTRO=humble
ARG PX4_WORKDIR=/PX4_tools

USER root

# seems this breaks python3 ....  possibly issue with pybind11 ???
#RUN apt update && sudo apt-get -y install --no-install-recommends \
#	python-is-python3

RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash airsim_user && \
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
	adduser airsim_user sudo && \
	adduser airsim_user audio && \
	adduser airsim_user video

USER airsim_user
WORKDIR /home/airsim_user

# parameterize the version of Blocks to include (Dev, Debug, Shipping) and/or for the environment to install
#
# create symlink to the environment startup script
#
COPY --chown=airsim_user:root $AIRSIM_ENV_ROOT /home/airsim_user/ProjectAirSim/packages/$AIRSIMENV_DIR
RUN cd /home/airsim_user/ProjectAirSim/packages/$AIRSIMENV_DIR && sudo chmod +x $AIRSIM_ENV_SCRIPT.sh && sudo ln -s /home/airsim_user/ProjectAirSim/packages/$AIRSIMENV_DIR/$AIRSIM_ENV_SCRIPT.sh /home/airsim_user/ProjectAirSim/packages/start_unreal_environment.sh && sudo chmod +x /home/airsim_user/ProjectAirSim/packages/start_unreal_environment.sh

#
#  Directly install the client package ...
#
COPY --chown=airsim_user:root $AIRSIM_SOURCE_DIR/client/python/ /home/airsim_user/ProjectAirSim/client/python/
COPY --chown=airsim_user:root $AIRSIM_SOURCE_DIR/ros/ /home/airsim_user/ProjectAirSim/ros/

COPY --chown=airsim_user:root $AIRSIM_SOURCE_DIR/docker/projectairsim_ros_entry.sh /home/airsim_user/ProjectAirSim/

#
#  PX4 tooling ownership
#
RUN sudo chown -R airsim_user:root $PX4_WORKDIR
RUN sudo chmod -R 775 $PX4_WORKDIR

#
#  Startup script
#
RUN chmod +x /home/airsim_user/ProjectAirSim/projectairsim_ros_entry.sh

SHELL ["/bin/bash", "-c"]
RUN pip3 install -e  /home/airsim_user/ProjectAirSim/client/python/projectairsim
#
# An earlier pipeline step builds the code, try just installing the packages
#
RUN source /home/airsim_user/ProjectAirSim/ros/install/setup.bash && pip3 install -e /home/airsim_user/ProjectAirSim/ros/node/projectairsim-rosbridge && pip3 install -e /home/airsim_user/ProjectAirSim/ros/node/projectairsim-ros2
RUN echo "source /home/airsim_user/ProjectAirSim/ros/install/setup.bash" >> ~/.bashrc

# For storing test output
RUN mkdir -p /home/airsim_user/test_output

# Expose port 41451 so Airsim client can connect to server
EXPOSE 41451

ENTRYPOINT ["/home/airsim_user/ProjectAirSim/projectairsim_ros_entry.sh"]
