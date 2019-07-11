//
// Created by fulva on 2019-07-09.
//

#include <ros/ros.h>
#include <std_msgs/String.h>

void callback(const std_msgs::StringConstPtr &string) {
    static int count{0};
    ++count;
    ROS_INFO("I GOT %s FOR %d TIMES", string.get()->data.c_str(), count);
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "sub");
    ros::NodeHandle n;
    ros::Subscriber subscriber = n.subscribe<std_msgs::String>("string_topic", 32, boost::bind(&callback, _1));
    ros::spin();
}