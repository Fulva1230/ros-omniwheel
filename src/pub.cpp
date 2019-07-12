#include <ros/ros.h>
#include <std_msgs/String.h>
#include <boost/asio.hpp>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

volatile bool comp = false;
volatile bool processing = false;
boost::array<char, 20> array{0};

void handler(
        const boost::system::error_code &error, // Result of operation.
        std::size_t bytes_transferred           // Number of bytes read.
) {
    ROS_INFO("FINISH");
    comp = true;
    processing = false;
}

std::size_t completion_condition(
        // Result of latest async_read_some operation.
        const boost::system::error_code &error,

        // Number of bytes transferred so far.
        std::size_t bytes_transferred
) {
    if (bytes_transferred <= 0) {
        return 1;
    }
    return array.at(bytes_transferred - 1) == '\n' ? 0 : 1;
}

int main(int argc, char **argv) {

    ros::init(argc, argv, "init");
    boost::asio::io_service io_service;
    ROS_INFO("SERVICE ON");
    boost::asio::serial_port serial_port(io_service, "/dev/rfcomm0");
    ROS_INFO((serial_port.is_open() ? "open" : "not open"));
    ROS_INFO("SERIAL OPEN");
    std_msgs::String string;
    string.data = "Hello World";
    ROS_INFO("INIxT");
    ROS_INFO("argument size is %d", argc);
    if (argc == 2) {
        ROS_INFO("argument check");
        string.data = "Hello " + std::string(argv[1]);
    }
    ros::NodeHandle n;
    ros::Publisher publisher = n.advertise<std_msgs::String>("string_topic", 32);
    ros::Rate rate(10);
    while (ros::ok()) {
        if (comp) {
            ROS_INFO("%s", array.data());
            string.data.append(array.data());
            array.fill(0);
            comp = false;
        } else {
            if (processing) {

            } else {
                ROS_INFO("START READ");
                boost::asio::async_read(serial_port, boost::asio::buffer(array), &completion_condition, &handler);
                processing = true;
            }
        }
        io_service.poll_one();
        io_service.reset();
        publisher.publish(string);
        rate.sleep();
        ros::spinOnce();
    }
    return 0;
}