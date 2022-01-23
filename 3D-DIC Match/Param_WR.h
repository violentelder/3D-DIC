// ���ƣ����������д
// ���ߣ����ĺ�
// ����ʱ�䣺2021.7.11
#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <ctype.h>
#include <sstream>

// xml��д������
#include "tinyxml.h"
#include "tinystr.h"

// ���ú�����
#include "CommonFunction.h"


using namespace std;

// ch:���ͼ���ʽ����
static unordered_map<unsigned int, string> MV_CC_PixelFormat = {
	{ 0x01080001, "Mono8" },
	{ 0x01100003, "Mono10" },
	{ 0x010C0004, "Mono10Packed" },
	{ 0x01100005, "Mono12" },
	{ 0x010C0006, "Mono12Packed" },
};

// ch:�����Ϣ�ṹ��
typedef struct CameraParam
{
	unsigned char	SerialNumber[64];	// ���к�
	string			PixelFormat;		// ͼ���ʽ
	float			ExposureTime;		// �ع�ʱ��
	float			Gain;				// ����

}CameraParam;


class Param_WR
{
public:
	Param_WR();
	~Param_WR();

	// ch:��ӡ�������������Ϣ
	static void PrintCameraParam(const CameraParam* const camera_p);
	
	// ch:д���������������Ϣ
	static bool SetCameraParam(string location, CameraParam* left_camera, CameraParam* right_camera);

	// ch:��ȡ�������������Ϣ
	static void ReadCameraParam(TiXmlDocument &camera_param, CameraParam* left_camera, CameraParam* right_camera);


	// ch:��ӡ�궨����������Ϣ
	static void PrintCalibParam(const CalibPattern* const camera_p);

	// ch:д��궨����������Ϣ
	static bool SetCalibParam(string location, CalibPattern* calib_p);

	// ch:��ȡ�궨����������Ϣ
	static bool ReadCalibParam(TiXmlDocument &calib_param, CalibPattern* calib_p);
	
	// ch:д�����νǵ���Ϣ 
	static bool WritePolyline(string location, const vector<Point> &roi_polyline);

	// ch:��ȡ����νǵ���Ϣ 
	static bool ReadPolyline(TiXmlDocument &polylinepoint, vector<Point> &roi_polyline);

};

