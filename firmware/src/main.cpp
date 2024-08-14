
#include <iostream>

#include "lumina.hpp"

#include "mavlink/common/mavlink.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <thread>
#include <chrono>

extern "C"
void app_main(void)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));

    lumina::wlan<lumina::AP> wlan("MAV", "12345678");
    wlan.enable();

    auto ip = wlan.ip();
    auto mask = wlan.mask();
    auto broadcast_ip = (ip & mask) | ~mask;

    std::this_thread::sleep_for(std::chrono::seconds(2));

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        std::cerr << "Error creating socket!" << std::endl;

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(14550);
    serverAddr.sin_addr.s_addr = broadcast_ip;
        
    while (true)
    {
        mavlink_message_t msg;
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        mavlink_heartbeat_t heartbeat;

        heartbeat.type = MAV_TYPE_QUADROTOR;           // Set UAV type (e.g., quadrotor)
        heartbeat.autopilot = MAV_AUTOPILOT_GENERIC;  // Set autopilot type
        heartbeat.base_mode = MAV_MODE_MANUAL_ARMED;   // Set mode (manual, armed, etc.)
        heartbeat.custom_mode = 0;                     // Custom mode (typically set to 0)
        heartbeat.system_status = MAV_STATE_ACTIVE;    // System status
        heartbeat.mavlink_version = MAVLINK_VERSION;   // MAVLink version

        mavlink_msg_heartbeat_encode(1, 1, &msg, &heartbeat);

        uint16_t messageLength = mavlink_msg_to_send_buffer(buffer, &msg);

        ssize_t sentBytes = sendto(sockfd, buffer, messageLength, 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (sentBytes != messageLength) {
            std::cerr << "Error sending heartbeat!" << std::endl;
        } else {
            std::cout << "Heartbeat sent!" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
