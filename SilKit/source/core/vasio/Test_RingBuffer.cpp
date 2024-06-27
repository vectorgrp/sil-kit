// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <random>

#include "RingBuffer.hpp"

#include "gtest/gtest.h"

using namespace SilKit::Core;

static std::mt19937 generator{0}; // constant seed for deterministic behaviour 

std::vector<std::vector<uint8_t> > GenerateDataBlocks(const size_t maxSize, const size_t numDataBlocks)
{    
    std::vector<std::vector<uint8_t> > dataBlocks;

    // configure random number generators
    std::uniform_int_distribution<size_t> distrSize(1, maxSize);
    std::uniform_int_distribution<int> distrValue(0, std::numeric_limits<uint8_t>::max());

    for (size_t i = 0; i < numDataBlocks; i++)
    {
        // random size
        std::vector<uint8_t> block(distrSize(generator));

        // random values
        for (auto& elem : block)
        {
            elem = static_cast<uint8_t>(distrValue(generator));
        }

        dataBlocks.push_back(block);
    }

    return dataBlocks;
}

// mimic use of ring buffer in VAsioPeer
void WriteViaDirectMemoryAccess(RingBuffer<uint8_t>& ringBuffer, const std::vector<uint8_t>& dataBlock)
{
    auto arrayOne = ringBuffer.GetArrayOne();
    size_t numBytesForArrayOne = std::min(arrayOne.second, dataBlock.size());
    std::memcpy(arrayOne.first, dataBlock.data(), numBytesForArrayOne);

    if (numBytesForArrayOne != dataBlock.size())
    {
        auto arrayTwo = ringBuffer.GetArrayTwo();
        std::memcpy(arrayTwo.first, dataBlock.data() + numBytesForArrayOne, dataBlock.size() - numBytesForArrayOne);
    }

    ringBuffer.AdvanceWPos(dataBlock.size());
}

// alternating write and read, write via dedicated function
TEST(Test_RingBuffer, writeStandard)
{
    const size_t capacity{1000};
    RingBuffer<uint8_t> ringBuffer(capacity);

    const size_t numDataBlocks{500};
    auto dataBlocks = GenerateDataBlocks(capacity, numDataBlocks);

    // write to ring buffer - read from ring buffer - compare
    for (const auto& elem : dataBlocks)
    {
        ringBuffer.Write(elem);

        std::vector<uint8_t> readData(ringBuffer.Size());
        ringBuffer.Read(readData);        

        ASSERT_EQ(elem, readData);
    }
}

// alternating write and read, write via direct memory access
TEST(Test_RingBuffer, writeDirectMemoryAccess)
{
    const size_t capacity{1000};
    RingBuffer<uint8_t> ringBuffer(capacity);

    const size_t numDataBlocks{500};
    auto dataBlocks = GenerateDataBlocks(capacity, numDataBlocks);

    for (const auto& elem : dataBlocks)
    {
        WriteViaDirectMemoryAccess(ringBuffer, elem);

        std::vector<uint8_t> readData(ringBuffer.Size());
        ringBuffer.Read(readData);

        ASSERT_EQ(elem, readData);
    }
}

// resizing of ring buffer (via SetCapacity)
TEST(Test_RingBuffer, resizeRingBuffer)
{
    const size_t capacity{1000};
    RingBuffer<uint8_t> ringBuffer(capacity);

    const size_t numDataBlocks{500};
    std::vector<size_t> blockSizes{2000, 3000, 4000, 5000};

    for (const auto& maxSize : blockSizes)
    {
        auto dataBlocks = GenerateDataBlocks(maxSize, numDataBlocks);

        for (const auto& elem : dataBlocks)
        {
            // resize if necessary
            if (elem.size() > ringBuffer.Capacity())
            {
                ringBuffer.SetCapacity(elem.size());
            }
            WriteViaDirectMemoryAccess(ringBuffer, elem);

            std::vector<uint8_t> readData(ringBuffer.Size());
            ringBuffer.Read(readData);

            ASSERT_EQ(elem, readData);
        }    
    }
}

// write and read of multiple data blocks at once (resizing capacity allowed)
TEST(Test_RingBuffer, writeMultiple_resizeAllowed)
{
    const size_t capacity{1000};
    RingBuffer<uint8_t> ringBuffer(capacity);

    const size_t numBlockSets = 100;
    const size_t numDataBlocks = 500; // same number of data blocks for every group

    std::vector<uint8_t> currentDataBlock;

    for (size_t i = 0; i < numBlockSets; i++)
    {
        auto dataBlocks = GenerateDataBlocks(capacity, numDataBlocks);

        // write all data blocks at once (resize capacity if necessary)
        for (const auto& elem : dataBlocks)
        {
            auto remainingSpace = ringBuffer.Capacity() - ringBuffer.Size(); 
            if (elem.size() > remainingSpace)
            {
                ringBuffer.SetCapacity(ringBuffer.Size() + elem.size());
            }

            WriteViaDirectMemoryAccess(ringBuffer, elem);

            // append current data block for comparison
            currentDataBlock.insert(currentDataBlock.end(), elem.begin(), elem.end());            
        }

        // read all data available 
        std::vector<uint8_t> readData(ringBuffer.Size());
        ringBuffer.Read(readData);

        ASSERT_EQ(currentDataBlock, readData);

        currentDataBlock.clear();
    }
}

// write and read of multiple data blocks at once (fixed capacity)
TEST(Test_RingBuffer, writeMultiple_fixedCapacity) 
{
    const size_t capacity{1000};
    RingBuffer<uint8_t> ringBuffer(capacity);

    const size_t numBlockSets = 100;
    const size_t numDataBlocks = 500; // same number of data blocks for every group
    const size_t maxSizeDataBlock = 200; // size smaller than capacity reasonable in this case

    std::vector<uint8_t> currentDataBlock;

    for (size_t i = 0; i < numBlockSets; i++)
    {
        auto dataBlocks = GenerateDataBlocks(maxSizeDataBlock, numDataBlocks);

        // iterate until all data blocks are consumed
        while (dataBlocks.size() != 0)
        {
            size_t numWrittenBlocks{0};

            // write as many data blocks as possible
            for (const auto& elem : dataBlocks)
            {
                auto remainingSpace = ringBuffer.Capacity() - ringBuffer.Size();
                
                if (elem.size() <= remainingSpace) // write current block, if there is enough space
                {
                    WriteViaDirectMemoryAccess(ringBuffer, elem);

                    // append current data block for comparison
                    currentDataBlock.insert(currentDataBlock.end(), elem.begin(), elem.end());

                    numWrittenBlocks++;
                }
                else // stop writing
                {
                    break;
                }                
            }

            dataBlocks.erase(dataBlocks.begin(), dataBlocks.begin() + numWrittenBlocks);

            // read all data
            std::vector<uint8_t> readData(ringBuffer.Size());
            ringBuffer.Read(readData);

            ASSERT_EQ(currentDataBlock, readData);

            currentDataBlock.clear();
        }
    }
}