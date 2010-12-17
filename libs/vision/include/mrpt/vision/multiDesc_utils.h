            /* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2010  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#ifndef mrpt_multiDesc_utils_H
#define mrpt_multiDesc_utils_H

#include <mrpt/vision/CFeature.h>
#include <mrpt/utils/CImage.h>
#include <mrpt/math/utils.h>
#include <mrpt/utils/CLoadableOptions.h>
#include <mrpt/utils/TMatchingPair.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/poses/CPoint2D.h>
#include <mrpt/poses/CPoint3D.h>
#include <mrpt/slam/CLandmarksMap.h>
#include <mrpt/slam/CObservationVisualLandmarks.h>

#include <mrpt/vision/types.h>
#include <mrpt/vision/chessboard_camera_calib.h>

#include <mrpt/vision/link_pragmas.h>

namespace mrpt
{
    namespace vision
    {
        using namespace mrpt::slam;
		using namespace mrpt::math;
		using namespace mrpt::utils;

        // A 3D quantization table for storing pairs of TFeatureIDs and scales
        typedef map<int,map<int,map<int,deque<pair<TFeatureID, double> > > > > TQuantizationTable;

        void VISION_IMPEXP insertHashCoeffs(
                                const CFeaturePtr                   & feat,
                                TQuantizationTable                  & qTable );

        TMultiResMatchingOutput VISION_IMPEXP relocalizeMultiDesc(
                                const CImage                        & image,
                                CFeatureList                        & baseList,
                                CFeatureList                        & currentList,
                                TQuantizationTable                  & qTable,
                                const TMultiResDescOptions          & desc_opts,
                                const TMultiResDescMatchOptions     & match_opts );

        void VISION_IMPEXP updateBaseList(
                                CFeatureList                        & baseList,
                                const CFeatureList                  & currentList,
                                const vector<int>                   & idx );

        void VISION_IMPEXP checkScalesAndFindMore(
                                CMatchedFeatureList                 & baseList,
                                const CFeatureList                  & currentList,
                                const CImage                        & currentImage,
                                const TMultiResMatchingOutput       & output,
                                const TMultiResDescOptions          & computeOpts,
                                const TMultiResDescMatchOptions     & matchOpts );

        /** Computes the gradient of certain pixel within the image.
          * \param image    [IN]    The input image.
          * \param x        [IN]    The 'x' coordinate of the image point.
          * \param y        [IN]    The 'y' coordinate of the image point.
          * \param mag      [OUT]   The magnitude of the gradient.
          * \param ori      [OUT]   The orientation of the gradient.
          * \return True if the gradient could be computed and False if the pixel is located outside the image or at its border (where the gradient cannot be computed)
          */
        bool VISION_IMPEXP computeGradient(
                                const CImage                        & image,
                                const unsigned int                  x,
                                const unsigned int                  y,
                                double                              & mag,
                                double                              & ori );

        /** Computes the main orientations (within 80% of the peak value of orientation histogram) of a certain point within an image (for using in SIFT-based algorithms)
          * \param image        [IN]    The input image.
          * \param x            [IN]    The 'x' coordinate of the image point.
          * \param y            [IN]    The 'y' coordinate of the image point.
          * \param patchSize    [IN]    The size of the patch to be considered for computing the orientation histogram.
          * \param orientations [OUT]   A vector containing the main orientations of the image point.
          * \param sigma        [IN]    The sigma value of the Gaussian kernel used to smooth the orientation histogram (typically 7.5 px).
          */
        bool VISION_IMPEXP computeMainOrientations(
                                const CImage                        & image,
                                const unsigned int                  x,
                                const unsigned int                  y,
                                const unsigned int                  patchSize,
                                std::vector<double>                 & orientations,
                                const double                        & sigma );

        /** Inserts the orientation value of a certain pixel within the keypoint neighbourhood into the histogram of orientations. This value can
          * affect to more than one entry within the histogram.
          * \param hist [IN/OUT]    The histogram of orientations.
          * \param cbin [IN]        The entry rotated column bin.
          * \param rbin [IN]        The entry rotated row bin.
          * \param obin [IN]        The entry rotated orientation bin.
          * \param mag  [IN]        The gradient magnitude value in the pixel.
          * \param d    [IN]        The number of row (and column) bins used in the histogram (typically 4).
          * \param n    [IN]        The number of orienation bins used in the histogram (typically 8).
          */
        void VISION_IMPEXP interpolateHistEntry(
                                vector<double>                      & hist,
                                const double                        & cbin,
                                const double                        & rbin,
                                const double                        & obin,
                                const double                        & mag,
                                const int                           d,
                                const int                           n );

        /** Computes the SIFT-like descriptor of a certain point within an image at the base scale, i.e. its rotated orientation histogram.
          * \param image        [IN]    The input image.
          * \param x            [IN]    The 'x' coordinate of the image point.
          * \param y            [IN]    The 'y' coordinate of the image point.
          * \param patchSize    [IN]    The size of the patch to be considered for computing the orientation histogram.
          * \param orientation  [IN]    The orientation considered for this point (used to rotate the patch).
          * \param orientation  [OUT]   The computed SIFT-like descriptor.
          * \param opts         [IN]    The options for computing the SIFT-like descriptor.
          * \param hashCoeffs   [OUT]   A vector containing the computed coefficients for the HASH table used in relocalization.
          * \sa TMultiResDescOptions
          */
        void VISION_IMPEXP computeHistogramOfOrientations(
                                    const CImage                    & image,
                                    const unsigned int              x,
                                    const unsigned int              y,
                                    const unsigned int              patchSize,
                                    const double                    & orientation,
                                    vector<int>                     & descriptor,
                                    const TMultiResDescOptions      & opts,
                                    vector<int>                     & hashCoeffs );

        /** Matches two CFeatureList containing mulit-resolution descriptors. The first list is taken as a base, i.e. its features must contain multi-resolution descriptors
          * at a set of different scales. The second list doesn't need to contain such information because it will be computed if necessary according to the
          * the fulfillment of some restriction regarding the matching process. This function will try to find the best matches within list2 corresponding to the features
          * within base list list1.
          * \param list1            [IN]        The base list of features.
          * \param list2            [IN/OUT]    The other list of features.
          * \param rightImage       [IN]        The image from where the list2 was extracted. It's used to compute the descriptor of these features if necessary.
          * \param output           [OUT]       The output structure with the matching information.
          * \param matchOpts        [IN]        The options structure for the matching process.
          * \param computeOpts      [IN]        The options structure for the descriptor computation process.
          * \return A structure containing the results of the matching.
          * \sa TMultiResDescMatchOptions, TMultiResDescOptions, TMultiResMatchingOutput
          */
        TMultiResMatchingOutput VISION_IMPEXP matchMultiResolutionFeatures(
                                    const CFeatureList              & list1,
                                    CFeatureList                    & list2,
                                    const CImage                    & rightImage,
                                    const TMultiResDescMatchOptions & matchOpts,
                                    const TMultiResDescOptions      & computeOpts );

        /** Matches two CMatchedFeatureList containing mulit-resolution descriptors. This is performed for both the "left" and "right" lists
          * The first matched list is taken as a base, i.e. its features must contain multi-resolution descriptors
          * at a set of different scales. The second list doesn't need to contain such information because it will be computed if necessary according to the
          * the fulfillment of some restriction regarding the matching process. This function will try to find the best matches within list2 corresponding to the features
          * \param mList1           [IN]    The base list.
          * \param mList2           [IN]    The other list of features.
          * \param leftImage        [IN]    The image from where the list2 was extracted. It's used to compute the descriptor of these features if necessary.
          * \param rightImage       [IN]    The image from where the list2 was extracted. It's used to compute the descriptor of these features if necessary.
          * \param matchOpts        [IN]        The options structure for the matching process.
          * \param computeOpts      [IN]        The options structure for the descriptor computation process.
          * \return The number of matches found
          * \sa TMultiResDescMatchOptions
          */
        int VISION_IMPEXP matchMultiResolutionFeatures(
                                    CMatchedFeatureList             & mList1,
                                    CMatchedFeatureList             & mList2,
                                    const CImage                    & leftImage,
                                    const CImage                    & rightImage,
                                    const TMultiResDescMatchOptions & matchOpts,
                                    const TMultiResDescOptions      & computeOpts );

        /** Computes more multi-resolution SIFT-like descriptors for a feature using its position in a new image. This
          * is called when we have found a match between a feature and itself in a new frame but it has been found in
          * a boundary scale. We now expand the range of scales, orientations and descriptors for that feature.
          * \param image        [IN]    The new frame.
          * \param inputFeat    [IN]    The feature in the new frame.
          * \param outputFeat   [OUT]   The base feature (detected in the base frame).
          * \param lowerScales  [IN]    If we should find descriptors for lower scales or for higher ones.
          * \param opts         [IN]    The options for computing the new descriptors.
          */
        int VISION_IMPEXP computeMoreDescriptors(
                                    const CImage                    & image,
                                    const CFeaturePtr               & inputFeat,
                                    CFeaturePtr                     & outputFeat,
                                    const bool                      & lowerScales,
                                    const TMultiResDescOptions      & opts );

        /** Computes the initial and final scales where to look when finding a match between multi-resolution features.
          * Both features must have their "depth" member properly computed.
          * \param feat1        [IN]    The base feature which MUST contain a set of different scales.
          * \param feat2        [IN]    The other feature which must be computed at base scale (1.0).
          * \param firstScale   [OUT]   The initial scale (within [0 feat1->multiScale.size()-1]) where to look.
          * \param firstScale   [OUT]   The final scale (within [0 feat1->multiScale.size()-1]) where to look.
          */
        void VISION_IMPEXP setProperScales(
                                    const CFeaturePtr               & feat1,
                                    const CFeaturePtr               & feat2,
                                    int                             & firstScale,
                                    int                             & lastScale );

        /** Computes the multi-resolution SIFT-like descriptor of a set of matched features
          * \param imageLeft    [IN]        The input left image.
          * \param imageRight   [IN]        The input right image.
          * \param matchedFeats [IN/OUT]    The list of matched features. They will be updated with the multi-scales, multi-orientations, multi-descriptors and depth information.
          * \param opts         [IN]        The options structure for the descriptor computation process.
          * \sa TMultiResDescOptions
        */
        void VISION_IMPEXP computeMultiResolutionDescriptors(
                                    const CImage                    & imageLeft,
                                    const CImage                    & imageRight,
                                    CMatchedFeatureList             & matchedFeats,
                                    const TMultiResDescOptions      & opts );


        /** Computes the multi-resolution SIFT-like descriptor of a features
          * \param image        [IN]        The input left image.
          * \param feat         [IN/OUT]    The feature. It will be updated with the multi-scales, multi-orientations, multi-descriptors
          * \param opts         [IN]        The options structure for the descriptor computation process.
          * \sa TMultiResDescOptions
        */
        bool VISION_IMPEXP computeMultiResolutionDescriptors(
                                    const CImage                    & image,
                                    CFeaturePtr                     & feat,
                                    const TMultiResDescOptions      & opts );

        /** Computes the multi-resolution SIFT-like descriptor of a list of features
          * \param image    [IN]        The input image.
          * \param list     [IN/OUT]    The list of features. They will be updated with the multi-scales, multi-orientations and multi-descriptors information.
          * \param opts     [IN]        The options structure for the descriptor computation process.
          * \sa TMultiResDescOptions
          */
        vector<bool> VISION_IMPEXP computeMultiResolutionDescriptors(
                                    const CImage                    & image,
                                    CFeatureList                    & list,
                                    const TMultiResDescOptions      & opts );

        /** Computes the multi-resolution SIFT-like descriptor of a list of features
          * \param image    [IN]        The input image.
          * \param list     [IN/OUT]    The list of features. They will be updated with the multi-scales, multi-orientations and multi-descriptors information.
          * \param opts     [IN]        The options structure for the descriptor computation process.
          * \sa TMultiResDescOptions
          */
        void VISION_IMPEXP computeMultiOrientations(
                                    const CImage                    & image,
                                    CFeatureList                    & list,
                                    const TMultiResDescOptions      & opts );









    }
} // end-namespace-mrpt

#endif