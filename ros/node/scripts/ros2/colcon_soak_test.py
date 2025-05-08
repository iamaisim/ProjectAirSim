import os
import sys
import time
import logging
from datetime import datetime

logging_directory = "~/.ros/log/" + datetime.today().strftime('%Y-%m-%d-%H:%M:%S') + "-" + str(os.getpid())
os.environ['ROS_LOG_DIR'] = logging_directory

logging.basicConfig(level=logging.INFO, format='%(asctime)s %(message)s', datefmt='%Y-%m-%d-%I-%M-%S-%p')
logger = logging.getLogger(__name__)

for iteration in range( 0, 1500 ):
    logger.info( f"Test Iteration {iteration} started." )
    os.system( "colcon test --python-testing pytest --packages-select projectairsim_ros")
    os.system( "colcon test-result --all --verbose" )
    logger.info( f"Test Iteration {iteration} ended." )
    time.sleep( 1 )