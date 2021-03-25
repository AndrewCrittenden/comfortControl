
#ifndef TRIPLEBUFFER_CPP
#define TRIPLEBUFFER_CPP
#include "TripleBuffer.h"

// both constructors set the first buffer to public, second buffer to intermediary, and third buffer to private by default,
// and sets the switching semaphore and read flag to false
// this constructor initializes the buffers with the default value of the type parameter
template <typename T>
TripleBuffer<T>::TripleBuffer() {
    this->buffers[0] = T();
    this->buffers[1] = T();
    this->buffers[2] = T();
    publicBuffer = 1;
    nextPublicBuffer = 2;
    privateBuffer = 3;
    switchingBuffers = 0;
    nextBufferRead = 0;
}

// this constructor initializes the buffers with the given values
template <typename T>
TripleBuffer<T>::TripleBuffer(T firstBuffer, T secondBuffer, T thirdBuffer) {
    this->buffers[0] = firstBuffer;
    this->buffers[1] = secondBuffer;
    this->buffers[2] = thirdBuffer;
    publicBuffer = 0;
    nextPublicBuffer = 1;
    privateBuffer = 2;
    switchingBuffers = 0;
    nextBufferRead = 0;
}

// explicit copy assignment operator, as required by QT
template <typename T>
TripleBuffer<T>& TripleBuffer<T>::operator=(const TripleBuffer<T>& other) {
    if (this != &other) {
        for (int i = 0; i < 3; i++) {
            this->buffers[i] = other.buffers[i];
        }
        this->publicBuffer = other.publicBuffer;
        this->nextPublicBuffer = other.nextPublicBuffer;
        this->privateBuffer = other.privateBuffer;
        this->switchingBuffers = other.switchingBuffers.load();
        this->nextBufferRead = other.nextBufferRead.load();
    }
    return *this;
}

// this will get the value of the public buffer
template <typename T>
T TripleBuffer<T>::GetPublicBuffer() {
    tryRotatePublicBuffer();
    return buffers[publicBuffer];
}

// this will set the value of the public buffer
template <typename T>
void TripleBuffer<T>::SetPublicBuffer(T newValue) {
    while (!compare_exchange_helper(switchingBuffers, 0, 1));
    buffers[publicBuffer] = newValue;
    nextBufferRead = 1;
    switchingBuffers = 0;
}

// this will get the value of the private buffer
template <typename T>
T TripleBuffer<T>::GetPrivateBuffer() {
    return buffers[privateBuffer];
}

// this will set the value of the private buffer
template <typename T>
void TripleBuffer<T>::SetPrivateBuffer(T newValue) {
    buffers[privateBuffer] = newValue;
}

// this will switch the private and intermediary buffers
template <typename T>
void TripleBuffer<T>::RotatePrivateBuffer() {
    while (!compare_exchange_helper(switchingBuffers, 0, 1));
    int nextPrivateBuffer = nextPublicBuffer;
    nextPublicBuffer = privateBuffer;
    privateBuffer = nextPrivateBuffer;
    nextBufferRead = 0;
    switchingBuffers = 0;
}

// this will rotate the public buffer, if the public buffer is not the most recent buffer
template <typename T>
void TripleBuffer<T>::tryRotatePublicBuffer() {
    if (compare_exchange_helper(nextBufferRead, 0, 1)) {
        while (!compare_exchange_helper(switchingBuffers, 0, 1));
        int nextPrivateBuffer = publicBuffer;
        publicBuffer = nextPublicBuffer;
        nextPublicBuffer = nextPrivateBuffer;
        nextBufferRead = 1;
        switchingBuffers = 0;
    }
}

// this function is just to avoid having to make an integer variable just to pass it to the atomic compare exchange function
template <typename T>
bool TripleBuffer<T>::compare_exchange_helper(volatile std::atomic<int>& obj, int expected, int val) {
    return atomic_compare_exchange_strong(&obj, &expected, val);
}

#endif /* TRIPLEBUFFER_CPP */
