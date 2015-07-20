/** @file
    @brief Header

    @date 2015

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2015 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDED_RegisteredStringMap_h_GUID_066235BE_3687_44AD_C2A4_593D6E6780F3
#define INCLUDED_RegisteredStringMap_h_GUID_066235BE_3687_44AD_C2A4_593D6E6780F3

// Internal Includes
#include <osvr/Common/Export.h>

// Library/third-party includes
#include <json/value.h>

// Standard includes
#include <string>
#include <stdint.h>
#include <vector>

namespace osvr {
namespace common {

#define MAX_REGISTRY_SIZE 100

    typedef char EntryName[100];

    /// idea from the
    /// http://www.ilikebigbits.com/blog/2014/5/6/type-safe-identifiers-in-c
    template <class Tag, class impl> class ID {
      public:
        static ID invalid() { return ID(); }

        // Default constructor which will set m_val to a 0xffffffff (UINT32 max)
        // and signify empty.
        ID() : m_val(0xffffffff) {}

        // Explicit constructor:
        explicit ID(impl val) : m_val(val) {}

        // Explicit conversion to get back the impl:
        // explicit operator impl() const { return m_val; }

        // Implicit conversion to get back the impl
        operator impl() const { return m_val; }

        bool empty() const { return m_val == 0xffffffff ? true : false; }

        // this messes with implicit conversion (type casting)
        /*
        friend bool operator>(ID a, impl b) { return a.m_val > b; }
        friend bool operator<(ID a, impl b) { return a.m_val < b; }
        friend bool operator==(ID a, ID b) { return a.m_val == b.m_val; }
        friend bool operator!=(ID a, ID b) { return a.m_val != b.m_val; }
        */
        impl m_val;
    };

    typedef ID<struct StringTag, uint32_t> StringID;
    typedef ID<struct StringTag, uint32_t> PeerStringID;

    /// create a container for string registry
    /// store the name and id
    typedef struct StringRegistry {
        char *name;
        StringID id;
    } StringRegistry;

    typedef std::vector<StringRegistry> RegistryList;

    /// @todo change this to serialized bytestream in the future
    typedef Json::Value SerializedStringMap;

    /// Centralize a string registry. Basically, the server side, and part
    /// of the client side internals.
    class RegisteredStringMap {
      public:
        /// @brief Constructor
        OSVR_COMMON_EXPORT RegisteredStringMap();

        OSVR_COMMON_EXPORT ~RegisteredStringMap();

        /// retrieve the ID for the current name or register new ID and return
        /// that
        OSVR_COMMON_EXPORT StringID getStringID(std::string const &str);

        /// retrieve the name of the string given the ID
        /// returns empty string if nothing found
        OSVR_COMMON_EXPORT std::string getNameFromID(StringID &id) const;

        /// package the entire (current) map into bytestream and return its copy
        /// will be used to transport of maps between server and client
        OSVR_COMMON_EXPORT SerializedStringMap getMap() const;

        /// add new entry to the string map and return new ID
        OSVR_COMMON_EXPORT StringID registerStringID(std::string const &str);

        OSVR_COMMON_EXPORT bool isUpdateAvailable();

        OSVR_COMMON_EXPORT void printCurrentMap();

      protected:
        /// keep track of number of entries in the registry,
        /// may not be needed if we stick with vectors
        int m_numEntries;
        RegistryList m_regEntries;

        /// special flag that gets switched whenever new element is inserted;
        bool m_updateMap;
    };

    /// This is like a RegisteredStringMap, except it also knows that some peer
    /// also has a string map, likely with some of the same strings, but with
    /// different ids. Used in reconciliation between server and client since
    /// they are
    /// separate entities
    class CorrelatedStringMap : public RegisteredStringMap {
      public:
        /// @brief Constructor
        OSVR_COMMON_EXPORT CorrelatedStringMap();

        OSVR_COMMON_EXPORT ~CorrelatedStringMap();

        /// This is the extra method used by clients, to convert from server's
        /// ids. Will return NULL if peerID to Local ID mapping doesn't exist
        OSVR_COMMON_EXPORT StringID
        convertPeerToLocalID(PeerStringID peerID) const;

        /// This populates the data structure used by the above method.
        OSVR_COMMON_EXPORT void setupPeerMappings(SerializedStringMap peerdata);

        /// This will add the mapping or ignore it if it's present
        void addPeerToLocalMapping(PeerStringID peerID, StringID localID);

      private:
        /// keeps the peer to local string ID mappings
        std::vector<std::pair<PeerStringID, StringID>> mappings;
    };
} // namespace common
} // namespace osvr
#endif // INCLUDED_RegisteredStringMap_h_GUID_066235BE_3687_44AD_C2A4_593D6E6780F3
