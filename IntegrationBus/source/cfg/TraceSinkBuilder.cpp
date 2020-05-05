#include "TraceSinkBuilder.hpp"


namespace ib {
namespace cfg {

TraceSinkBuilder::TraceSinkBuilder(ParticipantBuilder* participant, std::string name)
    : ParentBuilder<ParticipantBuilder>{participant}
{
	_traceSink.name = std::move(name);
}

TraceSinkBuilder::~TraceSinkBuilder() = default;

auto TraceSinkBuilder::operator->() -> TraceSinkBuilder*
{
	return this;
}

auto TraceSinkBuilder::Build() -> TraceSink
{
	return std::move(_traceSink);
}

auto TraceSinkBuilder::WithType(TraceSink::Type type) -> TraceSinkBuilder&
{
	_traceSink.type = type;
	return *this;
}

auto TraceSinkBuilder::WithOutputPath(std::string outputPath) -> TraceSinkBuilder&
{
	_traceSink.outputPath = outputPath;
	return *this;
}

auto TraceSinkBuilder::Enabled(bool isEnabled) -> TraceSinkBuilder&
{
	_traceSink.enabled = isEnabled;
	return *this;
}

} // namespace cfg
} // namespace ib

