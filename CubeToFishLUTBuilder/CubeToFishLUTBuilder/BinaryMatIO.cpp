
#include "BinaryMatIO.h"
#include <iostream>
#include <fstream>

using namespace CubemapToFisheye;


void BinaryMatIO::Write(const std::string& filename, const cv::Mat& mat)
{
	std::ofstream fs(filename, std::fstream::binary);

	// Header
	int type = mat.type();
	int channels = mat.channels();
	fs.write((char*)&mat.rows, sizeof(int));    // rows
	fs.write((char*)&mat.cols, sizeof(int));    // cols
	fs.write((char*)&type, sizeof(int));        // type
	fs.write((char*)&channels, sizeof(int));    // channels

	// Data
	if (mat.isContinuous())
	{
		fs.write(mat.ptr<char>(0), (mat.dataend - mat.datastart));
	}
	else
	{
		int rowsz = CV_ELEM_SIZE(type) * mat.cols;
		for (int r = 0; r < mat.rows; ++r)
		{
			fs.write(mat.ptr<char>(r), rowsz);
		}
	}
}

cv::Mat BinaryMatIO::Read(const std::string& filename)
{
	std::ifstream fs(filename, std::fstream::binary);

	// Header
	int rows, cols, type, channels;
	fs.read((char*)&rows, sizeof(int));         // rows
	fs.read((char*)&cols, sizeof(int));         // cols
	fs.read((char*)&type, sizeof(int));         // type
	fs.read((char*)&channels, sizeof(int));     // channels

	// Data
	cv::Mat mat(rows, cols, type);
	fs.read((char*)mat.data, CV_ELEM_SIZE(type) * rows * cols);

	return mat;
}