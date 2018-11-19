#include <direct.h>
#include <iostream>
#include <stdarg.h>
#include <string>
#include <opencv.hpp>
#include <thread>
#include <sstream>
#include <windows.h>


#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"

// �Զ���
#include "utils.h"
#include "LocalFaceModels.h"


using namespace std;
using namespace cv;

MHandle handle = NULL;
Mat frame;
VideoCapture capture;

bool faceNumChanged = false;
MInt32 tempFaceNum = 0;
stringstream textToFrame;
string sex[3] = { "unknown", "male", "female" };

ASF_MultiFaceInfo detectedFaces = { 0 };
ASF_GenderInfo genderInfo = { 0 };
ASF_AgeInfo ageInfo = { 0 };
ASF_Face3DAngle angleInfo = { 0 };

ASF_SingleFaceInfo singleFaceRes[4] = { 0 };

ASF_FaceFeature singleFeature[4] = { 0 };
ASF_FaceFeature videoFeature[4] = { 0 };

vector<Person> personList;

IplImage* img = cvLoadImage("netPhoto1.bmp");
IplImage* img1 = cvLoadImage("huanglei.bmp");

MFloat confidenceLevel;
bool lock = false;

/* ���������Ϣ */
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
		/* ÿһ�����timeUpdateΪtrue */
		if (time2 - time1 >= CLOCKS_PER_SEC)
		{
			timeUpdate = true;
			time1 = time2;
		}
		/* ����ܵ�������1.�����������˱仯 2.������δ�仯�����ǳ���ʱ�����1s */
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

/* �������� */
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

/* ��ӡ���䣬�Ա𣬽Ƕ���Ϣ */
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
	/* ����sdk */
	MRESULT activationRes = ASFActivation(APPID, SDKKey);
	if (MOK != activationRes && MERR_ASF_ALREADY_ACTIVATED != activationRes)
	{
		printf("ALActivation fail: %d\n", activationRes);
	}
	else
	{
		printf("ALActivation sucess: %d\n", activationRes);
	}

	/* ��ʼ������ */
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

	/* ������ͷ */
	capture.open(0);
	if (!capture.isOpened())
	{
		cout << "����ͷδ��" << endl;
		return ;
	}
}

void run() {

	init();

	///* �̴߳��� */
	//thread rectThread(drawRect);
	//rectThread.detach();
	//thread faceInfoThread(faceInfo);
	//faceInfoThread.detach();
	//thread printInfoThread(printInfo);
	//printInfoThread.detach();

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

void generateLocalFeatureList() {
	
	personList.reserve(100000);

	ASF_FaceFeature tempFaceFeature =  getFeatureFromBmp(handle, img);
	cout << "sizeOfFeature" << tempFaceFeature.featureSize << endl;
	Person person;
	person.faceFeature.featureSize = tempFaceFeature.featureSize;
	person.faceFeature.feature = (MByte *)malloc(tempFaceFeature.featureSize);
	memset(person.faceFeature.feature, 0, tempFaceFeature.featureSize);
	memcpy(person.faceFeature.feature, tempFaceFeature.feature, tempFaceFeature.featureSize);
	for (int i = 0; i < 99990; i++)
	{
		person.id = i;
		personList.push_back(person);
	}
	tempFaceFeature = getFeatureFromBmp(handle, img1);
	person.faceFeature = tempFaceFeature;
	person.id = 99990;
	personList.push_back(person);
}

void faceRecognization(ASF_FaceFeature feature)
{
	for (int i = 0; i < personList.size(); i++)
	{
		MRESULT res = ASFFaceFeatureCompare(handle, &feature, &personList.at(i).faceFeature, &confidenceLevel);
		if (confidenceLevel > 0.6)
		{
			cout << personList.at(i).id << endl;
			cout << confidenceLevel << endl;
		}
	}
}

void getFeatureFromVideo(Mat frame, ASF_MultiFaceInfo videoFaceRes) {
	double startTime = GetTickCount();
	ASF_FaceFeature videoFeature = { 0 };
	for (int i = 0; i < 4 && i< videoFaceRes.faceNum; i++)
	{
		singleFaceRes[i].faceRect.left = videoFaceRes.faceRect[i].left;
		singleFaceRes[i].faceRect.top = videoFaceRes.faceRect[i].top;
		singleFaceRes[i].faceRect.right = videoFaceRes.faceRect[i].right;
		singleFaceRes[i].faceRect.bottom = videoFaceRes.faceRect[i].bottom;
		singleFaceRes[i].faceOrient = videoFaceRes.faceOrient[i];
	}
	
	for (int i = 0; i < videoFaceRes.faceNum; i++)
	{
		MRESULT res = ASFFaceFeatureExtract(handle, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)frame.data, &singleFaceRes[i], &videoFeature);
		if (res == MOK)
		{
			//����feature
			singleFeature[i].featureSize = videoFeature.featureSize;
			singleFeature[i].feature = (MByte *)malloc(videoFeature.featureSize);
			memset(singleFeature[i].feature, 0, videoFeature.featureSize);
			memcpy(singleFeature[i].feature, videoFeature.feature, videoFeature.featureSize);
		}
		else
			printf("ASFFaceFeatureExtract 1 fail: %d\n", res);
	}
	
	faceRecognization(singleFeature[0]);
	double endTime = GetTickCount();
	cout << "time consume" << (endTime - startTime) << endl;
	
}





int main() 
{
	MInt32 processMask = ASF_AGE | ASF_GENDER | ASF_FACE3DANGLE;
	MRESULT genderRes = MERR_UNKNOWN;
	MRESULT ageRes = MERR_UNKNOWN;
	MRESULT angleRes = MERR_UNKNOWN;

	init();

	generateLocalFeatureList();
	cout << sizeof(Person) << endl;

	
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

		/* FaceInfo��� ����ܵ�������1.�����������˱仯 2.����������0 */
		if (faceNumChanged || detectedFaces.faceNum > 0)
		{
			ASFProcess(handle, frame.cols, frame.rows, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)frame.data, &detectedFaces, processMask);
			genderRes = ASFGetGender(handle, &genderInfo);
			ageRes = ASFGetAge(handle, &ageInfo);
			angleRes = ASFGetFace3DAngle(handle, &angleInfo);
			//if (angleRes == MOK && ageRes == MOK && genderRes == MOK)
			//	if (ageInfo.num > 0)
			//	{
			//		for (int i = 0; i < ageInfo.num; i++)
			//		{
			//			cout << ageInfo.ageArray[i] << endl;
			//		}
			//	}
		}

		/* ���������� */
		if (detectedFaces.faceNum > 0)
		{
			for (int i = 0; i < detectedFaces.faceNum; i++)
			{
				cv::rectangle(frame, CvPoint(detectedFaces.faceRect[i].left, detectedFaces.faceRect[i].top), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].bottom), CvScalar(255, 0, 0), 2);
			}
		}

		/* ��ӡ�Ա�������*/
		if (genderInfo.num > 0 && ageInfo.num > 0)
		{
			for (int i = 0; i < genderInfo.num; i++)
			{
				textToFrame << "sex:" << sex[genderInfo.genderArray[i] + 1] << "age:" << ageInfo.ageArray[i];
				cv::putText(frame, textToFrame.str(), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].bottom), cv::FONT_HERSHEY_SIMPLEX, 0.7, (55, 255, 155), 2);
				textToFrame.str("");
			}
		}

		/* ��ӡ�Ƕ���Ϣ*/
		if (angleInfo.num > 0)
		{
			for (int i = 0; i < angleInfo.num; i++)
			{
				textToFrame << "pitch:" << angleInfo.pitch[i] << "yaw:" << angleInfo.yaw[i] << "roll:" << angleInfo.roll[i];
				//<< "yaw:" << angleInfo.yaw[i] << "pitch:" << angleInfo.pitch[i] << "status:" << angleInfo.status[i];
				cv::putText(frame, textToFrame.str(), CvPoint(detectedFaces.faceRect[i].right, detectedFaces.faceRect[i].top), cv::FONT_HERSHEY_SIMPLEX, 0.7, (55, 255, 155), 2);
				textToFrame.str("");
			}
		}

		imshow("video", frame);
		if (waitKey(50) >= 0)
		{
			thread td1(getFeatureFromVideo,frame, detectedFaces);
			td1.detach();
		}

	}


	unInit();
	system("pause");
	return 0;

}