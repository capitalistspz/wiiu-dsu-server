#include <iostream>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>

#include <vpad/input.h>
#include <padscore/wpad.h>

#include <deque>
#include <set>
#include <array>
#include <thread>
#include <unordered_map>
#include <cmath>

#include "net/endpoint.h"
#include "net/udp_socket.h"

#include "dsu/DsuPacket.hpp"
#include "utils/reader.hpp"
#include "utils/logger.h"
#include "net/client.hpp"

std::unordered_map<sockets::endpoint, net::client> clients;

std::unordered_map<VPADButtons, DSU::ButtonGroup2> face_button_map;
std::unordered_map<VPADButtons, DSU::ButtonGroup1> non_face_button_map;



bool running = false;

void start_server();
void map_buttons();
void server_loop(sockets::udp_socket&);

int main(){
    WHBProcInit();
    WHBLogUdpInit();
    WHBLogPrint("Hello world");

    start_server();
    return 0;
}


void start_server(){
    std::thread loop_thread;

    sockets::udp_socket serverSocket;
    DEBUG_FUNCTION_LINE("Initialized socket")
    try {
        sockets::endpoint localEp{INADDR_ANY, 26760};

        DEBUG_FUNCTION_LINE("Initialized local EP on %s:%u", localEp.address(), localEp.port())


        serverSocket.set_option<int>(sockets::option_name::REUSE_ADDRESS, 1);
        //serverSocket.set_option<int>(sockets::option_name::NON_BLOCK, 1);
        DEBUG_FUNCTION_LINE("Set socket option")

        serverSocket.bind(localEp);
        DEBUG_FUNCTION_LINE("Bound socket")
        running = true;

        loop_thread = std::thread(server_loop, std::ref(serverSocket));
        DEBUG_FUNCTION_LINE("Started loop thread")

    }
    catch (const std::runtime_error& error){
        DEBUG_FUNCTION_LINE("An error occurred: %s", error.what())
        running  = false;
    }
    catch (const std::invalid_argument& error){
        DEBUG_FUNCTION_LINE("Invalid argument: %s", error.what())
        running  = false;
    }
    DEBUG_FUNCTION_LINE("Waiting for server loop thread to join")
    loop_thread.join();
    WHBProcShutdown();
    WHBLogUdpDeinit();


}

void server_loop(sockets::udp_socket& socket){
    std::array<uint8_t, 1024> bufferIn{};
    std::array<uint8_t, 1024> bufferOut{};
    constexpr sockets::endpoint defaultEp{};
    while (running && WHBProcIsRunning()){
        sockets::endpoint senderEp{};
        ssize_t recvBytes = 0;

        try {
            recvBytes = socket.receive_from(bufferIn.begin(), 1024, sockets::msg_flags::DONT_WAIT, senderEp);
        }

        catch (const std::runtime_error& error){
            DEBUG_FUNCTION_LINE("An error occurred: %s", error.what())
            running  = false;
        }

        catch (const std::invalid_argument& error){
            DEBUG_FUNCTION_LINE("Invalid argument: %s", error.what())
            running  = false;
        }

        if (recvBytes > 0){
            DEBUG_FUNCTION_LINE("Received %d bytes", recvBytes)

            utils::reader reader(bufferIn.begin(), recvBytes);
            auto header = DSU::Packets::Header{};
            header.read(reader);
            header.swap_member_endian();

            if (clients.contains(senderEp)){
                ++clients[senderEp].packet_number;
            }
            else if(defaultEp != senderEp){
                DEBUG_FUNCTION_LINE("New client connected from %s:%u", senderEp.address(), senderEp.port())
                clients[senderEp] = net::client{.remote_ep = senderEp, .client_id = header.peer_id, .packet_number = 0};
            }

            if (clients.empty())
                continue;

            DEBUG_FUNCTION_LINE("Packet Header: {Source: %s, Protocol Version: %u, Packet Length: %u, CRC 32: %u, ID: %u, Message Type: %u}",
                                header.magic_string, header.protocol_version, header.packet_length, header.crc_32, header.peer_id, static_cast<uint32_t>(header.message_type))

            DSU::Packets::Outgoing::OutgoingPacket packet{bufferOut.begin(), bufferOut.size()};

            if (header.message_type == DSU::DSUMessageType::PROTOCOL_VERSION){
                DEBUG_FUNCTION_LINE("Received protocol version request")

                DSU::Packets::Header headerOut;
                headerOut.message_type = DSU::DSUMessageType::PROTOCOL_VERSION;

                DSU::Packets::Outgoing::VersionInfo versionInfo;
                versionInfo.max_protocol_version = 1001;

                headerOut.swap_member_endian();
                versionInfo.swap_member_endian();
                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())

                packet.add_data(headerOut);
                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())

                packet.add_data(versionInfo);
                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())

                packet.set_crc32();

                const auto sentBytes = socket.send_to(packet.begin(), packet.cursor(), sockets::msg_flags::DONT_WAIT, senderEp);
                DEBUG_FUNCTION_LINE("Sent %d bytes", sentBytes)

            }
            else if (header.message_type == DSU::DSUMessageType::CONTROLLER_INFO){
                DEBUG_FUNCTION_LINE("Received controller information request")

                VPADStatus vpadStatus;
                VPADReadError error;
                VPADRead(VPADChan::VPAD_CHAN_0, &vpadStatus, 1, &error);

                DSU::Packets::Header headerOut{};
                headerOut.message_type = DSU::DSUMessageType::CONTROLLER_INFO;

                DSU::Packets::Outgoing::ControllerResponseHead crh{};
                crh.reporting_slot = 0;
                crh.slot_state = DSU::SlotState::CONNECTED;
                crh.device_model = DSU::DeviceModel::FULL_GYRO;
                crh.connection_type = DSU::ConnectionType::NOT_APPLICABLE;
                crh.mac_address = DSU::MacAddress{0};
                crh.battery_level = static_cast<DSU::BatteryLevel>(vpadStatus.battery / 6);

                DSU::Packets::Outgoing::ConnectedControllers cc;
                cc.head = crh;
                cc.tail = '\0';

                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())
                packet.add_data(headerOut);
                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())
                packet.add_data(cc);
                DEBUG_FUNCTION_LINE("Cursor at %u", packet.cursor())
                packet.set_crc32();

                const auto sentBytes = socket.send_to(packet.begin(), packet.cursor(), sockets::msg_flags::DONT_WAIT, senderEp);
                DEBUG_FUNCTION_LINE("Sent %d bytes", sentBytes)
            }
            else if (header.message_type == DSU::DSUMessageType::CONTROLLER_DATA){
                DEBUG_FUNCTION_LINE("Received controller data request")

                VPADStatus vpadStatus;
                VPADReadError error;
                VPADRead(VPADChan::VPAD_CHAN_0, &vpadStatus, 1, &error);

                DSU::Packets::Header headerOut{};
                headerOut.message_type = DSU::DSUMessageType::CONTROLLER_DATA;

                DSU::Packets::Outgoing::ControllerResponseHead crh{};
                crh.reporting_slot = 0;
                crh.slot_state = DSU::SlotState::CONNECTED;
                crh.device_model = DSU::DeviceModel::FULL_GYRO;
                crh.connection_type = DSU::ConnectionType::NOT_APPLICABLE;
                crh.mac_address = DSU::MacAddress{0};
                crh.battery_level = static_cast<DSU::BatteryLevel>(vpadStatus.battery / 6);

                DSU::Packets::Outgoing::ControllerData data{};

                data.beginning = crh;
                data.connected = true;
                data.packet_number = ++clients[senderEp].packet_number;

                for (auto kp : non_face_button_map){
                    if (vpadStatus.trigger & kp.first){
                        data.button_mask_1 = data.button_mask_1 | kp.second;
                    }
                }
                for (auto kp : face_button_map){
                    if (vpadStatus.trigger & kp.first){
                        data.button_mask_2 = data.button_mask_2 | kp.second;
                    }
                }
                data.home_button = vpadStatus.trigger & VPADButtons ::VPAD_BUTTON_HOME;
                data.touch_button = false; // No idea what this is equivalent to
                data.l_stick.x = (uint8_t)std::round(((vpadStatus.leftStick.x + 1) / 2) * 256);
                data.l_stick.y = (uint8_t)std::round(((vpadStatus.leftStick.y + 1) / 2) * 256);
                data.r_stick.x = (uint8_t)std::round(((vpadStatus.rightStick.x + 1) / 2) * 256);
                data.r_stick.y = (uint8_t)std::round(((vpadStatus.rightStick.y + 1) / 2) * 256);

                data.analog_dp.left = 255 * ((data.button_mask_1 & DSU::ButtonGroup1::DPAD_LEFT) == DSU::ButtonGroup1::DPAD_LEFT);
                data.analog_dp.down = 255 * ((data.button_mask_1 & DSU::ButtonGroup1::DPAD_DOWN) == DSU::ButtonGroup1::DPAD_DOWN);

                data.analog_dp.right = 255 * ((data.button_mask_1 & DSU::ButtonGroup1::DPAD_RIGHT) == DSU::ButtonGroup1::DPAD_RIGHT);
                data.analog_dp.up = 255 * ((data.button_mask_1 & DSU::ButtonGroup1::DPAD_UP) == DSU::ButtonGroup1::DPAD_UP);

                data.analog_face.y = 255 * ((bool)(data.button_mask_2 & DSU::ButtonGroup2::Y));
                data.analog_face.b = 255 * ((bool)(data.button_mask_2 & DSU::ButtonGroup2::B));
                data.analog_face.a = 255 * ((bool)(data.button_mask_2 & DSU::ButtonGroup2::A));
                data.analog_face.x = 255 * ((bool)((data.button_mask_2 & DSU::ButtonGroup2::X)));

                data.first_touch.active = 0;
                data.second_touch.active = 0;

                data.accelerometer.x = vpadStatus.accelorometer.acc.x;
                data.accelerometer.y = vpadStatus.accelorometer.acc.y;
                data.accelerometer.z = vpadStatus.accelorometer.acc.z;

                data.gyroscope.pitch = vpadStatus.gyro.x;
                data.gyroscope.roll = vpadStatus.gyro.z;
                data.gyroscope.yaw = vpadStatus.gyro.y;

                packet.add_data(headerOut);
                packet.add_data(data);

                const auto sentBytes = socket.send_to(packet.begin(), packet.cursor(), sockets::msg_flags::DONT_WAIT, senderEp);
            }
        }

    }
    DEBUG_FUNCTION_LINE("Socket closed")
    running = false;
    socket.close();
}

void map_buttons(){
    face_button_map[VPAD_BUTTON_A] = DSU::ButtonGroup2::A;
    face_button_map[VPAD_BUTTON_B] = DSU::ButtonGroup2::B;
    face_button_map[VPAD_BUTTON_X] = DSU::ButtonGroup2::X;
    face_button_map[VPAD_BUTTON_Y] = DSU::ButtonGroup2::Y;
    non_face_button_map[VPAD_BUTTON_LEFT] = DSU::ButtonGroup1::DPAD_LEFT;
    non_face_button_map[VPAD_BUTTON_RIGHT] = DSU::ButtonGroup1::DPAD_RIGHT;
    non_face_button_map[VPAD_BUTTON_UP] = DSU::ButtonGroup1::DPAD_UP;
    non_face_button_map[VPAD_BUTTON_DOWN] = DSU::ButtonGroup1::DPAD_DOWN;

    face_button_map[VPAD_BUTTON_ZL] = DSU::ButtonGroup2::L2;
    face_button_map[VPAD_BUTTON_ZR] = DSU::ButtonGroup2::R2;
    face_button_map[VPAD_BUTTON_L] = DSU::ButtonGroup2::L1;
    face_button_map[VPAD_BUTTON_R] = DSU::ButtonGroup2::R1;

    non_face_button_map[VPAD_BUTTON_PLUS] = DSU::ButtonGroup1::OPTIONS;
    non_face_button_map[VPAD_BUTTON_MINUS] = DSU::ButtonGroup1::SHARE;
    non_face_button_map[VPAD_BUTTON_STICK_L] = DSU::ButtonGroup1::L3;
    non_face_button_map[VPAD_BUTTON_STICK_R] = DSU::ButtonGroup1::R3;
}