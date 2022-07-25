/* Copyright (c) 2022 Vector Informatik GmbH

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#pragma once

#include "DataMessageDatatypes.hpp"

#include "silkit/util/HandlerId.hpp"

namespace SilKit {
namespace Services {
namespace PubSub {

class IDataSubscriber
{
public:
    virtual ~IDataSubscriber() = default;

    /*! \brief Set the default handler for data reception
     *
     * The handler is executed when data is received from a matching publisher.
     * The default handler will not be invoked if a specific is available.
     */
    virtual void SetDefaultDataMessageHandler(DataMessageHandlerT callback) = 0;

    /*! \brief Register a handler for data received by DataPublishers matching specific criteria
     * 
     * Specific data handlers can be used to distinguish publications on a single topic. If all of the labels provided
     * here appear in the labels of a DataPublisher, publications are redirected to the specific handler. A label key 
     * must match, an empty string in the value of a label given here is a wildcard. Likewise, the 
     * mediaType must match the one given by a DataPublisher. An empty string in the 
     * mediaType provided here also is a wildcard.
     *
     * \return Returns a \ref HandlerId that can be used to remove the callback.
     */
    virtual auto AddExplicitDataMessageHandler(DataMessageHandlerT callback,
                                               const std::string& mediaType,
                                               const std::map<std::string, std::string>& labels) -> HandlerId = 0;

    /*! \brief Remove a explicit DataMessageHandlerT by HandlerId on this controller.
     *
     * \param handlerId Identifier of the callback to be removed. Obtained upon adding to respective handler.
     */
    virtual void RemoveExplicitDataMessageHandler(HandlerId handlerId) = 0;
};

} // namespace PubSub
} // namespace Services
} // namespace SilKit
