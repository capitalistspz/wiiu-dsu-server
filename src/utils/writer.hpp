#pragma once

#include <concepts>
#include <type_traits>
#include <ranges>
#include <cstring>
#include <iterator>
#include <vector>

namespace utils {
    class writer {
        uint8_t* m_data{};
        size_t m_cursor;
        size_t m_end;
    public:
        /**
         * Takes a view over the data to write
         * @param container1 the buffer to be written to
         * @param size the capacity of the buffer to be written to
         * @param offset the position to start from within the buffer
         * */
        explicit writer(uint8_t* container1, size_t size, size_t offset = 0)
        : m_data(container1 + offset), m_cursor(0), m_end(size){}

        template <typename T> requires (!std::is_pointer_v<T>)
        /**
         *
         * @tparam T type to be written into the buffer
         * @param value value to be read into the writer
         */
        void write(const T& value){
            memcpy(m_data + m_cursor, &value, sizeof(value));
            m_cursor += sizeof(value);
        }

        /**
         *
         * @param outBuffer the vector for the data to be written to
         * @param size number of bytes to read to the buffer
         * @param offset start position to within the buffer
         */
        void write(const std::vector<uint8_t>& out_container, size_t size, size_t offset = 0){
            std::copy(out_container.begin() + offset,  out_container.begin() + offset + size, m_data + m_cursor);

        }
        /**
         *
         * @param outBuffer the buffer for the data to be copied from
         * @param size number of bytes to read from the buffer
         * @param offset start position to within the buffer
         */
        void write(const uint8_t* outBuffer, size_t size, size_t offset = 0){
            std::copy(outBuffer + offset, outBuffer + offset + size, m_data + m_cursor);

        }

        /**
         *
         * @param src pointer to the beginning of some memory
         * @param size number of bytes to copy
         * @param offset number of bytes offset from the pointer to copy from
         */
        void write(const void* src, size_t size, size_t offset = 0){
            const auto dup = const_cast<void*>(src);
            std::memcpy(m_data + m_cursor, reinterpret_cast<uint8_t*>(dup) + offset, size);
        }

        /**
         * Move the writer cursor to the location
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
        [[nodiscard]] auto pos() const {
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