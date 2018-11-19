#pragma once
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

#include "inc/amcomdef.h"
#include "inc/arcsoft_face_sdk.h"
#include "inc/asvloffscreen.h"
#include "inc/merror.h"

using namespace std;

typedef struct Person{
int id;
ASF_FaceFeature faceFeature;
}Person, *pPerson;

ASF_FaceFeature getFeatureFromBmp(MHandle handle, IplImage* img);
