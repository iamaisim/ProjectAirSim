FROM adamrehn/ue4-runtime:22.04-cudagl11-x11

USER root

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y --no-install-recommends \
	wget \
	vim \
	unzip \
	python3 \
	python3-pip \
	python3-dev \
	sudo \
	libglu1-mesa-dev \
	xdg-user-dirs \
	pulseaudio \
	x11-xserver-utils \
	git \
	libvulkan1 \
	mesa-vulkan-drivers \
	libxcb-keysyms1-dev

RUN python3 -m pip install --upgrade pip && \
    pip3 install setuptools wheel && \
	pip3 install cmake

RUN apt update && apt install locales && \
    locale-gen en_US en_US.UTF-8 && \
    update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
    export LANG=en_US.UTF-8 && \
    apt install -y software-properties-common && \
    add-apt-repository universe

RUN apt update && apt install -y curl && \
    curl -sSl https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg && \
	echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null

RUN apt update && \
    apt upgrade -y && \
    apt install -y ros-humble-ros-base ros-dev-tools

RUN apt-get update && apt install -y ros-humble-tf2-sensor-msgs \
	ros-humble-tf2-geometry-msgs \
	ros-humble-radar-msgs \
	ros-humble-mavros* \
	libyaml-cpp-dev \
	libunwind-dev \
	ros-humble-navigation2 \
	ros-humble-depth-image-proc \
	ros-humble-image-proc
	
RUN curl -s https://packagecloud.io/install/repositories/dirk-thomas/colcon/script.deb.sh | bash &&\
	apt install python3-colcon-common-extensions 


RUN adduser --force-badname --disabled-password --gecos '' --shell /bin/bash airsim_user && \
	echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers && \
	adduser airsim_user sudo && \
	adduser airsim_user audio && \
	adduser airsim_user video

USER airsim_user
WORKDIR /home/airsim_user

COPY /packages/Blocks/ /home/airsim_user/ProjectAirSim/packages/Blocks/
COPY /client/python/ /home/airsim_user/ProjectAirSim/client/python/
COPY /ros/node/ /home/airsim_user/ProjectAirSim/ros/node/
COPY /docker/entrypoint.sh /home/airsim_user/ProjectAirSim/entrypoint.sh

RUN sudo chown -R airsim_user /home/airsim_user
RUN chmod +x /home/airsim_user/ProjectAirSim/entrypoint.sh


SHELL ["/bin/bash", "-c"]
RUN pip3 install -e  /home/airsim_user/ProjectAirSim/client/python/projectairsim
RUN source /opt/ros/humble/setup.bash && cd /home/airsim_user/ProjectAirSim/ros/node && colcon build
RUN source /opt/ros/humble/setup.bash && source /home/airsim_user/ProjectAirSim/ros/node/install/setup.bash && pip3 install -e /home/airsim_user/ProjectAirSim/ros/node/projectairsim-rosbridge && pip3 install -e /home/airsim_user/ProjectAirSim/ros/node/projectairsim-ros2
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# Expose port 41451 so Airsim client can connect to server
EXPOSE 41451
#EXPOSE 8888
#EXPOSE 8989
#EXPOSE 8990

ENTRYPOINT ["/home/airsim_user/ProjectAirSim/entrypoint.sh"]
