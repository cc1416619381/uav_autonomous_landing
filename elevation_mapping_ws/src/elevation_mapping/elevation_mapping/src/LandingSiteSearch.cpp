#include <ros/ros.h>
#include "elevation_mapping/ElevationMapping.hpp"
#include <std_msgs/String.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <cmath>

// define the expect size with EXPECT_SIZE
#define EXPECT_SIZE 4 
// the height difference is considered flat within the HEIGHT_ALLOWANCE
#define HEIGHT_ALLOWANCE 0.4

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

float get_flatness_cost(int i, int j, int expect_size, const std::vector<std::vector<float>>& map2d) {
    int central_x = i + (expect_size - 1) / 2;
    int central_y = j + (expect_size - 1) / 2;
    float central_h = map2d[central_x][central_y];
    float height_diff;
    float flatness_cost = 0;            
    for (int row = i; row < i + expect_size; row++) {
        for (int col = j; col < j + expect_size; col++) {
            height_diff = map2d[row][col] - central_h;
            if (fabs(height_diff) > HEIGHT_ALLOWANCE) {
                return -1;
            } else {
                flatness_cost += height_diff * height_diff;
            }
        }
    }
    return flatness_cost;
}

std::stringstream find_landing_site(const std::vector<std::vector<float>>& map2d) {
    std::stringstream landing_site;
    // convert to the size in cell with the resolution 0.2m/cell
    int expect_size = EXPECT_SIZE * 5;
    int rows = map2d.size();
    int cols = map2d[0].size();
    if (expect_size > rows || expect_size > cols) {
        ROS_ERROR("EXPECT SIZE IS LARGER THAN THE SUBMAP SIZE!");
    }
    float min_flatness_cost = expect_size * expect_size * 100;
    float flatness_cost;
    int landing_site_x = (rows - 1) / 2;
    int landing_site_y = (cols - 1) / 2;
    bool landing_trigger = false;
    for (int i = 0; i < rows - expect_size + 1; i++) {
        for (int j = 0; j < cols - expect_size + 1; j++) {
            flatness_cost = get_flatness_cost(i, j, expect_size, map2d);
            if (fabs(flatness_cost - -1) < 0.01) {
                // ROS_INFO("LANDING NOT TRIGGER!");
                continue;
            } else if (flatness_cost < min_flatness_cost) {
                landing_trigger = true;
                ROS_INFO("LANDING TRIGGER!");
                min_flatness_cost = flatness_cost;
                landing_site_x = i + (expect_size - 1) / 2;
                landing_site_y = j + (expect_size - 1) / 2;
            }
        }
    }

    landing_site << landing_site_x << ',' << landing_site_y;
    return landing_site;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "landing_site_search");
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<grid_map_msgs::GetGridMap>("/elevation_mapping/get_submap");
    ros::Publisher position_pub = n.advertise<std_msgs::String>("landing_site_search_point", 1000);

    grid_map_msgs::GetGridMap srv;
    init_srv(srv);
    ROS_INFO("INIT SRV SUCCESS!");

    std::vector<std::vector<float>> map2d;
    ros::Rate loop_rate(1);
    while (ros::ok()) {
        get_submap_from_srv(client, srv, map2d);
        ROS_INFO("GET_SUBMAP_FROM_SRV SUCCESS!");

        std_msgs::String msg;
        std::stringstream ss = find_landing_site(map2d);
        // std::stringstream ss;
        // ss << "1,0";
        msg.data = ss.str();
        ROS_INFO("position (%s) is published.", msg.data.c_str());
        position_pub.publish(msg);
        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}