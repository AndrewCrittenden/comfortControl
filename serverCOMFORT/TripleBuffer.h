

#ifndef TRIPLEBUFFER_H
#define TRIPLEBUFFER_H
#include <atomic>

template <typename T>
class TripleBuffer
{
public:
    // both constructors set the first buffer to public, second buffer to intermediary, and third buffer to private by default,
    // and sets the switching semaphore and read flag to false
    // this constructor initializes the buffers with the default value of the type parameter
    TripleBuffer();

    // this constructor initializes the buffers with the given values
    TripleBuffer(T firstBuffer, T secondBuffer, T thirdBuffer);

    // this will get the value of the public buffer
    T GetPublicBuffer();

    // this will set the value of the public buffer
    void SetPublicBuffer(T newValue);

    // this will get the value of the private buffer
    T GetPrivateBuffer();

    // this will set the value of the private buffer
    void SetPrivateBuffer(T newValue);

    // this will switch the private and intermediary buffers
    void RotatePrivateBuffer();

private:
    // this will rotate the public buffer, if the public buffer is not the most recent buffer
    void tryRotatePublicBuffer();

    // this function is just to avoid having to make an integer variable just to pass it to the atomic compare exchange function
    bool compare_exchange_helper(volatile std::atomic<int>& obj, int expected, int val);

    T buffers[3]; // space where the buffers is stored
    int publicBuffer; // a number indicating which of the buffers is currently the public buffer
    int nextPublicBuffer; // a number indicating which of the buffers is currently the intermediate buffer
    int privateBuffer; // a number indicating which of the buffers is currently the private buffer
    std::atomic<int> switchingBuffers; // a semaphore indicating whether the buffers are currently being rotated
    std::atomic<int> nextBufferRead; // an atomic flag indicating whether the current public buffer contains the most recent information 
};

#include "TripleBuffer.cpp"

#endif /* TRIPLEBUFFER_H */