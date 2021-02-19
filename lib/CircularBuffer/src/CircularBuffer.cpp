#include "CircularBuffer.h"


    CircularBuffer::CircularBuffer(uint16_t itemSize, uint8_t numItems){

        buffer =(uint8_t*) malloc(itemSize*numItems);
        itemPtrs = (uint8_t**)malloc(numItems*sizeof(uint8_t*));

        for(int i=0; i<numItems;i++){

            itemPtrs[i] = &buffer[i*itemSize];  //Assing our item pointers to locations in the buffer spaced by itemSize.
        }

        writeIdx = 0;
        readIdx  = 0;
        currNumItems = 0;
        maxNumItems = numItems;

    }

    //Returns the read pointer and moves it forward one item. Essentially reading an item from the buffer.
    uint8_t* CircularBuffer::getReadPtr(){

        //Buffer is empty.
        if(readIdx == writeIdx) return NULL;

        uint8_t temp = readIdx;
        readIdx= (readIdx+1)%maxNumItems;

        if(writeIdx>=readIdx){
             currNumItems = writeIdx-readIdx;
        }
        else{
            currNumItems = maxNumItems-readIdx+writeIdx;
        }

        full = false;
        return itemPtrs[temp];
    }

    //Returns the write pointer and moves it forward one item. Essentially writing an item to the buffer.
    uint8_t* CircularBuffer::getWritePtr(){

        uint8_t temp = writeIdx;
        writeIdx = (writeIdx+1)%maxNumItems;


        if(full){
            readIdx = (readIdx+1) %maxNumItems;//Also move the read index forward, so we read the oldest data.
        }
        else if(readIdx == writeIdx){

            currNumItems = maxNumItems;
            full = true;
        }
        else if(writeIdx>readIdx){
             currNumItems = writeIdx-readIdx;
        }
        else{
            currNumItems = maxNumItems-readIdx+writeIdx;
        }

        
        return itemPtrs[temp];
    }

    //Returns how many items are in the buffer.
    uint8_t CircularBuffer::getCurrNumItems(){
        return currNumItems;
    }