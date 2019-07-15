//
// Created by fulva on 2019-07-11.
//

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <QtBluetooth>

void bluetooth_rec(const std_msgs::StringConstPtr &string, QBluetoothSocket **ppSocket) {
    if (*ppSocket != nullptr) {
        std::string msg = string.get()->data;
        msg.push_back('\n');
        (**ppSocket).write(msg.c_str());
    }
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ros::init(argc, argv, "bluetooth");
    ros::NodeHandle n;
    ros::Publisher pub = n.advertise<std_msgs::String>("bluetooth", 10);
    QBluetoothSocket *pSocket_cal{nullptr};
    QBluetoothSocket **ppSocket{&pSocket_cal};
    ros::Subscriber sub = n.subscribe<std_msgs::String>("bluetooth_rec", 10, boost::bind(&bluetooth_rec, _1, ppSocket));
    QBluetoothLocalDevice localDevice;
    QString localDeviceName;
    // Check if Bluetooth is available on this device
    if (localDevice.isValid()) {

        // Turn Bluetooth on
        localDevice.powerOn();

        // Read local device name
        localDeviceName = localDevice.name();
        ROS_INFO("%s", localDeviceName.toStdString().c_str());


        // Make it visible to others
//        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }
    QBluetoothServer *rfcommServer = new QBluetoothServer(QBluetoothServiceInfo::RfcommProtocol);
    QBluetoothServiceInfo info = rfcommServer->listen(QBluetoothUuid::SerialPort);
    ROS_INFO("is %s registed", info.isRegistered() ? "" : "not");
    ROS_INFO("is %s completed", info.isComplete() ? "" : "not");
    ros::Rate rate(10);
    while (ros::ok()) {
        ROS_INFO("there is %s pending connection", rfcommServer->hasPendingConnections() ? "" : "not");
        if (rfcommServer->hasPendingConnections()) {
            QBluetoothSocket *pSocket = rfcommServer->nextPendingConnection();
            pSocket_cal = pSocket;
            pSocket->open(QIODevice::OpenModeFlag::ReadWrite);
            while (pSocket->isOpen()) {
                if (pSocket->canReadLine()) {
                    const QByteArray &array = pSocket->readLine(20).trimmed();
                    ROS_INFO("%s", array.toStdString().c_str());
                    std_msgs::String msg;
                    msg.data = array.toStdString();
                    pub.publish(msg);
                }
                ros::spinOnce();
                app.processEvents();
                rate.sleep();
            }
            pSocket_cal = nullptr;
            delete pSocket;
        }
        ros::spinOnce();
        app.processEvents();
        rate.sleep();
    }
    return 0;
}


