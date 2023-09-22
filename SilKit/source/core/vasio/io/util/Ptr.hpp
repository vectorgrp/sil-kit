#pragma once


#include <functional>
#include <memory>
#include <type_traits>


namespace VSilKit {


template <typename T>
using Ptr = std::unique_ptr<T, std::function<void(void*)>>;


template <typename T, typename U, typename Deleter, typename = std::enable_if_t<std::is_convertible<U*, T*>::value>>
auto IntoPtr(std::unique_ptr<U, Deleter> ptr) -> Ptr<T>
{
    auto typeErasedDeleter = [deleter = std::move(ptr.get_deleter())](void* p) {
        deleter(static_cast<U*>(static_cast<T*>(p)));
    };
    return Ptr<T>{static_cast<T*>(ptr.release()), std::move(typeErasedDeleter)};
}


template <typename T, typename... Args>
auto MakePtr(Args&&... args) -> Ptr<T>
{
    return IntoPtr<T>(std::make_unique<T>(std::forward<Args>(args)...));
}


template <typename T, typename U, typename = std::enable_if_t<std::is_convertible<U&, T&>::value>>
auto MakeViewPtr(U& ref) -> Ptr<T>
{
    return Ptr<T>{std::addressof(ref), [](void*) {}};
}


} // namespace VSilKit


namespace SilKit {
namespace Core {
using VSilKit::Ptr;
using VSilKit::IntoPtr;
using VSilKit::MakePtr;
using VSilKit::MakeViewPtr;
} // namespace Core
} // namespace SilKit
