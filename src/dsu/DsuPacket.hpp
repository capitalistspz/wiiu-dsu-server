#include "../utils/letype.hpp"
#include "../utils/reader.hpp"
#include "../utils/writer.hpp"
#include "../utils/crc.hpp"

#include "DsuInfo.hpp"

namespace DSU::Packets {
    struct PacketData {
        /**
         * Swaps endianness for any integer values that are supposed to be represented as multiple bytes,
         * used due to the Wii U being Big Endian
         */
        virtual void swap_member_endian() = 0;

        /** Reads bytes from the reader into a structure
         * @param writer the target writer
         */
        virtual void read(utils::reader &) = 0;

        /** Writes bytes from the structure into a writer
         * @param writer the target writer
         */
        virtual void write(utils::writer &) const = 0;

        /**
         *
         * @return number of bytes taken by this piece of data
         */
        [[nodiscard]] virtual size_t size() const = 0;
    };

    struct Header : PacketData {
        std::array<char, 4> magic_string = {'D', 'S', 'U', 'S'};
        uint16_t protocol_version = 1001u;
        uint16_t packet_length;
        uint32_t crc_32;
        uint32_t peer_id{};
        DSUMessageType message_type;

        Header(DSUMessageType messageType, uint32_t CRC32) {
            packet_length = sizeof(DSUMessageType);
            message_type = messageType;
            crc_32 = CRC32;
        }

        Header()
        : packet_length(sizeof(DSUMessageType)), message_type(DSUMessageType::INVALID), crc_32(0), peer_id(server_id) {}

        void swap_member_endian() override {
            protocol_version = SwapEndian(protocol_version);
            packet_length = SwapEndian(packet_length);
            crc_32 = SwapEndian(crc_32);
            peer_id = SwapEndian(peer_id);
            message_type = SwapEndian(message_type);
        }

        void read(utils::reader &reader) override {
            reader.read(magic_string);
            reader.read(protocol_version);
            reader.read(packet_length);
            reader.read(crc_32);
            reader.read(peer_id);
            reader.read(message_type);
        }

        void write(utils::writer &writer) const override {
            writer.write(magic_string);
            writer.write(protocol_version);
            writer.write(packet_length);
            writer.write(crc_32);
            writer.write(peer_id);
            writer.write(message_type);
        }

        [[nodiscard]] constexpr size_t size() const override {
            return 24;
        }
    };
    namespace Incoming {
        struct ConnectedControllers : PacketData {
            int32_t report_port_count;
            std::vector<uint8_t> port_id;

            void swap_member_endian() override {
                report_port_count = SwapEndian(report_port_count);
            }

            void read(utils::reader &reader) override {
                reader.read(report_port_count);
                reader.read(port_id, report_port_count);
            }

            void write(utils::writer &writer) const override {
                writer.write(report_port_count);
                writer.write(port_id, port_id.size());
            }

            [[nodiscard]] size_t size() const override {
                return sizeof(report_port_count) + report_port_count;
            }
        };

        struct ControllerData : PacketData {
            RegistrationType registration_type{};
            uint8_t reporting_slot{};
            MacAddress mac_address;

            void swap_member_endian() override {
                mac_address.SwapEndian();
            }

            void read(utils::reader &reader) override {
                reader.read(registration_type);
                reader.read(reporting_slot);
                reader.read(mac_address);
            }

            void write(utils::writer &writer) const override {}

            [[nodiscard]] size_t size() const override {
                auto size = sizeof(registration_type);
                if ((registration_type & RegistrationType::MAC_BASED) == RegistrationType::MAC_BASED)
                    size += sizeof(mac_address);
                else if ((registration_type & RegistrationType::SLOT_BASED) == RegistrationType::SLOT_BASED) {
                    size += sizeof(reporting_slot);
                }
                return size;
            }
        };
    }

    namespace Outgoing {
        struct ControllerResponseHead : PacketData {
            uint8_t reporting_slot{};
            SlotState slot_state{};
            DeviceModel device_model{};
            ConnectionType connection_type{};
            MacAddress mac_address{};
            BatteryLevel battery_level{};

            ControllerResponseHead() = default;

            void swap_member_endian() override {
                mac_address.SwapEndian();
            }

            void read(utils::reader &reader) override {
                reader.read(reporting_slot);
                reader.read(slot_state);
                reader.read(device_model);
                reader.read(connection_type);
                reader.read(mac_address);
                reader.read(battery_level);
            }

            void write(utils::writer &writer) const override {
                writer.write(reporting_slot);
                writer.write(slot_state);
                writer.write(device_model);
                writer.write(connection_type);
                writer.write(mac_address);
                writer.write(battery_level);
            }

            [[nodiscard]] constexpr size_t size() const override {
                return 11;
            }
        };

        struct TouchData : PacketData {
            uint8_t active{};
            uint8_t id{};
            uint16_t x{};
            uint16_t y{};

            TouchData() = default;

            void swap_member_endian() override {
                x = SwapEndian(x);
                y = SwapEndian(y);
            }

            void read(utils::reader &reader) override {
                reader.read(active);
                reader.read(id);
                reader.read(x);
                reader.read(y);
            }

            void write(utils::writer &writer) const override {
                writer.write(static_cast<uint8_t>((active != 0) ? 1 : 0));
                writer.write(id);
                writer.write(x);
                writer.write(y);
            }

            [[nodiscard]] constexpr size_t size() const override {
                return 6;
            }
        };

        struct ConnectedControllers : PacketData {
            ControllerResponseHead head;
            uint8_t tail = 0;

            void read(utils::reader &reader) override {
                head.read(reader);
                reader.read(tail);
            }

            void write(utils::writer &writer) const override {
                head.write(writer);
                writer.write(tail);
            }

            void swap_member_endian() override {
                head.swap_member_endian();
            }

            [[nodiscard]] constexpr size_t size() const override {
                return 12;
            }
        };

        using namespace std::chrono_literals;

        struct ControllerData : PacketData {
            ControllerResponseHead beginning;
            bool connected{};
            uint32_t packet_number{};
            ButtonGroup1 button_mask_1{};
            ButtonGroup2 button_mask_2{};
            bool home_button{};
            bool touch_button{};
            struct {
                uint8_t x;
                uint8_t y;
            } l_stick{};
            struct {
                uint8_t x;
                uint8_t y;
            } r_stick{};
            struct {
                uint8_t left;
                uint8_t down;
                uint8_t right;
                uint8_t up;
            } analog_dp{};
            struct {
                uint8_t y;
                uint8_t b;
                uint8_t a;
                uint8_t x;
            } analog_face{};
            struct {
                uint8_t l;
                uint8_t r;
            } analog_bumper{};
            struct {
                uint8_t l;
                uint8_t r;
            } analog_trigger{};
            TouchData first_touch;
            TouchData second_touch;
            uint64_t motion_data_timestamp_usec;
            struct {
                float x;
                float y;
                float z;
            } accelerometer{};
            struct {
                float pitch;
                float yaw;
                float roll;
            } gyroscope{};

            void swap_member_endian() override {
                beginning.swap_member_endian();
                packet_number = SwapEndian(packet_number);
                first_touch.swap_member_endian();
                second_touch.swap_member_endian();
                motion_data_timestamp_usec = SwapEndian(motion_data_timestamp_usec);
                accelerometer.x = SwapEndian(accelerometer.x);
                accelerometer.y = SwapEndian(accelerometer.y);
                accelerometer.z = SwapEndian(accelerometer.z);
                gyroscope.pitch = SwapEndian(gyroscope.pitch);
                gyroscope.yaw = SwapEndian(gyroscope.yaw);
                gyroscope.roll = SwapEndian(gyroscope.roll);
            }

            ControllerData()
                    : motion_data_timestamp_usec(std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count()) {}

            void read(utils::reader &reader) override {}

            void write(utils::writer &writer) const override {
                beginning.write(writer);
                writer.write(connected);
                writer.write(packet_number);
                writer.write(button_mask_1);
                writer.write(button_mask_2);
                writer.write(home_button);
                writer.write(touch_button);
                writer.write(l_stick);
                writer.write(r_stick);
                writer.write(analog_dp);
                writer.write(analog_face);
                writer.write(analog_bumper);
                writer.write(analog_trigger);
                writer.write(motion_data_timestamp_usec);
                first_touch.write(writer);
                second_touch.write(writer);
                writer.write(accelerometer);
                writer.write(gyroscope);
            }

            [[nodiscard]] constexpr size_t size() const override {
                return 80;
            }
        };


        struct VersionInfo : PacketData {
            uint16_t max_protocol_version{};

            void swap_member_endian() override {
                max_protocol_version = SwapEndian(max_protocol_version);
            }

            void read(utils::reader &reader) override {
                reader.read(max_protocol_version);
            }

            void write(utils::writer &writer) const override {
                writer.write<uint16_t>(max_protocol_version);
            }

            [[nodiscard]] constexpr size_t size() const override {
                return 2;
            }
        };
        struct OutgoingPacket {
            utils::writer m_writer;
            explicit OutgoingPacket(uint8_t* data, size_t size)
            : m_writer(data, size) {}
            // Header should always be added first
            void add_data(PacketData& data){

                data.swap_member_endian();
                data.write(m_writer);

                const auto posCur = m_writer.pos();

                m_writer.seek(6);

                m_writer.write<uint16_t>(posCur - 20);

                m_writer.seek(posCur);
            }
            void set_crc32(){
                const auto posCur = m_writer.pos();

                m_writer.seek(8);
                m_writer.write<uint32_t>(0); // 0 out the crc32

                m_writer.seek(posCur);

                auto crc = utils::crc(m_writer.begin(), m_writer.begin() + m_writer.pos());
                m_writer.write<uint32_t>(crc);

                m_writer.seek(posCur);
            }

            [[nodiscard]] auto cursor() const {
                return m_writer.pos();
            }
            [[nodiscard]] auto begin() const {
                return m_writer.begin();
            }
            [[nodiscard]] auto end() const {
                return m_writer.end();
            }
        };
    }
}