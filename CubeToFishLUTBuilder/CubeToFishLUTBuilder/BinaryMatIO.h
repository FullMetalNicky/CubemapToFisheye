
#include <opencv2\opencv.hpp>


namespace CubemapToFisheye
{
	//courtesy of Miki from SO
	// https://stackoverflow.com/questions/32332920/efficiently-load-a-large-mat-into-memory-in-opencv/32357875#32357875

	class BinaryMatIO
	{
	public:

		/**
		* @brief Writes mat to binary file
		* @param filename - you can guess that on your own
		* @param mat - matrix you wish to write
		*/
		void Write(const std::string& filename, const cv::Mat& mat);

		/**
		* @brief reads binary file to mat
		* @param filename - you can guess that on your own
		* @ret matrix with values initialized by binary file
		*/
		cv::Mat Read(const std::string& filename);
	};
}
