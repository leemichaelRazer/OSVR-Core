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

#ifndef INCLUDED_VideoBasedTracker_h_GUID_831CC0DD_16A5_43AB_12D3_AE86E1998ED4
#define INCLUDED_VideoBasedTracker_h_GUID_831CC0DD_16A5_43AB_12D3_AE86E1998ED4

// Internal Includes
#include "LED.h"
#include "LedIdentifier.h"
#include "BeaconBasedPoseEstimator.h"
#include <osvr/Util/ChannelCountC.h>

// Library/third-party includes
#include <opencv2/core/core.hpp>

// Standard includes
#include <vector>
#include <list>
#include <functional>


// Define the constant below to provide debugging (window showing video and
// behavior, printing tracked positions)
#define VBHMD_DEBUG

namespace osvr {
namespace vbtracker {
    class VideoBasedTracker {
      public:
        typedef std::vector<LedIdentifierPtr> LedIdentifierList;
        typedef std::list<Led> LedGroup;
        typedef std::vector<LedGroup> LedGroupList;
        typedef std::vector<EstimatorPtr> EstimatorList;

        typedef std::vector<std::vector<double> > DoubleVecVec;
        void addOculusSensor();
        void addSensor(LedIdentifier * identifier, DoubleVecVec const& m, std::vector<double> const& d, DoubleVecVec const& locations);

        typedef std::function<void(OSVR_ChannelCount, OSVR_Pose3 const&)> PoseHandler;
        /// @return true if user hit q to quit.
        bool processImage(cv::Mat frame, cv::Mat grayImage,
                          PoseHandler handler);

      private:
        typedef std::vector<cv::KeyPoint> KeyPointList;
#if 0
        void m_processSensor(KeyPointList const &foundKeyPoints,
                             LedGroup &ledGroup);
#endif
        /// @name Images
        /// @{
        cv::Mat m_frame;
        cv::Mat m_imageGray;
        cv::Mat m_thresholdImage;
        cv::Mat m_imageWithBlobs;
#ifdef VBHMD_DEBUG
        cv::Mat *m_shownImage = &m_imageWithBlobs;
#endif
        /// @}

        /// @name Structures needed to do the tracking.
        /// @{
        LedIdentifierList m_identifiers;
        LedGroupList m_led_groups;
        EstimatorList m_estimators;
        /// @}

        /// @brief The pose that we report
        OSVR_PoseState m_pose;
    };

} // namespace vbtracker
} // namespace osvr

#endif // INCLUDED_VideoBasedTracker_h_GUID_831CC0DD_16A5_43AB_12D3_AE86E1998ED4
