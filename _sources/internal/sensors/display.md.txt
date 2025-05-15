# Sensor data visualization/display

## Choosing the display backend

Matplotlib is used to plot and display the sensor data. Image-specific display utility functions are implemented in [client/python/projectairsim/src/projectairsim/image_utils.py](../../../client/python/projectairsim/src/projectairsim/image_utils.py).
The backend is strictly enforced to be `TkAgg` for consistency and reliability. Other backends may work but GTK has been reported to cause issues.

The backend used for rendering the plot can be configured by changing the MPL backend value offered by Matplotlib.
The list of supported backends can be found here: https://matplotlib.org/stable/tutorials/introductory/usage.html#the-builtin-backends

Relevant details about how to choose the backend with Matplotlib is reproduced below:

There are three ways to configure your backend:

 1. The `backend` parameter in your `matplotlibrc` file
 2. The `MPLBACKEND` environment variable
 3. The function `matplotlib.use`

 Below is a more detailed description.

 If there is more than one configuration present, the last one from the
 list takes precedence; e.g. calling `matplotlib.use()` will override
 the setting in your `matplotlibrc`.

 Without a backend explicitly set, Matplotlib automatically detects a usable
 backend based on what is available on your system and on whether a GUI event
 loop is already running. On Linux, if the environment variable
 `DISPLAY` is unset, the "event loop" is identified as "headless",
 which causes a fallback to a noninteractive backend (agg); in all other
 cases, an interactive backend is preferred (usually, at least tkagg will be
 available).

 Here is a detailed description of the configuration methods:

### 1. Setting `backend` in your `matplotlibrc` file::

```bash
backend : qt5agg   # use pyqt5 with antigrain (agg) rendering.
```
### 2. Setting the `MPLBACKEND` environment variable:

You can set the environment variable either for your current shell or for
a single script.

On Unix:

```python
> export MPLBACKEND=qt5agg
> python simple_plot.py

> MPLBACKEND=qt5agg python sensor_data_plot.py
```

On Windows, only the former is possible:

```python
> set MPLBACKEND=qt5agg
> python sensor_data_plot.py
```

Setting this environment variable will override the ``backend`` parameter
in *any* `matplotlibrc`, even if there is a `matplotlibrc` in
your current working directory. Therefore, setting `MPLBACKEND`
globally, e.g. in your `.bashrc` or `.profile`, is discouraged
as it might lead to counter-intuitive behavior.

### 3. If your script depends on a specific backend you can use the function
 `matplotlib.use`

```python
import matplotlib
matplotlib.use('qt5agg')
```

This should be done before any figure is created, otherwise Matplotlib may
fail to switch the backend and raise an ImportError.

Using `~matplotlib.use` will require changes in your code if users want to
use a different backend.  Therefore, you should avoid explicitly calling
`~matplotlib.use` unless absolutely necessary.

---

Copyright (C) Microsoft Corporation.  All rights reserved.
