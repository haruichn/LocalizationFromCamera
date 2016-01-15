// background_substraction
// 画像同士の差分をを用いた背景差分の実装
// 差分部分を短形で囲み足元の座標を計算
// SPACE: 背景画像更新
// ESC: 終了
// s: 座標出力&終了
// http://opencv.blog.jp/cpp/backbround_subtraction


#include <opencv2/opencv.hpp>
#include <highgui.h>

#include <iostream>
#include <fstream>
#include <string>
#include <time.h>


using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
	// bool recFlag = false;
	vector<Point> footPositions;
	unsigned int idx_fposi = 0;
	int footPointX = 0;
	int footPointY = 0;

	Mat im, gray, bg, diff, dst, dst_blured, range_out;
	int th = 30; // threshold
	// const float redu_retio = 0.5;

	// Object for contour
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	vector<Vec3f> vecCircles;
	vector<Vec3f>::iterator itrCircles;


    // Camera Init
    cout << "Start camera initialization" << endl;
	VideoCapture cap(0);
    cout << "ScreenSize(" << cap.get(CV_CAP_PROP_FRAME_WIDTH) << ", " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << ")" << endl;
	// cout << "framerate: " << cap.get(CV_CAP_PROP_FPS) << endl;

	if (!cap.isOpened())
	{
        cout << "camera error" << endl;
        return -1;	// error processing of capture
    }
    cout << "Finish camera initialization" << endl;

	cap >> im;
	// resize(im, im, Size(), redu_retio, redu_retio, INTER_LINEAR);
	cvtColor(im, bg, CV_BGR2GRAY); // setting first camera image to background image

	while (1)
	{
		cap >> im;

		// TODO: Deploy a function calculates average of brightness level of sime frame

		// resize(im, im, Size(), redu_retio, redu_retio, INTER_LINEAR);

		cvtColor(im, gray, CV_BGR2GRAY);
		absdiff(gray, bg, diff); // Getting defference between present frame and background image

		threshold(diff, dst, th, 255, THRESH_BINARY);	// Binarizate defferenceimage using threshold

		// medianBlur(diff, blued, 5); // cut noise using median filter
		GaussianBlur(dst, dst_blured, Size(1,1),2.0,2.0);


		// Putting a framed box around largest contour
		/////////////////////////////////////////////////////////////////
		// Detecting contur
		// inRange(dst_blured, Scalar(100, 100, 100), Scalar(120, 255, 255), dst_blured);
		findContours(dst_blured, contours, hierarchy, CV_RETR_EXTERNAL,  CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

		/// Approximate contours to polygons + get bounding rects and circles
	    vector<vector<Point> > contours_poly( contours.size() );
	    vector<Rect> boundRect( contours.size() );
	    vector<Point2f>center( contours.size() );
	    vector<float>radius( contours.size() );

		int idx_maxRect = 0;
		int rectSize = 0;
		int max_rectArea = 0;

	    for( int i = 0; i < contours.size(); i++ )
	    {
			// Find a largest contour index
			double a = contourArea( contours[i],false);  //  Find the area of contour
			if(a > max_rectArea){
				max_rectArea = a;
				idx_maxRect = i;
			}
	        approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
	        boundRect[i] = boundingRect( Mat(contours_poly[i]) );
	    }

		// Calculates the bounding rect of the largest area contour
	    Mat drawing = Mat::zeros( dst_blured.size(), CV_8UC3 );

		/// Draw polygonal contour + bonding rects
		Scalar color = Scalar(0,0,255);
		drawContours( drawing, contours_poly, idx_maxRect, color, 1, 8, vector<Vec4i>(), 0, Point() );
		rectangle( drawing, boundRect[idx_maxRect].tl(), boundRect[idx_maxRect].br(), color, 2, 8, 0 );
		rectangle( im, boundRect[idx_maxRect].tl(), boundRect[idx_maxRect].br(), color, 2, 8, 0 );


		// Calc FootPoint
		Point pt1, pt2;
		pt1.x = boundRect[idx_maxRect].x;
		pt1.y = boundRect[idx_maxRect].y;
		pt2.x = boundRect[idx_maxRect].x + boundRect[idx_maxRect].width;
		pt2.y = boundRect[idx_maxRect].y + boundRect[idx_maxRect].height;

		int footPointX = (pt1.x+pt2.x)/2;
		int footPointY = pt2.y;


		// Drowing a foot point
		circle(drawing, Point((pt1.x+pt2.x)/2, pt2.y), 5, Scalar(255,0,0), -1, CV_AA);
		circle(im, Point((pt1.x+pt2.x)/2, pt2.y), 5, Scalar(255,0,0), -1, CV_AA);
		// cout << "FootPoint: " << Point(footPointX, footPointY) << endl;

		// Add foot position into positons array
		int rectWidth = boundRect[idx_maxRect].width;
		if (rectWidth > 50 && rectWidth <1800) {
			footPositions.push_back(Point(footPointX, footPointY));
		}

		// Draw multi rectangle
		/*
	    for( int i = 0; i< contours.size(); i++ )
	    {
	        Scalar color = Scalar(0,0,255);
	        drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	        rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
	    }
		*/
		/////////////////////////////////////////////////////////////////

		namedWindow("camera", CV_WINDOW_KEEPRATIO);
		namedWindow("dst", CV_WINDOW_KEEPRATIO);
		namedWindow("background subtraction", CV_WINDOW_KEEPRATIO);
		imshow("camera", im);
		imshow("dst", dst);
		imshow("background subtraction", drawing);


		// Press space key, renew background image
		if (waitKey(1) == 32) cvtColor(im, bg, CV_BGR2GRAY);
		else if (waitKey(1) == 27) break; // Press esc key, exit this program;

		else if (waitKey(1) == 115)
		{
			cout << "Save output file" << endl;

			// Press s key to save output file
			// for output
			time_t now = time(NULL);
		    struct tm *pnow = localtime(&now);

			string out_filename = "footPositions" + to_string(pnow->tm_year+1900) + to_string(pnow->tm_mon + 1) + to_string(pnow->tm_mday) + to_string(pnow->tm_hour) + to_string(pnow->tm_min) + ".txt";

			ofstream output_file;
			output_file.open(out_filename, ios::out | ios::app);

			// Wriring foot positions to output file from position arrays
			for(int i=0; i<footPositions.size(); i++){
				output_file << footPositions[i].x << " " << footPositions[i].y << endl;
			}
			break;
		}
	}

	return 0;
}
