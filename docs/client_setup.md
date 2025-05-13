# Project AirSim Client Setup

## Python Client

A Python client uses the following to communicate with the Project AirSim simulation server:

- Python 3.10, 64-bit
- **[pynng](https://github.com/codypiersall/pynng)** nanomsg-next-gen wrapper pip package

### Setting Up the Client on **Windows**

1. Install Python 3.10 for Windows. There are many options for installing Python, but one recommended way is to:

    - Download the official **[Windows installer for Python](https://www.python.org/downloads/windows/)**.  Please note that the 64-bit version is required.

    - Install using these **custom** options:
        - Install for all users
        - Install pip
        - Do not add Python to environment variables to prevent version conflicts (instead use virtual environments to manage Python paths and packages)
        - Precompile standard library

2. Activate the Python environment to use with Project AirSim.

    If you don't have a suitable Python environment yet, do the following.  Here, we assume we're using Python 3.10 installed in the directory `C:\Python310`, but use the version and directory of your installation of Python:

    A) Install `virtualenv` and create a new environment (here named `airsim-venv` but you may choose any convenient name):

        C:\Python310\python -m pip install virtualenv
        C:\Python310\python -m venv C:\path\to\airsim-venv

    B) Activate your environment:

        C:\path\to\airsim-venv\Scripts\activate

    C) Verify the version of Python in your environment.

    Run this command:

        python --version

    to display the version of Python.

    Run the "python" command to enter the Python console and enter the following:

        import struct
        print(struct.calcsize("P") * 8)

    A 64-bit build of Python will print "64" while a 32-bit build of Python will print "32".

    If the environment is using a wrong version or the 32-bit build of Python, you'll need to exit the environment, delete the environment, and return to step A to recreate the environment with the correct version of Python.  If the installed Python is 32-bit, you'll also need to uninstall the 32-bit build and install the 64-bit build instead.

    D) Install basic tools for setting up other pip packages in the activated environment:

        python -m pip install --upgrade pip
        python -m pip install setuptools wheel
    
    Developers must also install the cmake package:
        python -m pip install cmake

3. Install the Project AirSim Python client library:

        cd path\to\repo
        python -m pip install -e client\python\projectairsim

    If you get this error:

        Error: Could not find a version that satisfies the requirement open3d

    most likely your virtual environment is using a 32-bit build or an unsupported version of Python. In either case, delete and rebuild the virtual environment (see step 2A) using a supported 64-bit version of Python.

---

### Setting Up the Client on **Linux**

1. Install Python 3.10 to your system:

    Ubuntu 22.04 comes with Python 3.10, but Project AirSim requires additional packages:

        sudo apt install python3-dev python3-venv

    *Note: In Ubuntu 22.04, the system-installed Python (v3.10) can be launched by running `python3` instead of `python3.10`.*

2. Activate the Python environment to use with Project AirSim.

    If you don't have a suitable Python environment yet, do the following:

    A) Create a new `virtualenv` environment (here named `airsim-venv` but you may choose any convenient name):

        python3 -m venv /path/to/airsim-venv

    B) Activate your environment:

        source /path/to/airsim-venv/bin/activate

    C) Verify the version of Python in your environment.

    Run this command:

        python --version

    to display the version of Python.

    Run the "python" command to enter the Python console and enter the following:

        import struct
        print(struct.calcsize("P") * 8)

    A 64-bit build of Python will print "64" while a 32-bit build of Python will print "32".

    If the environment is using the wrong version or the 32-bit build of Python, you'll need to exit the environment, delete the environment, and return to step A) to recreate the environment with the correct version of Python.  If the installed Python is 32-bit, you'll also need to uninstall the 32-bit build and install the 64-bit build instead.

    D) Install basic tools for setting up other pip packages in the activated environment:

        python -m pip install --upgrade pip
        python -m pip install setuptools wheel

    Developers must also install the cmake package:
            python -m pip install cmake

3. Install the Project AirSim Python client library:

        cd path\to\repo
        python -m pip install -e client\python\projectairsim
    
    If you get this error:
        Error: Could not find a version that satisfies the requirement open3d

    most likely your virtual environment is using a 32-bit build or an unsupported version of Python. In either case, delete and rebuild the virtual environment (see step 2A) using a supported 64-bit version of Python.

---

### Checking Client Operation

After installation, you can check if the client works by launching Project AirSim and running one of the client demo scripts such as `hello_drone.py` with the Project AirSim Python environment activated:

```
<from activated Python environment with Project AirSim running>

python hello_drone.py
```

In your own custom client scripts, just import the client for your robot type (ex. Drone) from the Project AirSim Python client library:

``` python
from projectairsim import ProjectAirSimClient, Drone, World
```

For more details about using the client to connect, send/receive signals, etc, see the `hello_drone.py` example script contents or the specific **[API documentation](api.md)** for your simulation scenario.

---

## C++ Client

Currently, only Python clients are supported.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
