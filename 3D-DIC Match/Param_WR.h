// 名称：相机参数读写
// 作者：吴文浩
// 创建时间：2021.7.11
#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <ctype.h>
#include <sstream>

// xml读写依赖项
#include "tinyxml.h"
#include "tinystr.h"

// 常用函数库
#include "CommonFunction.h"


using namespace std;

// ch:相机图像格式定义
static unordered_map<unsigned int, string> MV_CC_PixelFormat = {
	{ 0x01080001, "Mono8" },
	{ 0x01100003, "Mono10" },
	{ 0x010C0004, "Mono10Packed" },
	{ 0x01100005, "Mono12" },
	{ 0x010C0006, "Mono12Packed" },
};

// ch:相机信息结构体
typedef struct CameraParam
{
	unsigned char	SerialNumber[64];	// 序列号
	string			PixelFormat;		// 图像格式
	float			ExposureTime;		// 曝光时间
	float			Gain;				// 增益

}CameraParam;


class Param_WR
{
public:
	Param_WR();
	~Param_WR();

	// ch:打印相机参数配置信息
	static void PrintCameraParam(const CameraParam* const camera_p);
	
	// ch:写入相机参数配置信息
	static bool SetCameraParam(string location, CameraParam* left_camera, CameraParam* right_camera);

	// ch:读取相机参数配置信息
	static void ReadCameraParam(TiXmlDocument &camera_param, CameraParam* left_camera, CameraParam* right_camera);


	// ch:打印标定参数配置信息
	static void PrintCalibParam(const CalibPattern* const camera_p);

	// ch:写入标定参数配置信息
	static bool SetCalibParam(string location, CalibPattern* calib_p);

	// ch:读取标定参数配置信息
	static bool ReadCalibParam(TiXmlDocument &calib_param, CalibPattern* calib_p);
	
	// ch:写入多边形角点信息 
	static bool WritePolyline(string location, const vector<Point> &roi_polyline);

	// ch:读取多边形角点信息 
	static bool ReadPolyline(TiXmlDocument &polylinepoint, vector<Point> &roi_polyline);

};

