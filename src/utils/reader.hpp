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
        /**
         * Takes a view over the data to write
         * @param container1 the buffer to be read from to
         * @param size the capacity of the buffer to be read from
         * @param offset the position to start from within the buffer
         * */
        explicit reader(uint8_t* container1, size_t size, size_t offset = 0 )
        : m_data(container1), m_cursor(0), m_end(size){
        }
        template <typename T> requires (!std::is_pointer_v<T>)
        /**
         *
         * @tparam T type to be gotten from buffer
         * @param value value to be read from the reader
         */
        void read(T& value){
            memcpy(&value, m_data + m_cursor, sizeof(value));
            m_cursor += sizeof(value);
        }

        template <typename T> requires (!std::is_pointer_v<T>)
        /**
         *
         * @tparam T type to be gotten from the buffer
         * @param value value to be read from the writer
         */
        void read(T&& value){
            memcpy(&value, m_data + m_cursor, sizeof(value));
            m_cursor += sizeof(value);
        }

        /**
         *
         * @param outBuffer the vector for the data to be copied to
         * @param size number of bytes to copy into the vector
         * @param offset start position to within the buffer
         */
        void read(std::vector<uint8_t>& out_container, size_t size, size_t offset = 0){
            std::copy(m_data + m_cursor, m_data + m_cursor + size, out_container.begin() + offset);
        }

        /**
         *
         * @param outBuffer the buffer for the data to be copied into
         * @param size number of bytes to copy into the buffer
         * @param offset start position to within the buffer
         */
        void read(uint8_t* out_container, size_t size, size_t offset = 0){
            std::copy(m_data + m_cursor, m_data + m_cursor + size, out_container + offset);
        }

        /**
         *
         * @param src pointer to the beginning of some memory
         * @param size number of bytes to copy to that location
         * @param offset number of bytes offset from the pointer to copy to
         */
        void read(void* out_container, size_t size, size_t offset = 0){
            std::memcpy(reinterpret_cast<uint8_t*>(out_container) + offset, m_data + m_cursor, size);
        }

        /**
         * Move the reader cursor to the location
         * @param pos the position to seek to
         * @return whether the position was successfully seeked
         */
        bool seek(size_t pos){
            if (pos < m_end){
                m_cursor = pos;
                return true;
            }
            return false;
        };

        /**
         * Move the writer cursor to the location
         * @return the position of the cursor
         */
        [[nodiscard]] auto pos() const{
            return m_cursor;
        }

        /**
        *
        * @return max space that can be taken by this writer
        */
        [[nodiscard]] auto capacity() const{
            return m_end;
        }

        /**
         *
         * @return pointer to the start of the data
         */
        [[nodiscard]] uint8_t* begin() const {
            return m_data;
        }

        /**
         *
         * @return pointer to cursor position
         */
        [[nodiscard]] uint8_t* end() const {
            return m_data + m_cursor;
        }

    };
}