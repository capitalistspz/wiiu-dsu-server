#pragma once
#include <concepts>
#include <type_traits>
#include <ranges>
#include <cstring>
#include <iterator>
#include <vector>

namespace utils {
    class reader {
        uint8_t* m_data{};
        size_t m_cursor;
        size_t m_end;
    public:
        explicit reader(uint8_t* container1, size_t size, size_t offset = 0 )
        : m_data(container1), m_cursor(0), m_end(size){
        }
        template <typename T> requires (!std::is_pointer_v<T>)
        void read(T& value){
            memcpy(&value, m_data + m_cursor, sizeof(value));
            m_cursor += sizeof(value);
        }

        void read(std::vector<uint8_t>& out_container, size_t size, size_t offset = 0){
            std::copy(m_data + m_cursor, m_data + m_cursor + size, out_container.begin() + offset);
        }

        void read(uint8_t* out_container, size_t size, size_t offset = 0){
            std::copy(m_data + m_cursor, m_data + m_cursor + size, out_container + offset);
        }
        void read(void* out_container, size_t size, size_t offset = 0){
            std::memcpy(reinterpret_cast<uint8_t*>(out_container) + offset, m_data + m_cursor, size);
        }

        /**
         * Move the reader cursor to the location
         * @param pos the position to seek to
         * @return whether if successfully seeked
         */
        bool seek(size_t pos){
            if (pos < m_end){
                m_cursor = pos;
                return true;
            }
            return false;
        };

        [[nodiscard]] auto pos() const{
            return m_cursor;
        }

        [[nodiscard]] auto size() const{
            return m_end;
        }


    };
}