#include "CircularBuffer.h"
#include "Particle.h"

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
        itemSizeBytes = itemSize;
        full = false;


    }

    void CircularBuffer::printDebugInfo(bool verbose){

        if(buffer == NULL) Log.error("Circular Buffer malloc failed!");
        if(itemPtrs == NULL) Log.error("Circular Buffer malloc failed!");

        if(verbose){
            Log.info("Circular buffer with %d items of %d bytes",maxNumItems,itemSizeBytes);
            Log.info("Circ buffer mem location: %x",buffer);
            for(int i=0; i<maxNumItems;i++){
                Log.info("Circ Buff pointer %d : %x",i,itemPtrs[i]);
            }

        }

        Log.info("Read: %d Write: %d numItems: %d full: %d",readIdx,writeIdx,currNumItems,full);

    }

    //Returns the read pointer and moves it forward one item. Essentially reading an item from the buffer.
    uint8_t* CircularBuffer::getReadPtr(){

        //Buffer is empty.
        if(readIdx == writeIdx && !full) return NULL;

        uint8_t temp = readIdx;
        readIdx= (readIdx+1)%maxNumItems;

        SINGLE_THREADED_BLOCK(){
        if(writeIdx>=readIdx){
             currNumItems = writeIdx-readIdx;
        }
        else{
            currNumItems = maxNumItems-readIdx+writeIdx;
        }

        full = false;
        }
        return itemPtrs[temp];
    }

    //Returns the write pointer and moves it forward one item. Essentially writing an item to the buffer.
    uint8_t* CircularBuffer::getWritePtr(){

        uint8_t temp = writeIdx;
        writeIdx = (writeIdx+1)%maxNumItems;

        SINGLE_THREADED_BLOCK(){
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
        }
        Log.info("Write to circ buff. % items",currNumItems);
        return itemPtrs[temp];
    }

    //Returns how many items are in the buffer.
    uint8_t CircularBuffer::getCurrNumItems(){
        return currNumItems;
    }

    uint16_t CircularBuffer::getItemSize(){
        return itemSizeBytes;
    }