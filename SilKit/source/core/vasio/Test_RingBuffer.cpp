// SPDX-FileCopyrightText: 2023 Vector Informatik GmbH
//
// SPDX-License-Identifier: MIT

#include <random>
#include <cstring>

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

// mimic use of ring buffer in VAsioPeer (writing into ring buffer)
void Write(RingBuffer& ringBuffer, const std::vector<uint8_t>& dataBlock)
{
    std::vector<MutableBuffer> bufferArrays;
    ringBuffer.GetWritingBuffers(bufferArrays);

    if (bufferArrays.empty())
    {
        return;
    }

    auto arrayOne = bufferArrays.front();
    size_t numBytesForArrayOne = std::min(arrayOne.GetSize(), dataBlock.size());
    memcpy(arrayOne.GetData(), dataBlock.data(), numBytesForArrayOne);

    if (bufferArrays.size() > 1)
    {
        auto arrayTwo = bufferArrays.back();
        memcpy(arrayTwo.GetData(), dataBlock.data() + numBytesForArrayOne, dataBlock.size() - numBytesForArrayOne);
    }

    ringBuffer.AdvanceWPos(dataBlock.size());
}

// alternating write and read
TEST(Test_RingBuffer, writeSingle)
{
    const size_t capacity{1000};
    RingBuffer ringBuffer(capacity);

    const size_t numDataBlocks{500};
    auto dataBlocks = GenerateDataBlocks(capacity, numDataBlocks);

    for (const auto& elem : dataBlocks)
    {
        Write(ringBuffer, elem);

        std::vector<uint8_t> readData(ringBuffer.Size());
        ringBuffer.Read(readData);

        ASSERT_EQ(elem, readData);
    }
}

// resizing of ring buffer (via Reserve)
TEST(Test_RingBuffer, resizeRingBuffer)
{
    const size_t capacity{1000};
    RingBuffer ringBuffer(capacity);

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
                ringBuffer.Reserve(elem.size());
            }
            Write(ringBuffer, elem);

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
    RingBuffer ringBuffer(capacity);

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
                ringBuffer.Reserve(ringBuffer.Size() + elem.size());
            }

            Write(ringBuffer, elem);

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
    RingBuffer ringBuffer(capacity);

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
                    Write(ringBuffer, elem);

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