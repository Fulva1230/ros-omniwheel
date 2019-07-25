//
// Created by fulva on 2019-07-25.
//

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <ros/ros.h>
#include <std_msgs/String.h>

using namespace boost::asio;

void read_handler(
        const boost::system::error_code &ec,
        std::size_t bytes_transferred,
        const ros::Publisher &pub,
        const char *buffer
) {
    ROS_INFO("GOT MESSAGE");
    std_msgs::String msg{};
    std::copy(buffer, buffer + bytes_transferred, std::inserter(msg.data, msg.data.end()));
    boost::trim(msg.data);
    boost::trim_if(msg.data, [](char c) -> bool {
        return c == '\0';
    });
    pub.publish(msg);

}

size_t comp(const boost::system::error_code &ec,
            std::size_t bytes_transferred,
            const char *buffer) {
    if (bytes_transferred > 1) {
        ROS_INFO("CHECK COMPLETENESS");
        ROS_INFO("CHECK CHARACTER %c", buffer[bytes_transferred - 1]);
        return buffer[bytes_transferred - 1] != '\n';
    }
    return 1;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "MCU_con");
    ros::NodeHandle n;
    ROS_INFO("INIT");
    io_service service;
    ROS_INFO("INITT");
    serial_port serialPort{service, "/dev/ttyAMA0"};
    if (!serialPort.is_open()) {
        ROS_INFO("NOT OPEN");
        return 1;
    }
    ros::Publisher pub = n.advertise<std_msgs::String>("MCU", 20);
    ros::Rate rate{4};
    std::array<char, 20> bufferArray{};
    while (ros::ok()) {
        async_read(serialPort,
                   buffer(bufferArray),
                   boost::bind(comp, _1, _2, bufferArray.data()),
                   boost::bind(&read_handler, _1, _2, pub, bufferArray.data()));
        service.poll();
        write(serialPort, buffer("abcdefgggg\n"));
        ros::spinOnce();
        ROS_INFO("SPIN ONCE");
        rate.sleep();
        service.reset();
    }
    ROS_INFO("CLOSE");
}