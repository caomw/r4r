/*
 * tval.h
 *
 *  Created on: Apr 4, 2012
 *      Author: jbalzer
 */

#ifndef TVAL_H_
#define TVAL_H_


#include <opencv2/opencv.hpp>
#include "tracker.h"


namespace R4R {

/*! \brief tracker validation and visualization
 *
 *
 *
 */
class CTrackerValidation {

public:

	//! Constructor.
	CTrackerValidation(const char* file, cv::Size size);

	//! Constructor.
	CTrackerValidation(const char* file);

	//! Constructor.
	CTrackerValidation(const char* file, double fps);

	//! Desctructor.
	~CTrackerValidation() { m_input.release(); };

	/*! \brief Writes the video #m_input to file.
	 *
	 * \param[out] file name of output file
	 * \param[in] T maximal length of output video
	 *
	 */
	bool WriteVideo(const char* file, size_t T);

	/*! Writes a video overlayed with the tracks of different trackers.
	 *
	 * \param[in] file name of output file
	 * \param[in] trackers pointers to trackers to overlay with video stram
	 * \param[in] tmax maximum number of frames to write
	 *
	 */
	bool WriteTracks(const char* file, std::vector<CTracker*> trackers, size_t tmax);

	/*! \brief Writes a RoI to a sequence of images w.r.t. to its covariant frame.
	 *
	 * \param[out] dir output directory
	 * \param[out] prefix prefix to identify the kind of tracker used
	 * \param[in] tracklet pointer to the tracklet to save
	 * \param[in] hsize half of the size of the RoI
	 *
	 */
	bool WriteTrack(const char* dir, const char* prefix, shared_ptr<CTracklet> tracklet, size_t hsize);

	/*! \brief Shows some statistics of a tracker over its life time.
	 *
	 * \details Currently being computed are:
	 * - the number of tracks that have been created in total
	 * - maximum number of frames that some tracklet survived
	 * - average number of frames a tracklet survived
	 *
	 */
	void ShowStatistics(std::vector<CTracker*> trackers);

	//! Saves the covisibility graph to file.
	bool SaveToFile(const char* filename, std::map<shared_ptr<CTracklet>,std::list<shared_ptr<CTracklet> > > graph);

private:

	cv::VideoCapture m_input;				//!< input stream
	cv::Size m_size;						//!< size of output video
	double m_fps;							//!< frame rate of output


};

}

#endif /* TVAL_H_ */