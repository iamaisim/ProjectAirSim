# Setting-up ProjectAirSim Autonomy Blocks

1. Setup the runtime environment and install dependencies

    1. Setup the `ProjectAirSim` client by following the steps listed in [client_setup](../client_setup.md) based on your platform (Linux/Windows)
    1. Activate the python environment where you have installed the `projectairsim` python client package and install the additional `autonomy` module dependencies using the following command where `VERSION` is the Project AirSim version ID such as `0.1.8` (e.g. `projectairsim-0.1.8-py3-none-any.whl`) depending on the wheel package you have.

        > Note: The extra **[autonomy]** (including the square braces) after the wheel file name is necessary.

        `python -m pip install projectairsim-{VERSION}-py3-none-any.whl[autonomy]`


    > Note: In order to run the pre-trained autonomy models efficiently,  A CUDA-capable GPU device with the compatible driver is expected to be installed on your system. The above setup would install the necessary `cudatoolkit` and deep learning framework (PyTorch) for Linux. If you are using **Windows** or **Mac OS**, in order to utilize the GPU device using `cudatoolkit`, please install using the command provided at [https://pytorch.org](https://pytorch.org). As an example, for installing the PyTorch stable release version `1.10.1` on Windows using Pip and CUDA 11.3, run the following command:
    `python -m pip install torch==1.10.1+cu113 torchvision==0.11.2+cu113 -f https://download.pytorch.org/whl/cu113/torch_stable.html`

1. Check if your setup went successful

    1. Run the followingImport a perception module from Project AirSim Autonomy to ensure your `autonomy` module setup went through successfully:

    ```bash
    python -c  "from projectairsim.autonomy.perception import PosePredictor"
    ```

    If the above line executes without error on your `ProjectAirSim` python virtual environment, you are all set!

    While not absolutely necessary, it is recommended to have a GPU on your machine to run the perception models. You can check if you have a GPU device that the model can use using the following command returns True:

    ```bash
    python -c "import torch; print(torch.cuda.is_available())"
    ```

1. Obtain the pre-trained models using the separate instructions provided to you  or reach out to the ProjectAirSim Autonomy team for instructions.