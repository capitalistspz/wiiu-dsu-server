#pragma once

#include <cstddef>
#include <stdexcept>

namespace utils {
    template <typename T>
    class h_array {
        T* m_data;
        size_t m_length;
    private:
        h_array()
                : m_data(nullptr), m_length(0){}
    public:
        explicit h_array(size_t size)
        : m_data(new T[size]), m_length(size){}

        template <size_t Size>
        explicit h_array(const T start[Size])
        : m_data(new T[Size]), m_length(Size) {
            std::copy(start, start + Size, m_data);
        }

        explicit h_array(const T* start, size_t size)
                : m_data(new T[size]), m_length(size) {
            std::copy(start, start + size, m_data);
        }

        static h_array move_construct(T*& start, size_t size){
            h_array arr;
            arr.m_data = start;
            start = nullptr;
            arr.m_length = size;
        }
        T operator[](size_t index) const{
            if (index > m_length)
                throw std::out_of_range("index");
            return m_data[index];
        }
        T& operator[](size_t index){
            if (index > m_length)
                throw std::out_of_range("index");
            return m_data[index];
        }

        T* begin(){
            return m_data;
        }
        T* end(){
            return m_data + m_length;
        }

        const T* cbegin() const {
            return m_data;
        }
        const T* cend() const {
            return m_data + m_length;
        }
        [[nodiscard]] size_t size() const{
            return m_length;
        }
        ~h_array(){
            delete[] m_data;
        }
    };
}