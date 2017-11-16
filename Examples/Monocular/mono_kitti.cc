/**
* This file is part of ORB-SLAM2.
*
* Copyright (C) 2014-2016 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <https://github.com/raulmur/ORB_SLAM2>
*
* ORB-SLAM2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM2 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM2. If not, see <http://www.gnu.org/licenses/>.
*/


#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include <thread>
#include <iomanip>
#include <opencv2/core/core.hpp>

#include"System.h"
#include "MapPoint.h"
#include "config.h"

#define USE_JOHN_FACE

using namespace std;

void LoadImages(const string &strSequence, vector<string> &vstrImageFilenames,
                vector<double> &vTimestamps);

int main(int argc, char **argv) {
	/*if(argc != 4)
	{
	cerr << endl << "Usage: ./mono_kitti path_to_vocabulary path_to_settings path_to_sequence" << endl;
	return 1;
	}*/

	//#ifdef USE_JOHN_FACE
	//	std::string dataPath = "C:\\Users\\User\\Source\\Repos\\ARSL_v3.0_CalibrationData\\calibration-6-18\\tracking\\"/*"C:\\Users\\User\\Source\\Repos\\ORB_SLAM2_Windows_samed\\data\\"*/;
	//
	//	std::string videoFile = dataPath + "john-face.mp4"/*"ORBSLAM_Example_Video.mp4"*/;
	//	std::string voc = /*dataPath*/ "C:\\Users\\User\\Source\\Repos\\ORB_SLAM2_Windows_samed\\data\\ORBvoc_new\\ORBvoc_new.txt";
	//	std::string settings = dataPath + "john-face-settings.yaml"/*"Settings_Complete.yaml"*/;
	//#else
	//	std::string dataPath = "C:\\Users\\User\\Source\\Repos\\ORB_SLAM2_Windows_samed\\data\\";
	//
	//	std::string videoFile = dataPath + "ORBSLAM_Example_Video.mp4";
	//	std::string voc = /*dataPath*/ "C:\\Users\\User\\Source\\Repos\\ORB_SLAM2_Windows_samed\\data\\ORBvoc_new\\ORBvoc_new.txt";
	//	std::string settings = dataPath + "Settings_Complete.yaml";
	//#endif

	std::string dataPath = "C:\\Users\\User\\Source\\Repos\\ARSL_v3.0_CalibrationData\\calibration-6-18\\tracking\\"/*"C:\\Users\\User\\Source\\Repos\\ORB_SLAM2_Windows_samed\\data\\"*/;

	std::string videoFile = VIDEO_PATH;
	std::string voc = VOCAB_PATH;
	std::string settings = SETTINGS_PATH;
	std::string arsl2d = ARSL2D;
	std::string arsl3d = ARSL3D;

	// Retrieve paths to images
	vector<string> vstrImageFilenames;
	vector<double> vTimestamps;
	//LoadImages(string(argv[3]), vstrImageFilenames, vTimestamps);
	cv::Mat dispIm = cv::imread(ARSL_IMAGE, CV_LOAD_IMAGE_COLOR);
	//int nImages = vstrImageFilenames.size();

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
	ORB_SLAM2::System SLAM(voc, settings, ARSL3D, ARSL2D, cv::Mat(), ORB_SLAM2::System::MONOCULAR, true);
	//ORB_SLAM2::System SLAM(voc, settings, ORB_SLAM2::System::MONOCULAR, true);
	// Vector for tracking time statistics
	vector<double> vTimesTrack;
	//vTimesTrack.resize(nImages);

	std::cout << endl << "-------" << endl;
	std::cout << "Start processing sequence ..." << endl;
	//cout << "Images in the sequence: " << nImages << endl << endl;

	// Main loop
	bool open = true;
	cv::VideoCapture cap;
	open = cap.open(videoFile);
	cv::Mat im;
	int ni = 0;
	//for(int ni=0; ni<nImages; ni++)
	while (open) {
		open = cap.read(im);
		if (!open)
			break;

		// Read image and depthmap from file
		//im = cv::imread(vstrImageFilenames[ni],CV_LOAD_IMAGE_UNCHANGED);
		double tframe = 33.3; // vTimestamps[ni];

		if (im.empty()) {
			cerr << endl << "Failed to load image at: " << vstrImageFilenames[ni] << endl;
			return 1;
		}

		double ttrack = (double)cv::getTickCount();

		vector<cv::KeyPoint> ARSL2DPts;
		vector<cv::Point3f> ARSL3DPts;

        // Pass the image to the SLAM system
        SLAM.TrackMonocular(im,tframe, ARSL2DPts, ARSL3DPts);
		//SLAM.TrackMonocular(im, tframe);
		ARSL2DPts = SLAM.get2DPts();
		//vector<ORB_SLAM2::MapPoint*> test = SLAM.OutputAllMapPoints();

		//fstream outputFile;
		//outputFile.open("ORB2DPts.txt", ios::out);
		//for (size_t i = 0; i < ARSL2DPts.size(); i++)
		//	outputFile << ARSL2DPts[i].pt.x << ", " << ARSL2DPts[i].pt.y << endl;
		//outputFile.close();

		SLAM.TrackMonocular(im, tframe);

		ttrack = 1000 * (((double)cv::getTickCount() - ttrack) / cv::getTickFrequency());
		//printf("Total: %f\n", ttrack);

		//double ttrack= std::chrono::duration_cast<std::chrono::duration<double> >(t2 - t1).count();
		//printf("TRACKED: %f\n", ttrack);
		vTimesTrack.push_back(ttrack);

		/*// Wait to load the next frame
		double T=0;
		if(ni<nImages-1)
		T = vTimestamps[ni+1]-tframe;
		else if(ni>0)
		T = tframe-vTimestamps[ni-1];

		if (ttrack < T) {
			//usleep((T - ttrack)*1e6);
			std::this_thread::sleep_for(std::chrono::milliseconds((int)((T - ttrack)*1e6)));
		}*/
    }

	// Stop all threads
	SLAM.Shutdown();

	// Tracking time statistics
	//sort(vTimesTrack.begin(),vTimesTrack.end());
	double totaltime = 0;
	for (int ni = 0; ni<vTimesTrack.size(); ni++) {
		totaltime += vTimesTrack[ni];
	}
	std::cout << "-------" << endl << endl;
	//cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
	std::cout << "mean tracking time: " << totaltime / ((double)vTimesTrack.size()) << endl;

	// Save camera trajectory
	SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

	return 0;
}

void LoadImages(const string &strPathToSequence, vector<string> &vstrImageFilenames, vector<double> &vTimestamps)
{
    ifstream fTimes;
    string strPathTimeFile = strPathToSequence + "/times.txt";
    fTimes.open(strPathTimeFile.c_str());
    while(!fTimes.eof())
    {
        string s;
        getline(fTimes,s);
        if(!s.empty())
        {
            stringstream ss;
            ss << s;
            double t;
            ss >> t;
            vTimestamps.push_back(t);
        }
    }

    string strPrefixLeft = strPathToSequence + "/image_0/";

    const int nTimes = vTimestamps.size();
    vstrImageFilenames.resize(nTimes);

    for(int i=0; i<nTimes; i++)
    {
        stringstream ss;
        ss << setfill('0') << setw(6) << i;
        vstrImageFilenames[i] = strPrefixLeft + ss.str() + ".png";
    }
}
