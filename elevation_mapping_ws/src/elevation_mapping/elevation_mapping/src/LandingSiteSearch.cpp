#include <ros/ros.h>
#include "elevation_mapping/ElevationMapping.hpp"
#include <iostream>

int main(int argc, char **argv) {
    ros::init(argc, argv, "landing_site_search");
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<grid_map_msgs::GetGridMap>("/elevation_mapping/get_submap");
    
    grid_map_msgs::GetGridMap srv;
    srv.request.frame_id = "body";
    srv.request.layers = {"elevation"};
    srv.request.position_x = 0;
    srv.request.position_y = 0;
    srv.request.length_x = 15;
    srv.request.length_y = 15;

    if (client.call(srv))
    {
        ROS_INFO("SUCCESS!");
        grid_map_msgs::GridMap grid_map = srv.response.map;

        // map为grid_map中的elevation层数据，每个元素存储对应单元格的高度值，坐标系为机体坐标系
        // ROS_INFO("print:%d", grid_map.data[0].data.size());
        std::vector<float> map = grid_map.data[0].data;
        int map_size = map.size();

        // column_index
        int cols = grid_map.data[0].layout.dim[0].size;

        // row_index
        int rows = grid_map.data[0].layout.dim[1].size;
        
        // 解决map中的nan值
        for (int i = 0; i < map_size; i++) {
            if (map[i] > -100 && map[i] < 100) {
                // pass
            } else {
                map[i] = -100;
            } 
        }

        // 由map构造2d map
        int k = 0;
        std::vector<std::vector<float>> map2d(cols, std::vector<float>(rows, 0));
        for (int i = 0; i < cols; i++) {
            for (int j = 0; j < rows; j++) {
                map2d[i][j] = map[k];
                k++;
            }
        }
        std::cout << map2d.size() << " " << map2d[0].size() << std::endl;

        
    } else {
        ROS_ERROR("FAILED!");
        return 1;
    }
    return 0;
}