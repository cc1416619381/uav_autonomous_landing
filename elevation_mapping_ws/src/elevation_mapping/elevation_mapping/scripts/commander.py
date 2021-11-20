#!/usr/bin/env python
import rospy
from mavros_msgs.msg import GlobalPositionTarget, State
from mavros_msgs.srv import CommandBool, CommandTOL, SetMode
from geometry_msgs.msg import PoseStamped, Twist
from sensor_msgs.msg import Imu, NavSatFix
from std_msgs.msg import Float32, String
# from pyquaternion import Quaternion
import time
import math


class Commander:
    def __init__(self):
        rospy.init_node("commander_node")
        rate = rospy.Rate(20)
        self.position_target_pub = rospy.Publisher('gi/set_pose/position', PoseStamped, queue_size=10)
        self.yaw_target_pub = rospy.Publisher('gi/set_pose/orientation', Float32, queue_size=10)
        self.custom_activity_pub = rospy.Publisher('gi/set_activity/type', String, queue_size=10)


    def move(self, x, y, z, BODY_OFFSET_ENU=True):
        self.position_target_pub.publish(self.set_pose(x, y, z, BODY_OFFSET_ENU))


    def turn(self, yaw_degree):
        self.yaw_target_pub.publish(yaw_degree)

    
    # land at current position
    def land(self):
        self.custom_activity_pub.publish(String("LAND"))


    # hover at current position
    def hover(self):
        self.custom_activity_pub.publish(String("HOVER"))


    # return to home position with defined height
    def return_home(self, height):
        self.position_target_pub.publish(self.set_pose(0, 0, height, False))


    def set_pose(self, x=0, y=0, z=2, BODY_FLU = True):
        pose = PoseStamped()
        pose.header.stamp = rospy.Time.now()

        # ROS uses ENU internally, so we will stick to this convention
        if BODY_FLU:
            pose.header.frame_id = 'base_link'

        else:
            pose.header.frame_id = 'map'

        pose.pose.position.x = x
        pose.pose.position.y = y
        pose.pose.position.z = z

        return pose

def command_callback(data, con):
    # data.data is described with x,y
    position2d = data.data.split(",")
    position_x = position2d[0]
    position_y = position2d[1]

    # now position_x = x, position_y = y, but is char type
    # print(position_x)
    # print(position_y)

    # con.move() need float args
    con.move(float(position_x), float(position_y), 0)
    time.sleep(10)

def listener(con):
    rospy.Subscriber("landing_site_search_point", String, command_callback, con)
    rospy.spin()

if __name__ == "__main__":
    con = Commander()
    listener(con)
    # con = Commander()
    # time.sleep(2)
    # con.move(6, -3, 0)
    # time.sleep(10)
    # con.move(0, 15, 0)
    # time.sleep(20)
    # con.move(6, 0, 0)
    # time.sleep(10)
    # con.move(0, -15, 0)
    # time.sleep(20)
    # con.move(6, 0, 0)
    # time.sleep(10)
    # con.move(0, 15, 0)
    # time.sleep(20)
    # con.move(6, 0, 0)
    # time.sleep(10)
    # con.move(0, -15, 0)
    # time.sleep(20)
    # con.move(-5,3,0)
    # time.sleep(15)
    # con.land()
    # time.sleep(2)

