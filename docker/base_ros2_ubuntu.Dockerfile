ARG BASE_IMAGE=adamrehn/ue4-runtime:22.04-cudagl11-x11
FROM $BASE_IMAGE

ARG ROS2_DISTRO=humble
ARG PX4_WORKDIR=/PX4_tools

USER root

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y --no-install-recommends \
	apt-transport-https \
	binutils-dev \
	ca-certificates \
	ccache \
	cmake \
	cppcheck \
	curl \
	dirmngr \
	doxygen \
	g++-multilib \
	gcc-multilib \
	gdb \
	gettext \
	git \
	gnupg \
	gosu \
	python3-grpcio \
	lcov \
	libelf-dev \
	libexpat-dev \
	libfreetype6-dev \
	libglu1-mesa-dev \
	libgmp-dev \
	libgtest-dev \
	libisl-dev \
	libmpc-dev \
	libmpfr-dev \
	libpng-dev \
	libssl-dev \
	libvecmath-java \
	libxcb-keysyms1-dev \
	lsb-release \
	make \
	ninja-build \
	openssh-client \
	pulseaudio \
	python3 \
	python3-dev \
	python3-pip \
	python3-venv \
	rsync \
	screen \
	shellcheck \
	software-properties-common \
	sudo \
	texinfo \
	tzdata \
	unzip \
	util-linux \
	vim \
	wget \
	x11-xserver-utils \
	xdg-user-dirs \
	xsltproc \
	zip

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null

RUN echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ jammy main' | sudo tee /etc/apt/sources.list.d/kitware.list >/dev/null

RUN apt update && sudo apt-get -y install --no-install-recommends \
	build-essential \
	cmake \
	clang-15 \
	libc++-15-dev \
	libc++abi-15-dev \
	ninja-build \
	libvulkan1 \
	mesa-vulkan-drivers \
	vulkan-tools

RUN python3 -m pip install --upgrade pip && \
    pip3 install setuptools wheel && \
	pip3 install numpy\<2 && \
	pip3 install msgpack-rpc-python && \
	pip3 install transforms3d && \
	pip3 install numpy-quaternion && \
	pip3 install sympy && \
	pip3 install empy && \
	pip3 install -U \
		argcomplete \
		flake8 \
		flake8-blind-except \
		flake8-builtins \
		flake8-class-newline \
		flake8-comprehensions \
		flake8-deprecated \
		flake8-docstrings \
		flake8-import-order \
		flake8-quotes \
		pytest-repeat \
		pytest-rerunfailures

RUN apt update && apt install locales && \
    locale-gen en_US en_US.UTF-8 && \
    update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
    export LANG=en_US.UTF-8 && \
    add-apt-repository universe

RUN apt update && apt install -y curl && \
    curl -sSl https://raw.githubusercontent.com/ros/rosdistro/master/ros.key -o /usr/share/keyrings/ros-archive-keyring.gpg && \
	echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | tee /etc/apt/sources.list.d/ros2.list > /dev/null

RUN apt update && \
    apt upgrade -y && \
    apt install -y ros-$ROS2_DISTRO-desktop ros-dev-tools

RUN apt-get update && apt install -y ros-$ROS2_DISTRO-tf2-sensor-msgs \
	ros-$ROS2_DISTRO-launch-testing-ament-cmake \
	ros-$ROS2_DISTRO-ros2bag \
	ros-$ROS2_DISTRO-rosidl-generator-dds-idl \
	ros-$ROS2_DISTRO-tf2-geometry-msgs \
	ros-$ROS2_DISTRO-radar-msgs \
	ros-$ROS2_DISTRO-mavros* \
	libyaml-cpp-dev \
	libunwind-dev \
	ros-$ROS2_DISTRO-navigation2 \
	ros-$ROS2_DISTRO-depth-image-proc \
	ros-$ROS2_DISTRO-image-proc \
	ros-$ROS2_DISTRO-launch-pytest \
	ros-$ROS2_DISTRO-cv-bridge
	
RUN apt install -y python3-colcon-common-extensions \
		python3-colcon-mixin \
		python3-rosdep \
		python3-vcstool

#
#  For PX4, see https://docs.px4.io/main/en/ros2/user_guide.html
#  ROS2 nodes for PX4 built and installed by build pipeline
#
RUN pip3 install pyros-genmsg

# bootstrap rosdep
RUN rosdep init && rosdep update

# setup colcon mixin and metadata
RUN colcon mixin add default \
		https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml \
	&& colcon mixin update \
	&& colcon metadata add default \
		https://raw.githubusercontent.com/colcon/colcon-metadata-repository/master/index.yaml \
	&& colcon metadata update

RUN mkdir -p $PX4_WORKDIR
WORKDIR $PX4_WORKDIR

#
# Setup XRCE-DDS Agent & Client
#
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS-Agent.git && \
    cd Micro-XRCE-DDS-Agent && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    sudo make install && \
    sudo ldconfig /usr/local/lib/ && \
    cd ../..

#
# PX4 sitl
#
# make px4_sitl_default-clang && \ cannot find clang-15
#
RUN git clone https://github.com/PX4/PX4-Autopilot.git --recursive && \
      	cd PX4-Autopilot && \
	pip3 install -r Tools/setup/requirements.txt && \
      	make px4_sitl && \
      	cd ..

#
# MavSDK and Python package
#
RUN wget https://github.com/mavlink/MAVSDK/releases/download/v2.14.0/libmavsdk-dev_2.14.0_ubuntu22.04_amd64.deb && \
	sudo dpkg -i libmavsdk-dev_2.14.0_ubuntu22.04_amd64.deb && \
	rm libmavsdk-dev_2.14.0_ubuntu22.04_amd64.deb && \
	pip3 install mavsdk

#
#  mavsdk examples (not in pypi package)
#
RUN git clone https://github.com/mavlink/MAVSDK-Python.git

# SITL UDP PORTS
# MAVLINK
EXPOSE 14540/udp

# QGroundControl
EXPOSE 14550/udp

EXPOSE 14556/udp
EXPOSE 14557/udp

# Simulator
EXPOSE 4560/tcp		

CMD ["bash"]
