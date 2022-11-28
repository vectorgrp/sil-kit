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

#include <functional>

#include "oatpp/core/async/Executor.hpp"
#include "oatpp/web/protocol/http/incoming/Response.hpp"

#include "silkit/services/logging/ILogger.hpp"
#include "ILogger.hpp"

#include "SimulationCreationResponseDto.hpp"

namespace SilKit {
namespace Dashboard {

class CreateSimulationCoroutine : public oatpp::async::Coroutine<CreateSimulationCoroutine>
{
private:
    typedef oatpp::web::protocol::http::incoming::Response Response;

public:
    CreateSimulationCoroutine(
        Services::Logging::ILogger* logger, std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper,
        std::function<oatpp::async::AbstractCoroutineWithResult<const std::shared_ptr<Response>&>::StarterForResult()>
            act,
        std::function<void(oatpp::Object<SimulationCreationResponseDto>)> onBody)
        : _logger(logger)
        , _objectMapper(objectMapper)
        , _act(act)
        , _onBody(onBody)
    {
    }
    ~CreateSimulationCoroutine() {}

public:
    Action act() override { return _act().callbackTo(&CreateSimulationCoroutine::onResponse); }

    Action onResponse(const std::shared_ptr<Response>& response)
    {
        if (response->getStatusCode() != 201)
        {
            Services::Logging::Error(_logger, "Dashboard: creating simulation failed: {}", response->getStatusCode());
            _onBody(nullptr);
            return finish();
        }

        return response->readBodyToDtoAsync<oatpp::Object<SimulationCreationResponseDto>>(_objectMapper)
            .callbackTo(&CreateSimulationCoroutine::onBody);
    }

    Action onBody(const oatpp::Object<SimulationCreationResponseDto>& body)
    {
        Services ::Logging::Debug(_logger, "Dashboard: created simulation with id {}", body->id);
        _onBody(body);
        return finish();
    }

private:
    Services::Logging::ILogger* _logger;
    std::shared_ptr<oatpp::data::mapping::ObjectMapper> _objectMapper;
    std::function<oatpp::async::AbstractCoroutineWithResult<const std::shared_ptr<Response>&>::StarterForResult()> _act;
    std::function<void(oatpp::Object<SimulationCreationResponseDto>)> _onBody;
};

} // namespace Dashboard
} // namespace SilKit
