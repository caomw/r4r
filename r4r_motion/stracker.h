/*
 * stracker.h
 *
 *  Created on: Mar 20, 2012
 *      Author: jbalzer
 */

#ifndef STRACKER_H_
#define STRACKER_H_

#define COMPUTE_CURVE 0

#include "tracker.h"
#include <opencv2/opencv.hpp>

namespace R4R {

/*! \brief simple LK tracker
 *
 *
* \details The following parameters have to be supplied by means of an object of CParameters:
 *
 * - SCALE number of scale levels to track
 * - FEATURE_THRESHOLD detection threshold
 * - TERMINATION_TIME number of frames to process in video stream
 * - REFRESH_RATE number of frames between redetections
 * - TRACKING_HSIZE half window size (for low-level motion estimation)
 * - DESCRIPTOR_HSIZE half-window size for descriptor computation
 * - MINIMAL_FEATURE_HDISTANCE half of the minimal distance that new features keep to existing ones
 * - MAX_HAMMING_DISTANCE threshold on hamming distance between initial and current BRIEF descriptor above which track
 * is dropped
 * - LK_PYRAMID_LEVEL number of scales to use in low-level LK motion estimation
 * - MAX_ITER maximum number of Gauss-Newton steps to execute in LK motion estimation
 * - ACCURACY time derivative of objective below which to break off Gauss-Newton iteration
 * - LAMBDA relative weight of the spatial image derivatives impact to the optical flow estimation
 *
 * Features stem from FAST detection with non-maximum suppression.  A weak minimal distance constraint is enforced
 * through integral images. A BRIEF descriptor is computed for every feature which enables a comparison with the
 * initial configuration of the corresponding tracklet. This provides a simple but efficient mechanism for occlusion
 * detection.
 *
 * \todo Make it faster by using only one integral image during cleaning and addition.
 */
class CSimpleTracker: public CTracker {

public:

	//! Constructor.
    CSimpleTracker(CParameters* params);

	/*! \copydoc CTracker::Init(cv::Mat& img)
	 *
	 * \details First, an image pyramid of height \f$SCALE+1\f$ is built. Features are extracted
	 * at the coarsest scale. They form the initial values of a set of tracklets that are attached
	 * to the tracker.
	 *
	 */
	bool Init(std::vector<cv::Mat>& pyramid);

	//! \copydoc CTracker::Update(std::vector<cv::Mat>&,std::vector<cv::Mat>&)
	bool Update(std::vector<cv::Mat>& pyramid0, std::vector<cv::Mat>& pyramid1);

	//! \copydoc CTracker::Update(std::vector<cv::Mat>&)
	bool UpdateDescriptors(std::vector<cv::Mat>& pyramid);

	//! \copydoc CTracker::Clean(cv::Mat&,cv::Mat&)
    void Clean(std::vector<cv::Mat>& pyramid0, std::vector<cv::Mat>& pyramid1);

	//! \copydoc CTracker::AddTracklets(cv::Mat&)
	bool AddTracklets(std::vector<cv::Mat>& pyramid);

protected:

	cv::FastFeatureDetector m_detector;						//!< feature detector

};

}

#endif /* STRACKER_H_ */