//
// Created by fulva on 2019-07-11.
//

#include <ros/ros.h>
#include <std_msgs/String.h>
#include <QtBluetooth>

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    ros::init(argc, argv, "bluetooth");
    ros::NodeHandle n;
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
    ros::Rate rate(1);
    while (ros::ok()) {
        ROS_INFO("there is %s pending connection", rfcommServer->hasPendingConnections() ? "" : "not");
        if (rfcommServer->hasPendingConnections()) {
            QBluetoothSocket *pSocket = rfcommServer->nextPendingConnection();
            assert(pSocket != nullptr);
            pSocket->open(QIODevice::OpenModeFlag::ReadWrite);
            while (pSocket->isOpen()) {
                const QByteArray &array = pSocket->readAll();
                ROS_INFO("%s", array.toStdString().c_str());
                ros::spinOnce();
                app.processEvents();
                rate.sleep();
            }
        }
        ros::spinOnce();
        app.processEvents();
        rate.sleep();
    }
    return 0;
}


