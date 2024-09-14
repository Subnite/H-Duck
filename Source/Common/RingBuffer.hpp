#pragma once
#include <stddef.h>
#include <string>
#include <iostream>
#include <vector>

template<typename T>
class RingBuffer{
    unsigned int m_startIndex{};
    unsigned int m_relativeSize{};
    std::vector<T> m_buffer;
    size_t m_size;

public:
    RingBuffer(size_t maxBufferSize)
    :   m_relativeSize(static_cast<int>(maxBufferSize)), m_buffer(), m_size(maxBufferSize)
    { 
        m_buffer.reserve(maxBufferSize);
        m_buffer.resize(maxBufferSize);
    }
    ~RingBuffer(){
    }

    // changes the relative pivot of the buffer
    void setStartIndex(int index){
        m_startIndex = index;
    }

    // sets the relative size from the relative start index onward
    void setRelativeSize(int size, bool ignoreAndUseEnd = false){
        if (ignoreAndUseEnd){
            m_relativeSize = static_cast<unsigned int>(capacity());
        } else {
            if (size <= m_size){
                m_relativeSize = size;
            } else { // not in range
                m_relativeSize = static_cast<unsigned int>(capacity());
                std::cout << "Ringbuffer size: '" << size << "' not valid, using max size now: '" << m_relativeSize << "'\n";
            }
        }
    }

    // returns the value stored within a relative index
    T getFromRelativeIndex(unsigned int index) const {
        int realIndex;
        if (index < m_relativeSize){
            realIndex = (m_startIndex + index) % capacity();
        }else{ // safety
            realIndex = m_startIndex;
        }

        return m_buffer[realIndex];
    }

    T* getRefFromRelativeIndex(unsigned int index) {
        int realIndex;
        if (index < m_relativeSize){
            realIndex = (m_startIndex + index) % capacity();
        }else{ // safety
            realIndex = m_startIndex;
        }

        return &m_buffer[realIndex];
    }

    // inputs the value at relative index
    void setAtRelativeIndex(int index, T value) {
        int realIndex;
        if (static_cast<unsigned int>(index) < m_relativeSize){
            realIndex = (m_startIndex + index) % capacity();
        } else { // safety
            realIndex = m_startIndex;
        }

        m_buffer[realIndex] = value;
    }

    // returns the absolute size / capacity
    size_t capacity() const {
        return m_size;
    }

    // returns the "relative" size
    size_t size() const {
        return m_relativeSize;
    }

    // will return the value at the last index and override it with the new value,
    // shuffles the buffer counter clockwise
    T insertAndPop(T value) {
        T previousValue = getFromRelativeIndex(m_relativeSize-1);

        if (m_startIndex < 1){
            m_startIndex = static_cast<unsigned int>(capacity()-1);
        } else {
            m_startIndex -= 1;
        }

        setAtRelativeIndex(0, value);

        return previousValue;
    }

    void fillAbsolute(T* samples, int size){
        if (size > m_size){
            return;
        }

        for (int i = 0; i < size; i++){
            m_buffer[i] = samples[i];
        }
    }

    void fillRelative(T* samples, int size){
        if (size > m_size){
            return;
        }

        for (int i = 0; i < size; i++){
            setAtRelativeIndex(i, samples[i]);
        }
    }

    void printBuffer(bool isRelative = true) {
        std::cout << "[";
        if (isRelative){
            for (int i{}; i < size(); i++){
                std::cout << getFromRelativeIndex(i);
                if (i < size() - 1)
                    std::cout <<", ";

            }
        } else {
            for (int i{}; i < capacity(); i++){
                std::cout << m_buffer[i];
                if (i < capacity() - 1)
                    std::cout <<", ";
            }
        }

        std::cout << "]\n";
    }
};
