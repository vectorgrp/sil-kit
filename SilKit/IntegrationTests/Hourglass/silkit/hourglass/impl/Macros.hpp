#pragma once

#define SILKIT_HOURGLASS_THROW_NOT_IMPLEMENTED(PREFIX, FUNCTION_NAME) \
    throw SilKitError \
    { \
        "SilKit::Hourglass::" PREFIX "::" FUNCTION_NAME " not implemented" \
    }

#define SILKIT_HOURGLASS_IMPL_THROW_NOT_IMPLEMENTED(PREFIX, FUNCTION_NAME) \
    throw SilKitError \
    { \
        "SilKit::Hourglass::Impl::" PREFIX "::" FUNCTION_NAME " not implemented" \
    }
