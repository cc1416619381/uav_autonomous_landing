###################环境配置##################
一、PX4+MAVROS+GAZEBO无人机仿真环境配置
1、参考https://gaas.gitbook.io/guide/software-realization-build-your-own-autonomous-drone/wu-ren-ji-zi-dong-jia-shi-xi-lie-offboard-kong-zhi-yi-ji-gazebo-fang-zhen安装ROS、MAVROS，编译PX4
2、将模型中iris_realsense_downward拷贝到~/catkin_ws/src/Firmware/Tools/sitl_gazebo/models/
3、将模型中realsense_camera拷贝到~/.gazebo/models/
4、将无人机参数文件中iris_realsense_downward拷贝到~/catkin_ws/src/Firmware/posix-configs/SITL/init/ekf2
5、编译realsense插件中realsense_ros_gazebo
编译方法：将realsense_ros_gazebo拷贝到ROS工作空间下catkin_make编译并source
6、将uav_mapping.launch拷贝到~/catkin_ws/src/Firmware/launch/
7、运行仿真环境roslaunch px4 uav_mapping.launch，用rostopic list查看是否有相机相关话题，如果有则配置成功。

二、VINS-Fusion编译
1、安装ceres
2、把压缩包中vins_fusion_ws拷贝出来
3、cd vins_fusion_ws
4、catkin_make

三、elevation_mapping编译
1、参考github安装相关依赖
2、mkdir -p ~/elevation_ws/src
3、把压缩包中elevation_mapping复制到~/elevation_ws/src/
4、cd ~/elevation
5、catkin_make

###################运行步骤##################
1.启动仿真环境并通过地面站配置无人机飞行速度参数
roslaunch px4 uav_mapping.launch
2.到地面站配置无人机飞行速度
首先双击QGroundControl.AppImage图标启动地面站，然后点击页面最上方齿轮图标进入设置页面，接着选择最左边那一列中的Parameters标签，最后搜索MAP_XY_VEL_MAX，并把值修改为1m/s
3.切换到脚本所在目录
cd ~/vins_fusion_ws/scripts
4.运行无人机驱动，无人机起飞到固定高度后悬停
python px4_mavros_run.py
5.等无人机稳定悬停后再启动vins-fusion！！！！
bash xtdrone_run_vio.sh
6.启动定位数据类型转换节点，将vins-fusion发布的odometry数据类型转换为elevation_mapping订阅的Posewithcovariance类型
rosrun vins Odom2PoseCov
7.发布realsense_camera::camera_depth_frame和camera_link之间的变换关系到/tf，近似认为两者一致
rosrun tf static_transform_publisher 0 0 0 0 0 camera_link  realsense_camera::camera_depth_frame 100
8.启动elevation_mapping
roslaunch elevation_mapping_demos realsense_demo.launch
9.启动无人机运动程序，无人机遍历地图后会降落到空地上
python commander.py


###################备注##################
1、世界模型修改方法
到github或gazebo官网下载想要的模型到~/.gazebo/models，打开gazebo选择insert插入对应模型，修改后保存world就可以。
2、无人机定位数据来源配置
QGroundControl中选择Parameter标签,搜索ekf2_aid_mask，勾选vision position fusion和vision yaw fusion两项后保存

