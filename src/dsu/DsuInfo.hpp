#pragma once
#include <wut_types.h>
#include <iostream>
#include <array>
#include <cstring>
#include <chrono>
namespace DSU {
    const uint32_t server_id = static_cast<uint32_t>(std::rand());
    enum class DSUMessageType : uint32_t {
        PROTOCOL_VERSION = 0x100000,
        CONTROLLER_INFO = 0x100001,
        CONTROLLER_DATA = 0X100002,
        INVALID = 0xFFFFFFFF
    };
    enum class SlotState : uint8_t{
        DISCONNECTED = 0,
        RESERVED = 1,
        CONNECTED = 2
    };
    enum class ConnectionType : uint8_t {
        NOT_APPLICABLE = 0x0,
        USB = 0x1,
        BT = 0x2
    };
    enum class DeviceModel : uint8_t {
        NOT_APPLICABLE = 0,
        NON_FULL_GYRO = 1,
        FULL_GYRO = 2
    };
    enum class BatteryLevel : uint8_t {
        NOT_APPLICABLE = 0x00,
        DYING = 0x01,
        LOW = 0x02,
        MEDIUM = 0x03,
        HIGH = 0x04,
        FULL = 0x05,
        CHARGING = 0xEE,
        CHARGED = 0xEF // When plugged in
    };
    enum class ButtonGroup1 : uint8_t {
        DPAD_LEFT = 1 << 0,
        DPAD_DOWN = 1 << 1,
        DPAD_RIGHT = 1 << 2,
        DPAD_UP = 1 << 3,
        OPTIONS = 1 << 4,
        R3 = 1 << 5,
        L3 = 1 << 6,
        SHARE = 1 << 7
    };
    inline ButtonGroup1 operator|(ButtonGroup1 a, ButtonGroup1 b)
    {
        return static_cast<ButtonGroup1>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    inline ButtonGroup1 operator&(ButtonGroup1 a, ButtonGroup1 b)
    {
        return static_cast<ButtonGroup1>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
    enum class ButtonGroup2 : uint8_t {
        Y = 1 << 0,
        B = 1 << 1,
        A = 1 << 2,
        X = 1 << 3,
        R1 = 1 << 4,
        L1 = 1 << 5,
        R2 = 1 << 6,
        L2 = 1 << 7
    };
    inline ButtonGroup2 operator|(ButtonGroup2 a, ButtonGroup2 b)
    {
        return static_cast<ButtonGroup2>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    inline ButtonGroup2 operator&(ButtonGroup2 a, ButtonGroup2 b)
    {
        return static_cast<ButtonGroup2>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    enum class RegistrationType : uint8_t {
        SUBSCRIBE_ALL = 0x0,
        SLOT_BASED = 0x1,
        MAC_BASED = 0x2
    };
    inline RegistrationType operator|(RegistrationType a, RegistrationType b)
    {
        return static_cast<RegistrationType>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }
    inline RegistrationType operator&(RegistrationType a, RegistrationType b)
    {
        return static_cast<RegistrationType>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }
    class MacAddress{
        std::array<uint8_t,6> m_data;
    public:
        explicit MacAddress(std::array<uint8_t,6> data) : m_data(){
            data = m_data;
        }
        MacAddress(uint8_t* data, uint8_t offset) : m_data(){
            std::copy(data, data + offset, m_data.begin());
        }
        MacAddress(uint32_t p1, uint16_t p2) : m_data(){
            std::memcpy(m_data.begin(), &p1, sizeof(p1));
            std::memcpy(m_data.begin() + sizeof(p1), &p2, sizeof(p2));
        }
        explicit MacAddress(uint64_t num) : m_data(){
            std::memcpy(m_data.begin(), &num, 6);
        }
        MacAddress() : m_data(){
            m_data.fill(0);
        }
        void SwapEndian(){
            constexpr auto length = 6;
            uint8_t temp;
            for (int i=0;i<length/2;i++)
            {
                temp = m_data[i];
                m_data[i] = m_data[length - i - 1];
                m_data[length - i - 1] = temp;
            }
        }
    };
}