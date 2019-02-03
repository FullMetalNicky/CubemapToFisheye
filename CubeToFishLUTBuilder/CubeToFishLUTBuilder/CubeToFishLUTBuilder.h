
#include <opencv2\opencv.hpp>

namespace CubemapToFisheye
{

	enum CubemapFace
	{
		Right = 0,
		Left,
		Top,
		Bottom,
		Front,
		Back,
		FaceNum
	};
	
	// lut for fish-eye creation from cubemap
	class CubeToFishLUTBuilder
	{
	public:
		/**
		* @brief constructor
		*/
		CubeToFishLUTBuilder();

		/**
		* @brief function that builds the LUT
		* @param outputSize - image size of the output fisheye
		* @param k - intrinsic matrix of the fisheye
		* @param dist - distortion coefficients of the fisheye
		* @param cubemapFaceSize - resolution of the cubemap faces (must be power of 2!!!!)
		*/
		virtual void BuildLUT(cv::Size outputSize, const cv::Mat& k, const cv::Vec4d& dist, float cubemapFaceSize);

		/**
		* @brief saves all maps and LUTs to folder, as binary files
		* @param folder_path - destination folder path
		*/
		void SaveBinary(std::string folder_path = "");

		/**
		* @brief reads all maps and LUTs from folder, to initialize CubeToFishLUTBuilder class
		* @param folder_path - folder path where all maps and LUTs are held
		*/
		void LoadBinary(std::string folder_path = "");

		/**
		* @brief projects 5 cubemap faces to specific fisheye, implemented using OpenCV remap method.
		* @param cubemapFaces - vector of cv::Mat holding the cubemap face. Must be inserted according to the order of CubemapFace enum (right, left, top, bottom, front)
		* @param projected_image - fisheye ouput
		* @param interpolation - ...
		*/
		void ProjectImage(const std::vector<cv::Mat>& cubemapFaces, cv::Mat& projected_image, int interpolation = cv::INTER_LINEAR);

		/**
		* @brief projects 5 cubemap faces to specific fisheye, implemented without OpenCV remap, emulating GPU remapping. For testing only.
		* @param cubemapFaces - vector of cv::Mat holding the cubemap face. Must be inserted according to the order of CubemapFace enum (right, left, top, bottom, front)
		* @param projected_image - fisheye ouput
		*/
		void ProjectImageWithLUT(const std::vector<cv::Mat>& cubemapFaces, cv::Mat& projected_image); //for debugging the shader, mainly


	protected:

		std::vector<cv::Mat> m_mapX;
		std::vector<cv::Mat> m_mapY;
		cv::Mat m_LUT;

		cv::Mat m_mapGPU;
		cv::Size m_outputSize;

	private:

		float CalculateFisheyeRadius(float focalX, float halfFOV, cv::Vec4f dist);

	};

}