#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>

 class CircularBuffer{

    public:

    CircularBuffer(uint16_t itemSize, uint8_t numItems);

    //Returns the read pointer and moves it forward one item. Essentially reading an item from the buffer.
    uint8_t* getReadPtr();

    //Returns the write pointer and moves it forward one item. Essentially writing an item to the buffer.
    uint8_t* getWritePtr();

    //Returns how many items are in the buffer.
    uint8_t getCurrNumItems();

    private:

    uint8_t * buffer;   //Raw memory for the whole buffer.
    uint8_t ** itemPtrs;//Pointers to the  location of each item.

    uint8_t readIdx;    //Current index of the read pointer.
    uint8_t writeIdx;   //Current index of the write pointer.

    uint8_t currNumItems;
    uint8_t maxNumItems;

    bool full;

 };   

#endif