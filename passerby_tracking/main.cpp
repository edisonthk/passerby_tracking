#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif



#define CAPTURE_WIDTH 500
#define CAPTURE_HEIGHT 400
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800
#define ANDROID_HEIGHT 150
#define ANDROID_WIDTH 130

#define INTRODUCING_FRAME_TIME 750		// unit in milliseconds
#define EXCLAMATED_TIME 700		// unit in milliseconds
#define ANIMATION_PERIOD 100			// unit in milliseconds

#define INTRODUCE_POSITION_X 350
#define INTRODUCE_POSITION_Y 350

// flag
#define NORMAL_STATE 0
#define EXCLAMATING_STATE 1
#define INTRODUCING_STATE 2
#define SOMEBODY_FLAG 1
#define NOBODY_FLAG 0
#define NORMAL_STATE_MOVING_RIGHT 1
#define NORMAL_STATE_MOVING_LEFT 0
#define SHOW_MOVING_ANIMATION 0
#define STOP_MOVING_ANIMATION 1


#define WINDOW_NAME "Web Camera"

// note for making it more faster
// 1. if it is stand and previous action is also stand, then don't do resize, overlay or something

#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <stdio.h>
#include <time.h>
#include <opencv2\highgui\highgui.hpp>
#include <opencv\highgui.h>
#include <opencv\cv.h>

using namespace cv;

struct HistoryData {
	int image_index;
	int h;
	int w;
	int x;
	int y;
};

struct HistoryManager {
	struct HistoryData now;
	struct HistoryData next;
	struct HistoryData dream;
};

typedef struct HistoryManager HistoryManager;
typedef struct HistoryData HistoryData;

HistoryManager historyManager;
double moving_distance_per_frame = 10.0;
int current_state = NORMAL_STATE;
int somebody_flag = NOBODY_FLAG;
int introducing_state_count = 0;
int introducing_current_frame_index = 0;
int exclamated_state_count = 0;
int normal_state_moving_direction = NORMAL_STATE_MOVING_RIGHT;
int show_moving_animation_flag = SHOW_MOVING_ANIMATION;

std::vector<cv::Mat> introducing_mats;
std::vector<cv::Mat> right_mats;
std::vector<cv::Mat> left_mats;
std::vector<cv::Mat> front_mats;
std::vector<cv::Mat> back_mats;
cv::Mat stand_mat;
cv::Mat exclamation_mat;
cv::Mat logo_mat;

std::vector<std::string> introducing_images = {
	"words/1.png",
	"words/2.png",
	"words/3.png"
};

std::vector<std::string> right_images = {
	"modified_side/right/2.png",
	"modified_side/right/3.png",
	"modified_side/right/4.png",
	"modified_side/right/5.png",
	"modified_side/right/6.png",
	"modified_side/right/7.png",
	"modified_side/right/8.png",
	"modified_side/right/9.png",
	"modified_side/right/10.png",
	"modified_side/right/11.png",
	"modified_side/right/12.png",
	"modified_side/right/13.png",
	"modified_side/right/14.png",
	"modified_side/right/15.png",
	"modified_side/right/16.png",
	"modified_side/right/17.png"
};

std::vector<std::string> left_images = {
	"modified_side/left/2.png",
	"modified_side/left/3.png",
	"modified_side/left/4.png",
	"modified_side/left/5.png",
	"modified_side/left/6.png",
	"modified_side/left/7.png",
	"modified_side/left/8.png",
	"modified_side/left/9.png",
	"modified_side/left/10.png",
	"modified_side/left/11.png",
	"modified_side/left/12.png",
	"modified_side/left/13.png",
	"modified_side/left/14.png",
	"modified_side/left/15.png",
	"modified_side/left/16.png",
	"modified_side/left/17.png",
};

std::vector<std::string> front_images = {
		"modified_front/1.png",
		"modified_front/2.png",
		"modified_front/3.png",
		"modified_front/4.png",
		"modified_front/5.png",
		"modified_front/6.png",
		"modified_front/7.png",
		"modified_front/8.png",
		"modified_front/9.png",
		"modified_front/10.png",
		"modified_front/11.png",
		"modified_front/12.png",
		"modified_front/13.png",
		"modified_front/14.png",
		"modified_front/15.png",
	};

std::vector<std::string> back_images = {
		"modified_back/1.png",
		"modified_back/2.png",
		"modified_back/3.png",
		"modified_back/4.png",
		"modified_back/5.png",
		"modified_back/6.png",
		"modified_back/7.png",
		"modified_back/8.png",
		"modified_back/9.png",
		"modified_back/10.png",
		"modified_back/11.png",
		"modified_back/12.png",
		"modified_back/13.png",
	};

std::string stand_image = "stand.png";
std::string exclamation_image = "exclamation.png";
std::string logo_image = "logo.png";


Mat background(Size(WINDOW_WIDTH, WINDOW_HEIGHT), CV_8UC3);


void loadMats() {
	int i;
	for (i = 0; i < introducing_images.size(); i++){
		introducing_mats.push_back(cvLoadImage(introducing_images[i].c_str(), -1));
	}
	for (i = 0; i < right_images.size(); i++){
		right_mats.push_back(cvLoadImage(right_images[i].c_str(), -1));
	}
	for (i = 0; i < left_images.size(); i++){
		left_mats.push_back(cvLoadImage(left_images[i].c_str(), -1));
	}
	for (i = 0; i < front_images.size(); i++){
		front_mats.push_back(cvLoadImage(front_images[i].c_str(), -1));
	}
	for (i = 0; i < back_images.size(); i++) {
		back_mats.push_back(cvLoadImage(back_images[i].c_str(), -1));
	}

	stand_mat = cvLoadImage(stand_image.c_str(), -1);
	exclamation_mat = cvLoadImage(exclamation_image.c_str(), -1);
	
}

void overlayImage(const cv::Mat &background, const cv::Mat &foreground, cv::Mat &output, cv::Point2i location)
{
	background.copyTo(output);


	// start at the row indicated by location, or at row 0 if location.y is negative.
	for (int y = std::max(location.y, 0); y < background.rows; ++y) {
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if (fY >= foreground.rows)
			break;

		// start at the column indicated by location, 

		// or at column 0 if location.x is negative.
		for (int x = std::max(location.x, 0); x < background.cols; ++x)
		{
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if (fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity =
				((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

				/ 255.;


			// and now combine the background and foreground pixel, using the opacity, 

			// but only if opacity > 0.
			for (int c = 0; opacity > 0 && c < output.channels(); ++c)
			{
				unsigned char foregroundPx =
					foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx =
					background.data[y * background.step + x * background.channels() + c];
				output.data[y*output.step + output.channels()*x + c] =
					backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}

// opencv Action
void opencvAction(cv::Mat a, int x, int y) {
	Mat rst, temp;
	overlayImage(background, a, rst, cv::Point(x, y));
	if (current_state == EXCLAMATING_STATE){
		Mat em = exclamation_mat;
		overlayImage(rst, em, rst, cv::Point(x + ANDROID_WIDTH + 2, y));
	}
	imshow(WINDOW_NAME, rst);
}


// handle all kinds of animation action in this function
void handleAction() {

	if (current_state == INTRODUCING_STATE) {
		historyManager.dream.x = INTRODUCE_POSITION_X;
		historyManager.dream.y = INTRODUCE_POSITION_Y;
		if (historyManager.now.x != historyManager.dream.x || historyManager.now.y != historyManager.dream.y){
			// walk toward 
		}
		else{

			show_moving_animation_flag = STOP_MOVING_ANIMATION;

			if (introducing_state_count * ANIMATION_PERIOD >= INTRODUCING_FRAME_TIME){
				introducing_current_frame_index++;
				if (introducing_current_frame_index >= introducing_images.size()){
					// return from introducing state to normal state
					introducing_current_frame_index = 0;
					historyManager.dream.y = 200;
					current_state = NORMAL_STATE;
					introducing_state_count = 0;
					if (normal_state_moving_direction == NORMAL_STATE_MOVING_LEFT){
						historyManager.dream.x = 0;
					}
					else{
						historyManager.dream.x = WINDOW_WIDTH - ANDROID_WIDTH;
					}
					show_moving_animation_flag = SHOW_MOVING_ANIMATION;
					return;
				}
				introducing_state_count = 0;
			}

			introducing_state_count++;

			// OpenCV Action here
			// overlay standing Android into background
			Mat rst, temp;
			Mat stand = stand_mat;
			overlayImage(background, temp, rst, cv::Point(historyManager.now.x, historyManager.now.y));

			// overlay message into background
			
			Mat dialog = introducing_mats[introducing_current_frame_index];
			overlayImage(rst, dialog, rst, cv::Point(historyManager.now.x + ANDROID_WIDTH + 8, historyManager.now.y - (ANDROID_HEIGHT/2)));

			// show in windows
			imshow(WINDOW_NAME, rst);

		}
	}
	else if (current_state == EXCLAMATING_STATE){
		if (exclamated_state_count * ANIMATION_PERIOD >= EXCLAMATED_TIME){
			current_state = INTRODUCING_STATE;
			exclamated_state_count = 0;
		}
		exclamated_state_count++;
	}
	else if (somebody_flag == NOBODY_FLAG && current_state == NORMAL_STATE){
		// normal state
		if (historyManager.now.x == 0){
			historyManager.dream.y = 200;
			historyManager.dream.x = WINDOW_WIDTH - ANDROID_WIDTH;
			normal_state_moving_direction = NORMAL_STATE_MOVING_RIGHT;
		}
		else if (historyManager.now.x == WINDOW_WIDTH - ANDROID_WIDTH){
			historyManager.dream.y = 200;
			historyManager.dream.x = 0;
			normal_state_moving_direction = NORMAL_STATE_MOVING_LEFT;
		}
	}
	else if (somebody_flag == SOMEBODY_FLAG && current_state == NORMAL_STATE) {
		// exclamation mark here
		current_state = EXCLAMATING_STATE;
	}

}

void movingForwardImage(int x, int y) {
	if (historyManager.now.image_index >= front_images.size()){
		historyManager.now.image_index = 0;
	}
	
	// opencv
	opencvAction(front_mats[historyManager.now.image_index], x, y);

	historyManager.now.image_index++;
}

void standImage(int x, int y) {
	opencvAction(stand_mat, x, y);
}

void movingBackwardImage(int x, int y) {
	if (historyManager.now.image_index >= back_images.size()){
		historyManager.now.image_index = 0;
	}

	// opencv 
	opencvAction(back_mats[historyManager.now.image_index], x, y);

	historyManager.now.image_index++;
}

void movingLeftImage(int x, int y) {
	if (historyManager.now.image_index >= left_images.size()){
		historyManager.now.image_index = 0;
	}

	// opencv 
	opencvAction(left_mats[historyManager.now.image_index], x, y);


	historyManager.now.image_index++;
}

void movingRightImage(int x, int y) {
	if (historyManager.now.image_index >= right_images.size()){
		historyManager.now.image_index = 0;
	}

	// opencv 
	opencvAction(right_mats[historyManager.now.image_index], x, y);

	historyManager.now.image_index++;
}

void handleMovingAnimation() {

	// exception of handling moving animation here
	if (current_state == EXCLAMATING_STATE){
		// exclamating and stand
		standImage(historyManager.now.x, historyManager.now.y);
		return;
	}

	double a2 = moving_distance_per_frame * moving_distance_per_frame;
	bool reach_dream = (historyManager.now.x == historyManager.dream.x && historyManager.now.y == historyManager.dream.y);
	double moved_x = 0.0;
	double moved_y = 0.0;
	double moved_dist = 0.0;
	int moving_flag = 0; // use to control move to x-axis or y-axis
	while (!reach_dream){

		if (historyManager.now.y == historyManager.dream.y){
			// It's already reach y, next it has to move x-axis to in order to achieve dream
			moving_flag = 0;
		}
		else if (historyManager.now.x == historyManager.dream.x){
			// It's already reach x, next it has to move y-axis to in order to achieve dream
			moving_flag = 1;
		}

		if (moving_flag == 1){
			// move to y
			if (historyManager.now.y - historyManager.dream.y > 0){
				historyManager.now.y--;
				moved_y--;
			}
			else{
				historyManager.now.y++;
				moved_y++;
			}
			moving_flag = 0;
		}
		else {
			// move to x
			if (historyManager.now.x - historyManager.dream.x > 0){
				historyManager.now.x--;
				moved_x--;
			}
			else{
				historyManager.now.x++;
				moved_x++;
			}
			moving_flag = 1;
		}
		moved_dist = (moved_x * moved_x) + (moved_y * moved_y);
		reach_dream = (historyManager.now.x == historyManager.dream.x && historyManager.now.y == historyManager.dream.y);
		if (moved_dist >= a2){
			break;
		}
	}

	// normal state
	if (moved_y > 0){
		// moving right and down
		movingForwardImage(historyManager.now.x, historyManager.now.y);
	}
	else if (moved_y == 0 && moved_x == 0){
		// stand
		standImage(historyManager.now.x, historyManager.now.y);
	}
	else if (moved_y < 0){
		// move up
		movingBackwardImage(historyManager.now.x, historyManager.now.y);
	}
	else if (moved_x > 0){
		// moving right
		movingRightImage(historyManager.now.x, historyManager.now.y);
	}
	else if (moved_x < 0){
		// moving left
		movingLeftImage(historyManager.now.x, historyManager.now.y);
	}
	
}

void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nIDEvent, DWORD dwTime)
{
	// handling state
	handleAction();
	
	// begin to moving
	if (show_moving_animation_flag  != STOP_MOVING_ANIMATION)
		handleMovingAnimation();

#ifdef DEBUG_MODE
	// DEBUG
	if (current_state == NORMAL_STATE){
		printf("NORMAL_STATE\tNOW:%d %d, DREAM:%d %d\n", historyManager.now.y, historyManager.now.x, historyManager.dream.y, historyManager.dream.x);
	}
	else if(current_state == INTRODUCING_STATE){
		if (somebody_flag == SOMEBODY_FLAG){
			printf("SOMEBODY,\tNOW:%d %d, DREAM:%d %d, CURRENT_INTRO: %d\n", historyManager.now.y, historyManager.now.x, historyManager.dream.y, historyManager.dream.x, introducing_current_frame_index);
		}
		else{
			printf("NOBODY, \tNOW:%d %d, DREAM:%d %d, CURRENT_INTRO: %d\n", historyManager.now.y, historyManager.now.x, historyManager.dream.y, historyManager.dream.x, introducing_current_frame_index);
		}
	}
	else{
		printf("UNKNOWN \tNOW:%d %d, DREAM:%d %d\n", historyManager.now.y, historyManager.now.x, historyManager.dream.y, historyManager.dream.x);

	}
#endif
	
}


int main(int argc, const char *argv[]) {
	
	const char *const cascade = "haarcascade_frontalface_alt.xml";
	

	//std::string rootDirectory = "C:/Users/LikWee - PC/Documents/Visual Studio 2013/Projects/passerby_tracking/passerby_tracking/";
	//rootDirectory.append(right_images[1]);
	
	// webcam
	CvCapture *capture = cvCreateCameraCapture(0);
	if (capture == NULL) return -1;
	IplImage* frame;
	// webcam property configuration
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_WIDTH);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_HEIGHT);

#ifdef DEBUG_MODE
	cvNamedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE);
#else
	// window for full screen
	cvNamedWindow(WINDOW_NAME, CV_WINDOW_NORMAL);
	cvSetWindowProperty(WINDOW_NAME, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
#endif


	// initial historyManager
	historyManager.now.y = 200;
	historyManager.now.x = 0;

	// 正面顔検出器の読み込み
	CvHaarClassifierCascade* cvHCC = (CvHaarClassifierCascade*)cvLoad(cascade);
	if (cvHCC == NULL){
		return -2;
	}

	// 検出に必要なメモリストレージを用意する
	CvMemStorage* cvMStr = cvCreateMemStorage(0);
	// 検出情報を受け取るためのシーケンスを用意する
	CvSeq* face;

	// 背景を飾る
	background.setTo(Scalar(1, 1, 1));
	Mat logoImage = cvLoadImage(logo_image.c_str(), -1);
	overlayImage(background, logoImage, background, cv::Point(WINDOW_WIDTH - logoImage.cols - 10, WINDOW_HEIGHT - logoImage.rows - 10));

	loadMats();

	// timer
	UINT TimerId = SetTimer(NULL, 100, ANIMATION_PERIOD, &TimerProc);
	if (!TimerId) return 0;

	while (1) {

		frame = cvQueryFrame(capture);

#ifdef DEBUG_MODE
		background.setTo(Scalar(1, 1, 1));
#endif

		if (frame != NULL){
			// 画像中から検出対象の情報を取得する
			face = cvHaarDetectObjects(frame, cvHCC, cvMStr);
			CvRect* faceRect;
			somebody_flag = NOBODY_FLAG;
			for (int i = 0; i < face->total; i++) {
				// 検出情報から顔の位置情報を取得
				faceRect = (CvRect*)cvGetSeqElem(face, i);

				if (faceRect->width > 100 && faceRect->height > 100){
					somebody_flag = SOMEBODY_FLAG;
				}

				//cvRectangle(a,
				//				cvPoint(faceRect->x, faceRect->y),
				//				cvPoint(faceRect->x + faceRect->width, faceRect->y + faceRect->height),
				//				CV_RGB(255, 0 ,0),
				//				2, CV_AA);

				// resize and overlay image with 

#ifdef DEBUG_MODE
				// 取得した顔の位置情報に基づき、矩形描画を行う
				rectangle(background, Point(faceRect->x, faceRect->y), Point(faceRect->x + faceRect->width, faceRect->y + faceRect->height), Scalar(255, 0, 0), 3, 4);
#endif
			}
		}

		char c = cvWaitKey(200);
		if (c == 27) break;
	}

	// release all memory
	// 用意したメモリストレージを解放
	cvReleaseMemStorage(&cvMStr);
	// カスケード識別器の解放
	cvReleaseHaarClassifierCascade(&cvHCC);
	cvReleaseCapture(&capture);
	cvDestroyWindow(WINDOW_NAME);
}
