#include <iostream>

#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_udp.h>

#include <vpad/input.h>
#include <padscore/wpad.h>
#include <padscore/kpad.h>
#include <deque>
#include <set>
#include <array>
#include <thread>
#include <unordered_map>

#include "net/endpoint.h"
#include "net/udp_socket.h"

#include "utils/reader.hpp"
#include "utils/logger.h"
#include "utils/letype.hpp"

std::unordered_map<sockets::endpoint, uint32_t> clients;

bool running = false;

void start_server();
void server_loop(sockets::udp_socket&);
VPADVec2D SwapEndian(const VPADVec2D& vec); // Swaps endianness for integral member values that take more than byte
VPADVec3D SwapEndian(const VPADVec3D& vec); // Swaps endianness for integral member values that take more than byte
VPADTouchData SwapEndian(const VPADTouchData& touchData); // Swaps endianness for integral member values that take more than byte
VPADStatus SwapEndian(const VPADStatus &status); // Swaps endianness for integral member values that take more than byte

int main(){
    WHBProcInit();
    WHBLogUdpInit();
    start_server();
    return 0;
}


void start_server(){
    std::thread loop_thread;

    sockets::udp_socket serverSocket;
    DEBUG_FUNCTION_LINE("Initialized socket");
    try {
        sockets::endpoint localEp{INADDR_ANY, 19470};
        DEBUG_FUNCTION_LINE("Initialized local EP on %s:%u", localEp.address(), localEp.port());

        serverSocket.set_option<int>(sockets::option_name::REUSE_ADDRESS, 1);
        serverSocket.set_option<int>(sockets::option_name::NON_BLOCK, 1);
        DEBUG_FUNCTION_LINE("Set socket option");

        serverSocket.bind(localEp);
        DEBUG_FUNCTION_LINE("Bound socket");
        running = true;

        loop_thread = std::thread(server_loop, std::ref(serverSocket));
        DEBUG_FUNCTION_LINE("Started loop thread");

    }
    catch (const std::runtime_error& error){
        DEBUG_FUNCTION_LINE("An error occurred: %s", error.what());
        running  = false;
    }
    catch (const std::invalid_argument& error){
        DEBUG_FUNCTION_LINE("Invalid argument: %s", error.what());
        running  = false;
    }
    DEBUG_FUNCTION_LINE("Waiting for server loop thread to join");
    loop_thread.join();

    WHBLogUdpDeinit();
    WHBProcShutdown();
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
            DEBUG_FUNCTION_LINE("An error occurred: %s", error.what());
            running = false;
        }
        catch (const std::invalid_argument& error){
            DEBUG_FUNCTION_LINE("Invalid argument: %s", error.what());
            running = false;
        }
        if (recvBytes <= 0)
            continue;

        if (senderEp != defaultEp && !clients.contains(senderEp)){
            DEBUG_FUNCTION_LINE("New client connected from %s:%u", senderEp.address(), senderEp.port());
            clients[senderEp] = 0;
        }
        else if (clients.empty())
            continue;
        VPADStatus status;
        VPADReadError error;
        VPADRead(VPADChan::VPAD_CHAN_0, &status, 1, &error);

        status = SwapEndian(status);
        try {
            socket.send_to(reinterpret_cast<uint8_t*>(&status), sizeof(VPADStatus), sockets::msg_flags::DONT_WAIT, senderEp);
        }
        catch (const std::runtime_error& error){
            DEBUG_FUNCTION_LINE("An error occurred: %s", error.what());
            running  = false;
        }
    }
    DEBUG_FUNCTION_LINE("Socket closed");
    running = false;
    socket.close();
}

VPADStatus SwapEndian(const VPADStatus& status){
    VPADStatus outStatus = status;
// Motion
    outStatus.accelorometer.acc = SwapEndian(outStatus.accelorometer.acc);
    outStatus.accelorometer.vertical = SwapEndian(outStatus.accelorometer.vertical);
    outStatus.accelorometer.magnitude = SwapEndian(outStatus.accelorometer.magnitude);
    outStatus.accelorometer.variation = SwapEndian(outStatus.accelorometer.variation);

    outStatus.direction.x = SwapEndian(outStatus.direction.x);
    outStatus.direction.y = SwapEndian(outStatus.direction.y);
    outStatus.direction.z = SwapEndian(outStatus.direction.z);

    outStatus.angle = SwapEndian(outStatus.angle);
    outStatus.gyro = SwapEndian(outStatus.gyro);
    outStatus.mag = SwapEndian(outStatus.mag);
// Sticks
    outStatus.rightStick = SwapEndian(outStatus.rightStick);
    outStatus.leftStick = SwapEndian(outStatus.leftStick);
// Buttons
    outStatus.trigger = SwapEndian(outStatus.trigger);
    outStatus.hold = SwapEndian(outStatus.hold);
    outStatus.release = SwapEndian(outStatus.release);
// Touchscreen
    outStatus.tpNormal = SwapEndian(outStatus.tpNormal);
    outStatus.tpFiltered1 = SwapEndian(outStatus.tpFiltered1);
    outStatus.tpFiltered2 = SwapEndian(outStatus.tpFiltered2);

    return outStatus;
}

VPADVec2D SwapEndian(const VPADVec2D& vec){
    return VPADVec2D {
        .x = SwapEndian(vec.x),
        .y = SwapEndian(vec.y)};
}

VPADVec3D SwapEndian(const VPADVec3D& vec){
    return VPADVec3D {
        .x = SwapEndian(vec.x),
        .y = SwapEndian(vec.y),
        .z = SwapEndian(vec.z)};
}

VPADTouchData SwapEndian(const VPADTouchData& touchData){
    return VPADTouchData {
        .x = SwapEndian(touchData.x),
        .y = SwapEndian(touchData.y),
        .touched = SwapEndian(touchData.touched),
        .validity = SwapEndian(touchData.validity)};
}
