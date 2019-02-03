#include "CubeToFishLUTBuilder.h"
#include "BinaryMatIO.h"


using namespace CubemapToFisheye;

CubeToFishLUTBuilder::CubeToFishLUTBuilder()
{
}


float CubeToFishLUTBuilder::CalculateFisheyeRadius(float focalX, float halfFOV, cv::Vec4f dist)
{
	float thetaDist = halfFOV + dist[0] * powf(halfFOV, 3.0f) + dist[1] * powf(halfFOV, 5.0f) + dist[2] * powf(halfFOV, 7.0f) + dist[3] * powf(halfFOV, 9.0f);
	float fish_eye_radius = thetaDist * focalX;

	return fish_eye_radius;
}


void CubeToFishLUTBuilder::BuildLUT(cv::Size outputSize, const cv::Mat& k, const cv::Vec4d& dist, float cubemapFaceSize)
{
	m_outputSize = outputSize;
	for (int f = 0; f < FaceNum; ++f)
	{
		cv::Mat mapX, mapY;
		mapX.create(m_outputSize, CV_32F);
		mapX.setTo(cv::Scalar(FLT_MAX));
		mapY.create(m_outputSize, CV_32F); 
		mapY.setTo(cv::Scalar(FLT_MAX));
		m_mapX.push_back(mapX);
		m_mapY.push_back(mapY);
	}

	m_LUT.create(m_outputSize, CV_32SC1);
	m_LUT.setTo(cv::Scalar(127));
	m_mapGPU.create(m_outputSize, CV_32FC3);
	m_mapGPU.setTo(cv::Scalar(FLT_MAX, FLT_MAX, FLT_MAX));


	cv::Point2f center(k.at<double>(0, 2), k.at<double>(1, 2));
	float half_AFOV_W = CV_PI / 2;
	float fish_eye_radius = CalculateFisheyeRadius(k.at<double>(0, 0), half_AFOV_W, dist);

	for (int j = 0; j < m_outputSize.height; j++)
	{
		unsigned int* lut_j = m_LUT.ptr<unsigned int>(j);

		for (int i = 0; i < m_outputSize.width; i++)
		{
			cv::Point2f distPt(i, j);

			// out of fish eye lens check
			float norm = cv::norm(distPt - center);
			if (cv::norm(distPt - center) > fish_eye_radius)
			{
				continue;
			}

			std::vector<cv::Point2f> distortPts;
			distortPts.push_back(distPt);

			std::vector<cv::Point2f> undistPts;
			cv::fisheye::undistortPoints(distortPts, undistPts, k, dist);
			cv::Point2f undist_pt(undistPts[0].x, undistPts[0].y);
			CubemapFace face;
			cv::Point2f uv(0, 0);

			cv::Point3f undist_pt_abs(abs(undist_pt.x), abs(undist_pt.y), 1.0f);
			if ((undist_pt_abs.x > undist_pt_abs.y) && (undist_pt_abs.x >= undist_pt_abs.z)) //x is the dominant component
			{
				if (undist_pt.x >= 0.0f)
				{
					face = CubemapFace::Right;
					uv = cv::Point2f(-1.0f / undist_pt.x, undist_pt.y / undist_pt.x);
				}
				else
				{
					face = CubemapFace::Left;
					uv = cv::Point2f(-1.0f / undist_pt.x, -undist_pt.y / undist_pt.x);
				}
			}
			else if ((undist_pt_abs.y >= undist_pt_abs.x) && (undist_pt_abs.y >= undist_pt_abs.z)) //y is the dominant component
			{
				if (undist_pt.y >= 0.0f)
				{
					face = CubemapFace::Bottom;
					uv = cv::Point2f(undist_pt.x / undist_pt.y, -1.0f / undist_pt.y);
				}
				else
				{
					face = CubemapFace::Top;
					uv = cv::Point2f(-undist_pt.x / undist_pt.y, -1.0f / undist_pt.y);
				}
			}
			else //z is the dominant component
			{
				face = CubemapFace::Front;
				uv = undist_pt;
			}
			
			uv.x = uv.x * cubemapFaceSize * 0.5 + cubemapFaceSize * 0.5;
			uv.y = uv.y * cubemapFaceSize * 0.5 + cubemapFaceSize * 0.5;

			float* map_x_j = m_mapX[face].ptr<float>(j);
			float* map_y_j = m_mapY[face].ptr<float>(j);

			if (uv.x < 1.0f) uv.x = 1.0f;
			if (uv.y < 1.0f) uv.y = 1.0f;
			if (uv.x >= cubemapFaceSize - 1) uv.x = cubemapFaceSize - 2;
			if (uv.y >= cubemapFaceSize - 1) uv.y = cubemapFaceSize - 2;

			map_x_j[i] = uv.x;
			map_y_j[i] = uv.y;
			lut_j[i] = face;
			
			m_mapGPU.at<cv::Vec3f>(j, i) = cv::Vec3f(uv.x, uv.y, face);		
			
		}
	}

}


void CubeToFishLUTBuilder::SaveBinary(std::string folder_path)
{
	BinaryMatIO bmio;

	for (int f = 0; f < FaceNum; ++f)
	{
		std::string mapXname = folder_path + "mapX_" + std::to_string(f) + ".bin";
		m_mapX.push_back(bmio.Read(mapXname));
		std::string mapYname = folder_path + "mapY_" + std::to_string(f) + ".bin";
		m_mapY.push_back(bmio.Read(mapYname));
	}

	std::string lutName = folder_path + "LUT.bin";
	m_LUT = bmio.Read(lutName);
	std::string gpuMapName = folder_path + "GPUmap.bin";
	m_mapGPU = bmio.Read(gpuMapName);
}


void CubeToFishLUTBuilder::LoadBinary(std::string folder_path)
{
	BinaryMatIO bmio;

	for (int f = 0; f < FaceNum; ++f) 
	{
		std::string mapXname = folder_path + "mapX_" + std::to_string(f) + ".bin";
		bmio.Write(mapXname, m_mapX[f]);
		std::string mapYname = folder_path + "mapY_" + std::to_string(f) + ".bin";
		bmio.Write(mapYname, m_mapY[f]);
	}

	std::string lutName = folder_path + "LUT.bin";
	bmio.Write(lutName, m_LUT);

	std::string gpuMapName = folder_path + "GPUmap.bin";
	bmio.Write(gpuMapName, m_mapGPU);
}


void CubeToFishLUTBuilder::ProjectImage(const std::vector<cv::Mat>& cubemapFaces, cv::Mat& projected_image, int interpolation)
{
	std::vector<cv::Mat> projectedVector;
	projected_image = cv::Mat::zeros(m_outputSize, CV_8UC3);

	for (int i = 0; i < 5; ++i) //no need for back face
	{
		cv::Mat tmp;
		cv::Mat mapX = m_mapX[i];
		cv::Mat mapY = m_mapY[i];
		cv::remap(cubemapFaces[i], tmp, m_mapX[i], m_mapY[i], interpolation);
		projectedVector.push_back(tmp);
		cv::add(projected_image, tmp, projected_image);
	}
}


void CubeToFishLUTBuilder::ProjectImageWithLUT(const std::vector<cv::Mat>& cubemapFaces, cv::Mat& projected_image)
{

}