#include "LocalFaceModels.h"

void generateList(MHandle handle, IplImage* img)
{
	
}

ASF_FaceFeature getFeatureFromBmp(MHandle handle, IplImage* img)
{
	ASF_MultiFaceInfo detectedFaces1 = { 0 };
	ASF_SingleFaceInfo SingleDetectedFaces1 = { 0 };
	ASF_FaceFeature feature1 = { 0 };
	ASF_FaceFeature copyfeature1 = { 0 };
	MRESULT res = ASFDetectFaces(handle, img->width, img->height, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)img->imageData, &detectedFaces1);
	if (MOK == res)
	{
		SingleDetectedFaces1.faceRect.left = detectedFaces1.faceRect[0].left;
		SingleDetectedFaces1.faceRect.top = detectedFaces1.faceRect[0].top;
		SingleDetectedFaces1.faceRect.right = detectedFaces1.faceRect[0].right;
		SingleDetectedFaces1.faceRect.bottom = detectedFaces1.faceRect[0].bottom;
		SingleDetectedFaces1.faceOrient = detectedFaces1.faceOrient[0];

		res = ASFFaceFeatureExtract(handle, img->width, img->height, ASVL_PAF_RGB24_B8G8R8, (MUInt8*)img->imageData, &SingleDetectedFaces1, &feature1);
		if (res == MOK)
		{
			//¿½±´feature
			copyfeature1.featureSize = feature1.featureSize;
			copyfeature1.feature = (MByte *)malloc(feature1.featureSize);
			memset(copyfeature1.feature, 0, feature1.featureSize);
			memcpy(copyfeature1.feature, feature1.feature, feature1.featureSize);
		}
		else
			printf("ASFFaceFeatureExtract 1 fail: %d\n", res);
	}
	else
		printf("ASFDetectFaces 1 fail: %d\n", res);
	return copyfeature1;
}


