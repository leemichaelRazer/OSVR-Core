/** @file
    @brief Implementation

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

// Internal Includes
#include <osvr/Common/RegisteredStringMap.h>

// Library/third-party includes
#include <boost/algorithm/string.hpp>

// Standard includes
#include <iostream>

namespace osvr {
namespace common {

    /// @brief set the initial number of entries to zero, and flag to update the
    /// Map (since it's new)
    RegisteredStringMap::RegisteredStringMap()
        : m_numEntries(0), m_updateMap(true) {}

    /// @todo add proper destructor
    RegisteredStringMap::~RegisteredStringMap() {}

    /// @brief helper function to print size and contents of the map
    void RegisteredStringMap::printCurrentMap() {

        std::cout << "Current map contains " << m_regEntries.size()
                  << " entries: " << std::endl;
        for (auto &entry : m_regEntries) {
            std::cout << "ID: " << uint32_t(entry.id) << "; "
                      << "Name: " << entry.name << std::endl;
        }
    }

    SerializedStringMap RegisteredStringMap::getMap() const {

        Json::Value serializedMap;

        for (auto &entry : m_regEntries) {

            serializedMap[entry.name] = entry.id.m_val;
        }
        // std::cout << serializedMap.toStyledString() << std::endl;
        return serializedMap;
    }

    StringID RegisteredStringMap::registerStringID(std::string const &str) {

        // we've checked the entries before and haven't found so we'll just add
        // new one
        StringRegistry newEntry;

        newEntry.name = new EntryName;
        strncpy(newEntry.name, str.c_str(), sizeof(EntryName) - 1);

        // IDs start at 0 so no need to offset, it will just follow vector index
        StringID newID(m_regEntries.size());
        newEntry.id = newID;
        m_numEntries++;

        m_regEntries.push_back(newEntry);

        // printCurrentMap();
        m_updateMap = true;
        return newID;
    }

    StringID RegisteredStringMap::getStringID(std::string const &str) {

        // printCurrentMap();

        // check the existing registry first
        for (auto &entry : m_regEntries) {
            // found a matching name, NOTE: CaSe Sensitive
            if (boost::equals(str, entry.name)) {
                m_updateMap = false;
                return entry.id;
            }
        }

        // we didn't find an entry in the registry so we'll add a new one
        return registerStringID(str);
    }

    std::string RegisteredStringMap::getNameFromID(StringID &id) const {

        // requested non-existent ID (include sanity check)
        if ((id > m_regEntries.size()) || (id < 0)) {
            // returning empty string
            return std::string();
        }

        // entries should be ordered 0-.. with new ones
        // appending to the end, so we should be safe at pulling by vector index

        for (auto &entry : m_regEntries) {
            if (entry.id == id) {
                // found name for this ID
                return entry.name;
            }
        }
        // returning empty string
        return std::string();
    };

    bool RegisteredStringMap::isUpdateAvailable() { return m_updateMap; }

    CorrelatedStringMap::CorrelatedStringMap() {}

    /// @todo add proper destructor
    CorrelatedStringMap::~CorrelatedStringMap() {}

    StringID
    CorrelatedStringMap::convertPeerToLocalID(PeerStringID peerID) const {

        // go thru the mappings and return an empty StringID if nothing's found
        for (auto &mapping : mappings) {

            if (mapping.first == peerID) {
                return mapping.second;
            }
        }

        return StringID();
    }

    void CorrelatedStringMap::addPeerToLocalMapping(PeerStringID peerID,
                                                    StringID localID) {

        // there should be 1 to 1 mapping between peer and local IDs,
        // so if we find one, it should be correct
        for (auto &mapping : mappings) {
            // got a match
            if ((mapping.first == peerID) && (mapping.second == localID)) {
                return;
            }
        }
        // add new mapping
        mappings.push_back(std::make_pair(peerID, localID));
    }

    void CorrelatedStringMap::setupPeerMappings(SerializedStringMap peerData) {

        // go thru the peerData, you get name and id
        // this name may already be stored in correlatedMap
        // so we get the localID first (or register it)
        // then store it in the mappings
        auto &gestureNames = peerData.getMemberNames();

        for (auto &name : gestureNames) {

            PeerStringID peerID(peerData[name].asUInt());
            StringID localID = getStringID(name);
            addPeerToLocalMapping(peerID, localID);
        }
    }
}
}