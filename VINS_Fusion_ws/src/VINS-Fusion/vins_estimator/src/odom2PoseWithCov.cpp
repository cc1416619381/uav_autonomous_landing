#include "ros/ros.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "nav_msgs/Odometry.h"

ros::Publisher uav_pose_pub;

ros::Subscriber camera_pose_sub;

geometry_msgs::PoseWithCovarianceStamped uav_pose;

void callback(const nav_msgs::Odometry::ConstPtr& msg)
{
  uav_pose.header = msg->header;
  uav_pose.pose = msg->pose;
  uav_pose_pub.publish(uav_pose);
}

int main(int argc, char **argv)
{

  ros::init(argc, argv, "Odom_2_PoseWithCov_node");

  ros::NodeHandle n;

  camera_pose_sub = n.subscribe("/vins_estimator/camera_pose", 10, callback);

  uav_pose_pub = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/pose", 10);
  
  ros::spin();

  return 0;
}
