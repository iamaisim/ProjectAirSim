#include <ros/ros.h>
#include <actionlib/server/action_server.h>
#include <pthread.h>
#include <mutex>

#include <trajectory_msgs/MultiDOFJointTrajectory.h>
#include <action_controller/MultiDofFollowJointTrajectoryAction.h>
#include <trajectory_msgs/JointTrajectory.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <tf/transform_broadcaster.h>
#include <tf/transform_datatypes.h>


class Controller{
private:
	typedef actionlib::ActionServer<action_controller::MultiDofFollowJointTrajectoryAction> ActionServer;
	typedef ActionServer::GoalHandle GoalHandle;
public:
	Controller(ros::NodeHandle &n) :
		node_(n),
		action_server_(node_, "multi_dof_joint_trajectory_action/multi_dof_follow_joint_trajectory",
				boost::bind(&Controller::goalCB, this, _1),
				boost::bind(&Controller::cancelCB, this, _1),
				false),
				has_active_goal_(false)
{
		created=0;
		empty.linear.x=0;
		empty.linear.y=0;
		empty.linear.z=0;
		empty.angular.z=0;
		empty.angular.y=0;
		empty.angular.x=0;
		pub_topic = node_.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
		sub_topic = node_.subscribe<geometry_msgs::PoseStamped>("/actual_pose", 1, boost::bind(&Controller::OnPoseStampedMessage, this, _1));

		action_server_.start();
		ROS_INFO("*****actionController: ready");
}
private:
	ros::NodeHandle node_;
	ActionServer action_server_;
	ros::Publisher pub_topic;
	ros::Subscriber sub_topic;
	geometry_msgs::Pose poseCur_;
	std::mutex mutexPose_;

  	tf::Quaternion q;
  	double des_roll, des_pitch, des_yaw;

	geometry_msgs::Twist empty;
	geometry_msgs::Transform_<std::allocator<void> > lastPosition;
	ros::Duration lastTime;
	ros::Duration currentTime;
	double dt;
	geometry_msgs::Twist cmd;
	pthread_t trajectoryExecutor;
	int created;

	bool has_active_goal_;
	GoalHandle active_goal_;
	trajectory_msgs::MultiDOFJointTrajectory_<std::allocator<void> > toExecute;

  	double current_time; 
  	double start_time; 

	void cancelCB(GoalHandle gh){
		if (active_goal_ == gh)
		{
			// Stops the controller.
			if(created){
				printf("Stop thread \n");
				pthread_cancel(trajectoryExecutor);
				created=0;
			}
			pub_topic.publish(empty);

			// Marks the current goal as canceled.
			active_goal_.setCanceled();
			has_active_goal_ = false;
		}
	}

	void goalCB(GoalHandle gh){
		ROS_INFO("*****actionController: Got goal");
		if (has_active_goal_)
		{
			// Stops the controller.
			if(created){
				pthread_cancel(trajectoryExecutor);
				created=0;
			}
			pub_topic.publish(empty);

			// Marks the current goal as canceled.
			active_goal_.setCanceled();
			has_active_goal_ = false;
		}

		gh.setAccepted();
		active_goal_ = gh;
		has_active_goal_ = true;
		toExecute = gh.getGoal()->trajectory;

		//controllore solo per il giunto virtuale Base
		if(pthread_create(&trajectoryExecutor, NULL, threadWrapper, this)==0){
			created=1;
			ROS_INFO("*****actionController: Trajectory execution thread created");
		} else {
			ROS_INFO("*****actionController: Trajectory execution thread creation failed");
		}

	}

	void OnPoseStampedMessage(const geometry_msgs::PoseStampedConstPtr &pposestamped)
	{
		std::lock_guard<std::mutex> lockguard(mutexPose_);


		poseCur_ = pposestamped->pose;
	}


	static void* threadWrapper(void* arg) {
		Controller * mySelf=(Controller*)arg;
		mySelf->executeTrajectory();
		return NULL;
	}

	void executeTrajectory(){
		if (!toExecute.joint_names.empty()
				&& (toExecute.joint_names[0] == "virtual_joint")
				&& (toExecute.points.size() > 0)){
			auto timeStart = ros::Time::now();


			for(int k=0; k<toExecute.points.size(); k++){
				auto transforms = toExecute.points[k].transforms;

				if (transforms.empty())
					continue;

				auto punto=transforms[0];
				auto timePoint = timeStart + toExecute.points[k].time_from_start;
				bool eseguito=true;
				if (k == 0) {
					publishRotationComand(punto,true);
				} else {
					dt = timePoint.toSec() - ros::Time::now().toSec();
					if (dt < 0.1)
						dt = 0.1;
					eseguito=publishTranslationComand(punto,false,dt);
					if(k==(toExecute.points.size()-1)){
						if(!eseguito) publishTranslationComand(punto,true,dt);
						publishRotationComand(punto,false);
					}
				}

				//aggiorno start position
				if(eseguito){
					lastPosition.translation=punto.translation;
					lastPosition.rotation=punto.rotation;
					lastTime = toExecute.points[k].time_from_start;
				}
			}

			pub_topic.publish(empty);
		}
		active_goal_.setSucceeded();
		has_active_goal_=false;
		created=0;

	}
	bool publishTranslationComand(geometry_msgs::Transform_<std::allocator<void> > punto, bool anyway, double dt){
		static const float c_deltaMax = 2.0;

		geometry_msgs::Point positionCur;


		//Get the robot's current pose
		{
		std::lock_guard<std::mutex> lockguard(mutexPose_);


		positionCur = poseCur_.position;
		}

		//creazione comando di traslazione
		cmd.linear.x=(punto.translation.x-positionCur.x)/dt;
		cmd.linear.y=(punto.translation.y-positionCur.y)/dt;
		cmd.linear.z=(punto.translation.z-positionCur.z)/dt;
		cmd.angular.x=cmd.angular.y=cmd.angular.z=0;

		if(anyway || cmd.linear.x>=0.05 || cmd.linear.y>=0.05 || cmd.linear.z>=0.05){
			printPositionInfo();
			printCmdInfo();
			pub_topic.publish(cmd);
			//tempo d'esecuzione
			//ros::Duration(1.0).sleep();
			ros::Duration(dt).sleep();

			//Wait until we're near the destination
			{
			auto timestamp_timeout = ros::Time::now() + ros::Duration(5.0);


			while (ros::Time::now() < timestamp_timeout) {
				//Get the robot's current pose
				{
				std::lock_guard<std::mutex> lockguard(mutexPose_);


				positionCur = poseCur_.position;
				}

				//Is the robot close enough to the target?
				if ((fabs(positionCur.x - punto.translation.x) <= c_deltaMax)
						&& (fabs(positionCur.y - punto.translation.y) <= c_deltaMax)
						&& (fabs(positionCur.z - punto.translation.z) <= c_deltaMax)) {
					break; //Yup, we're there!
				}

				//Nope, check for timeout
				if (ros::Time::now() > timestamp_timeout) {
					ROS_ERROR("*****actionController: Timeout moving robot to target position");
					return (false);
				}

				pub_topic.publish(cmd);

				//Sleep a bit before checking again
				ros::Duration(0.25).sleep();
			}
			}

			return true;
		}
		return false;
	}

	void publishRotationComand(geometry_msgs::Transform_<std::allocator<void> > punto, bool start){
		//comando di allineamento, permesse solo rotazioni sull'asse z
		cmd.linear.x=cmd.linear.y=cmd.linear.z=cmd.angular.x=cmd.angular.y=0;
		//start = true --> devo tornare nell'orientazione 0
		//start = false --> devo arrivare al'orientazione punto.rotation.z
		cmd.angular.z=(start?0-punto.rotation.z:punto.rotation.z);

		printCmdInfo();

		double sleep=cmd.angular.z*3.0; //tempo necessario a tornare nella giusta orientazione
		if(sleep<0) sleep=-sleep;
		pub_topic.publish(cmd);
		ros::Duration(sleep).sleep();
		cmd.angular.z=0;
	}

	void printPositionInfo(){
		ROS_INFO_STREAM("Start Position: ["<<lastPosition.translation.x<<
				", "<<lastPosition.translation.y<<
				", "<<lastPosition.translation.z<<"] "<<
				"[ "<<lastPosition.rotation.x<<
				", "<<lastPosition.rotation.y<<
				", "<<lastPosition.rotation.z<<" ] ");
	}

	void printCmdInfo(){
		ROS_INFO_STREAM("cmd to execute: "<<"x:"<<cmd.linear.x
				<<" y: " << cmd.linear.y
				<<" z: " << cmd.linear.z
				<<" rX: " << cmd.angular.x
				<<" rY: " << cmd.angular.y
				<<" rZ: " << cmd.angular.z);
	}

};

int main(int argc, char** argv)
{
	ros::init(argc, argv, "action_controller_node");
	ros::NodeHandle node;//("~");
	Controller control(node);

	ros::spin();

	return 0;
}
