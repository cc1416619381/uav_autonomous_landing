#include <ros/ros.h>
#include "elevation_mapping/ElevationMapping.hpp"
#include <std_msgs/String.h>
#include <iostream>
#include <vector>
#include <sstream>

void init_srv(grid_map_msgs::GetGridMap& srv) {
    srv.request.frame_id = "body";
    srv.request.layers = {"elevation"};
    srv.request.position_x = 0;
    srv.request.position_y = 0;
    srv.request.length_x = 15;
    srv.request.length_y = 15;
}

void get_submap_from_srv(ros::ServiceClient& client, grid_map_msgs::GetGridMap& srv, std::vector<std::vector<float>>& map2d) {
    if (client.call(srv)) {
        ROS_INFO("CALL GET_SUBMAP SRV SUCCESS!");
        grid_map_msgs::GridMap grid_map = srv.response.map;
        
        // map is the data in elevation layer of grid_map, each element corresponding the height of the grid
        // ROS_INFO("print:%d", grid_map.data[0].data.size());
        std::vector<float> map = grid_map.data[0].data;
        int map_size = map.size();

        // get row_index and column_index
        int cols = grid_map.data[0].layout.dim[0].size;
        int rows = grid_map.data[0].layout.dim[1].size;
        
        // convert nan value to -100
        for (int i = 0; i < map_size; i++) {
            if (map[i] > -100 && map[i] < 100) {
                // pass
            } else {
                map[i] = -100; 
            } 
        }

        // construct map2d from 1d representation
        int k = 0;
        std::vector<float> tmp;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                tmp.push_back(map[k]);
                if (j == cols - 1) {
                    map2d.push_back(tmp);
                    tmp.clear();
                }
                k++;
            }
        }
        ROS_INFO("obtain sub_map with size: %d * %d", map2d.size(), map2d[0].size());
    } else {
        ROS_ERROR("CALL GET_SUBMAP SRV FAILED!");
        return;
    }
}

// std::stringstream find_landing_site(std::vector<std::vector<float>>& map2d) {
//     std::stringstream landing_site;

// }

int main(int argc, char **argv) {
    ros::init(argc, argv, "landing_site_search");
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<grid_map_msgs::GetGridMap>("/elevation_mapping/get_submap");
    ros::Publisher position_pub = n.advertise<std_msgs::String>("landing_site_search_point", 1000);

    grid_map_msgs::GetGridMap srv;
    init_srv(srv);
    ROS_INFO("INIT SRV SUCCESS!");

    std::vector<std::vector<float>> map2d;
    get_submap_from_srv(client, srv, map2d);
    ROS_INFO("GET_SUBMAP_FROM_SRV SUCCESS!");

    ros::Rate loop_rate(1);
    while (ros::ok()) {
        std_msgs::String msg;
        // std::stringstream ss = find_landing_site(map2d);
        std::stringstream ss;
        ss << "1,0";
        msg.data = ss.str();
        // ROS_INFO("position (%s) is published.", msg.data.c_str());
        position_pub.publish(msg);
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}