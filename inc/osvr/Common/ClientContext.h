/** @file
    @brief Header

    @date 2014

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A
#define INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A

// Internal Includes
#include <osvr/Common/Export.h>
#include <osvr/Common/ClientContext_fwd.h>
#include <osvr/Common/ClientInterfacePtr.h>
#include <osvr/Common/PathTree_fwd.h>
#include <osvr/Util/KeyedOwnershipContainer.h>
#include <osvr/Common/SystemComponent_fwd.h>
#include <osvr/Util/SharedPtr.h>

// Library/third-party includes
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>

// Standard includes
#include <string>
#include <vector>
#include <map>

struct OSVR_ClientContextObject : boost::noncopyable {
  public:
    typedef std::vector<osvr::common::ClientInterfacePtr> InterfaceList;
    /// @brief Destructor
    OSVR_COMMON_EXPORT virtual ~OSVR_ClientContextObject();

    /// @brief System-wide update method.
    OSVR_COMMON_EXPORT void update();

    /// @brief Accessor for app ID
    std::string const &getAppId() const;

    /// @brief Creates an interface object for the given path. The context
    /// retains shared ownership.
    ///
    /// @param path Path to a resource. Should be absolute.
    OSVR_COMMON_EXPORT osvr::common::ClientInterfacePtr
    getInterface(const char path[]);

    /// @brief Searches through this context to determine if the passed
    /// interface object has been retained, and if so, ownership is transferred
    /// to the caller.
    ///
    /// @param iface raw interface pointer (from C, usually)
    ///
    /// @returns Pointer owning the submitted interface object, or an empty
    /// pointer if NULL passed or not found.
    OSVR_COMMON_EXPORT osvr::common::ClientInterfacePtr
    releaseInterface(osvr::common::ClientInterface *iface);

    InterfaceList const &getInterfaces() const { return m_interfaces; }

    /// @brief Sends a JSON route/transform object to the server.
    OSVR_COMMON_EXPORT void sendRoute(std::string const &route);

    /// @brief Gets a string parameter value.
    OSVR_COMMON_EXPORT std::string
    getStringParameter(std::string const &path) const;

    /// @brief Accessor for the path tree.
    OSVR_COMMON_EXPORT osvr::common::PathTree const &getPathTree() const;

    /// @brief Accessor for the system component
    OSVR_COMMON_EXPORT osvr::common::SystemComponent *getSystemComponent();

    /// @brief Pass (smart-pointer) ownership of some object to the client
    /// context.
    template <typename T> void *acquireObject(T obj) {
        return m_ownedObjects.acquire(obj);
    }

    /// @brief Frees some object whose lifetime is controlled by the client
    /// context.
    ///
    /// @returns true if the object was found and released.
    OSVR_COMMON_EXPORT bool releaseObject(void *obj);

  protected:
    /// @brief Constructor for derived class use only.
    OSVR_COMMON_EXPORT OSVR_ClientContextObject(const char appId[]);

  private:
    virtual void m_update() = 0;
    virtual void m_sendRoute(std::string const &route) = 0;
    /// @brief Optional implementation-specific handling of interface retrieval,
    /// before the interface is returned to the client.
    OSVR_COMMON_EXPORT virtual void
    m_handleNewInterface(osvr::common::ClientInterfacePtr const &iface);
    /// @brief Optional implementation-specific handling of interface release,
    /// before the interface is actually freed.
    OSVR_COMMON_EXPORT virtual void
    m_handleReleasingInterface(osvr::common::ClientInterfacePtr const &iface);

    /// @brief Implementation of accessor for the path tree.
    OSVR_COMMON_EXPORT virtual osvr::common::PathTree const &
    m_getPathTree() const = 0;

    /// @brief Implementation of accessor for the path tree.
    OSVR_COMMON_EXPORT virtual osvr::common::SystemComponent *
    m_getSystemComponent() = 0;

    std::string const m_appId;
    InterfaceList m_interfaces;

    osvr::util::MultipleKeyedOwnershipContainer m_ownedObjects;
};

#endif // INCLUDED_ContextImpl_h_GUID_9000C62E_3693_4888_83A2_0D26F4591B6A
