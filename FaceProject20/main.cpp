#include <direct.h>
#include <iostream>
#include <stdarg.h>
#include <string>
#include <opencv.hpp>
#include <thread>
#include <sstream>
#include <time.h>

#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"

// 自定义
#include "utils.h"


using namespace std;
using namespace cv;

MHandle handle = NULL;
Mat frame;
VideoCapture capture;

bool faceNumChanged = false;
MInt32 tempFaceNum = 0;

ASF_MultiFaceInfo detectedFaces = { 0 };
ASF_GenderInfo genderInfo = { 0 };
ASF_AgeInfo ageInfo = { 0 };
ASF_Face3DAngle angleInfo = { 0 };

IplImage* img = cvLoadImage("netPhoto1.bmp");
IplImage* img1 = cvLoadImage("huanglei.bmp");

/* 监测人脸信息 */
void faceInfo() {

	MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE;
	MRESULT genderRes = MERR_UNKNOWN;
	MRESULT ageRes = MERR_UNKNOWN;
	MRESULT angleRes = MERR_UNKNOWN;
	clock_t time1 = clock();
	clock_t time2 = clock();
	bool timeUpdate = false;
	while (true)
	{
		/* 每一秒更改timeUpdate为true */
		if (time2 - time1 >= CLOCKS_PER_SEC)
		{
			timeUpdate = true;
			time1 = time2;
		}
		/* 激活功能的条件：1.人脸数发生了变化 2.人脸数未变化，但是持续时间大于1s */
		if (faceNumChanged || (detectedFaces.faceNum > 0 && timeUpdate == true))
		{
			ASFProcess(handle, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)frame.data, &detectedFaces, processMask);
			genderRes = ASFGetGender(handle, &genderInfo);
			ageRes = ASFGetAge(handle, &ageInfo);
			angleRes = ASFGetFace3DAngle(handle, &angleInfo);
			if (angleRes == MOK && ageRes == MOK && genderRes == MOK)
				if (ageInfo.num > 0)
				{
					for (int i = 0; i < ageInfo.num; i++)
					{
						cout << ageInfo.ageArray[i] << endl;
					}
					
				}
		}
		time2 = clock();
		timeUpdate = false;
	}
}

/* 画人脸框 */
void drawRect() {
	
	while (true)
	{
		if (detectedFaces.faceNum > 0)
		{
			for (int i = 0; i < detectedFaces.faceNum; i++)
			{
				cv::rectangle(frame, CvPoint(detectedFaces.faceRect[i].left, detectedFaces.faceRect[i].top), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].bottom), CvScalar(255, 0, 0), 2);	
			}
		}
		
	}
}

/* 打印年龄，性别，角度信息 */
void printInfo() {
	stringstream text;
	string sex[3] = { "unknown", "male", "female" };
	while (true)
	{
		if (genderInfo.num > 0 && ageInfo.num > 0)
		{
			for (int i = 0; i < genderInfo.num; i++)
			{
				text << "sex:" << sex[genderInfo.genderArray[i] + 1] << "age:" << ageInfo.ageArray[i];
				cv::putText(frame, text.str(), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].bottom), cv::FONT_HERSHEY_SIMPLEX, 0.7, (55, 255, 155), 2);
				text.str("");
			}
		}
		if (angleInfo.num > 0)
		{
			for (int i = 0; i < angleInfo.num; i++)
			{
				text << "roll:" << angleInfo.roll[i];
				//<< "yaw:" << angleInfo.yaw[i] << "pitch:" << angleInfo.pitch[i] << "status:" << angleInfo.status[i];
				cv::putText(frame, text.str(), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].top), cv::FONT_HERSHEY_SIMPLEX, 0.7, (55, 255, 155), 2);
				text.str("");
			}
		}
	}
	
}

void init() {
	/* 激活sdk */
	MRESULT activationRes = ASFActivation(APPID, SDKKey);
	if (MOK != activationRes && MERR_ASF_ALREADY_ACTIVATED != activationRes)
	{
		printf("ALActivation fail: %d\n", activationRes);
	}
	else
	{
		printf("ALActivation sucess: %d\n", activationRes);
	}

	/* 初始化引擎 */
	MInt32 mask = ASF_FACE_DETECT | ASF_FACERECOGNITION | ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE;
	MRESULT initRes = ASFInitEngine(ASF_DETECT_MODE_VIDEO, ASF_OP_0_ONLY, 16, 5, mask, &handle);
	if (initRes != MOK)
	{
		printf("ALInitEngine fail: %d\n", initRes);
	}
	else
	{
		printf("ALInitEngine sucess: %d\n", initRes);
	}

	/* 打开摄像头 */
	capture.open(0);
	if (!capture.isOpened())
	{
		cout << "摄像头未打开" << endl;
		return ;
	}
}

void run() {

	init();

	/* 线程创建 */
	thread rectThread(drawRect);
	rectThread.detach();
	thread faceInfoThread(faceInfo);
	faceInfoThread.detach();
	thread printInfoThread(printInfo);
	printInfoThread.detach();

	while (true)
	{

		capture >> frame;
		ASFDetectFaces(handle, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, frame.data, &detectedFaces);

		if (detectedFaces.faceNum == tempFaceNum)
		{
			faceNumChanged = false;
		}
		else
		{
			faceNumChanged = true;
		}
		tempFaceNum = detectedFaces.faceNum;

		imshow("video", frame);
		if (waitKey(30) >= 0)
		{
			break;
		}

	}
}

void unInit() {
	MRESULT uninitRes = ASFUninitEngine(handle);
	if (uninitRes != MOK)
		printf("ALUninitEngine fail: %d\n", uninitRes);
	else
		printf("ALUninitEngine sucess: %d\n", uninitRes);
}

int main() 
{
	run();

	return 0;

}