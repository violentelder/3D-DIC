#include "Param_WR.h"

using namespace std;


Param_WR::Param_WR()
{
}


Param_WR::~Param_WR()
{
}

// ch:打印相机参数配置信息
void Param_WR::PrintCameraParam(const CameraParam* const camera_p)
{
	cout << "相机序列号：" << camera_p->SerialNumber << endl;
	cout << "相机图像格式：" << camera_p->PixelFormat << endl;
	cout << "相机曝光时间：" << camera_p->ExposureTime << endl;
	cout << "相机增益：" << camera_p->Gain << endl;
	cout << endl;
}

// ch:写入相机参数配置信息
bool Param_WR::SetCameraParam(string location, CameraParam* left_camera, CameraParam* right_camera)
{
	cout << endl;
	cout << "**********************************************************" << endl;
	cout << "                  当前相机参数信息" << endl;
	cout << "左相机参数：" << endl;
	PrintCameraParam(left_camera);
	cout << "右相机参数：" << endl;
	PrintCameraParam(right_camera);
	cout << "**********************************************************" << endl;
	unsigned char* SN_L = left_camera->SerialNumber;
	unsigned char* SN_R = right_camera->SerialNumber;
	// 创建XML文档指针
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc == NULL)
	{
		cout << "xml文件指针创建失败！" << endl;
		return 0;
	}
	// 声明XML
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "GBK", "");
	if (decl == NULL)
	{
		return 0;
	}
	doc->LinkEndChild(decl);
	// 创建XML备注
	TiXmlComment* pComment = new TiXmlComment("存储相机参数配置信息");
	doc->LinkEndChild(pComment);

	TiXmlElement *root, *left, *child_l, *right, *child_r;
	TiXmlText *text;
	// 创建根节点
	root = new TiXmlElement("Parameters");
	doc->LinkEndChild(root);

	// 左相机参数写入
	left = new TiXmlElement("Left");
	left->SetAttribute("SN", (char*)SN_L);
	root->LinkEndChild(left);
	//// 图像格式
	child_l = new TiXmlElement("PixelFormat");
	text = new TiXmlText(left_camera->PixelFormat.data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);
	//// 曝光时间
	child_l = new TiXmlElement("ExposureTime");
	text = new TiXmlText(ftos(left_camera->ExposureTime).data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);
	//// 增益
	child_l = new TiXmlElement("Gain");
	text = new TiXmlText(ftos(left_camera->Gain).data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);

	// 右相机参数写入
	right = new TiXmlElement("Right");
	right->SetAttribute("SN", (char*)SN_R);
	root->LinkEndChild(right);
	//// 图像格式
	child_r = new TiXmlElement("PixelFormat");
	text = new TiXmlText(right_camera->PixelFormat.data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	//// 曝光时间
	child_r = new TiXmlElement("ExposureTime");
	text = new TiXmlText(ftos(right_camera->ExposureTime).data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	//// 增益
	child_r = new TiXmlElement("Gain");
	text = new TiXmlText(ftos(right_camera->Gain).data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	// 输出到文件
	doc->SaveFile(location.data());
	return 1;
}

// ch:读取相机参数配置信息
void Param_WR::ReadCameraParam(TiXmlDocument &camera_param, CameraParam* left_camera, CameraParam* right_camera)
{
	TiXmlElement* root = camera_param.RootElement();
	// 读取左相机参数
	TiXmlElement* left = root->FirstChildElement("Left");
	const char* SN_L = left->Attribute("SN");
	memcpy(left_camera->SerialNumber, SN_L, sizeof(left_camera->SerialNumber));
	TiXmlElement* child_l = left->FirstChildElement();
	left_camera->PixelFormat = child_l->GetText();
	child_l = child_l->NextSiblingElement();
	left_camera->ExposureTime = atof(child_l->GetText());
	child_l = child_l->NextSiblingElement();
	left_camera->Gain = atof(child_l->GetText());

	// 读取右相机参数
	TiXmlElement* right = root->FirstChildElement("Right");
	const char* SN_R = right->Attribute("SN");
	memcpy(right_camera->SerialNumber, SN_R, sizeof(right_camera->SerialNumber));
	TiXmlElement* child_r = right->FirstChildElement();
	right_camera->PixelFormat = child_r->GetText();
	child_r = child_r->NextSiblingElement();
	right_camera->ExposureTime = atof(child_r->GetText());
	child_r = child_r->NextSiblingElement();
	right_camera->Gain = atof(child_r->GetText());
}

// ch:打印标定参数配置信息
void Param_WR::PrintCalibParam(const CalibPattern* const calib_p)
{
	cout << endl;
	cout << "**********************************************************" << endl;
	cout << "                  当前标定参数信息" << endl;
	cout << "图像输入文件夹：" << calib_p->image_folder << endl;
	cout << "调试目录：" << calib_p->cal_debug_folder << endl;
	cout << "横向特征点数：" << calib_p->sqXnum << endl;
	cout << "纵向特征点数：" << calib_p->sqYnum << endl;
	cout << "特征点间距（实际物理尺度）：" << calib_p->TargetSize << endl;
	cout << "标定图案类型：";
	switch (calib_p->PatternType) {
	case CALIB_PATTERN_CHECKERBOARD: {
		cout << "CALIB_PATTERN_CHECKERBOARD" << endl;
		break;
	}
	case CALIB_PATTERN_CIRCULAR: {
		cout << "CALIB_PATTERN_CIRCULAR" << endl;
		break;
	}
	case CALIB_PATTERN_SPECIAL: {
		cout << "CALIB_PATTERN_SPECIAL" << endl;
		cout << "起始标志点X坐标：" << calib_p->origin_offset_x << endl;
		cout << "起始标志点Y坐标：" << calib_p->origin_offset_y << endl;
		cout << "起始点到右方标志点距离：" << calib_p->inner_pattern_width << endl;
		cout << "起始点到上方标志点距离：" << calib_p->inner_pattern_height << endl;
		break;
	}
	default:
		cout << "NO_SUCH_TARGET_TYPE" << endl;
		break;
	}
	cout << endl;
	cout << "**********************************************************" << endl;
}

// ch:写入标定参数配置信息
bool Param_WR::SetCalibParam(string location, CalibPattern* calib_p)
{
	
	PrintCalibParam(calib_p);
	// 创建XML文档指针
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc == NULL)
	{
		cout << "xml文件指针创建失败！" << endl;
		return 0;
	}
	// 声明XML
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "GBK", "");
	if (decl == NULL)
	{
		return 0;
	}
	doc->LinkEndChild(decl);
	// 创建XML备注
	TiXmlComment* pComment = new TiXmlComment("存储相机参数配置信息");
	doc->LinkEndChild(pComment);
	TiXmlElement *root, *child;
	TiXmlText *text;
	// 创建根节点
	root = new TiXmlElement("Parameters");
	doc->LinkEndChild(root);

	// 图像输入文件夹
	child = new TiXmlElement("Image_Folder");
	text = new TiXmlText(calib_p->image_folder.data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 调试目录
	child = new TiXmlElement("Cal_Debug_Folder");
	text = new TiXmlText(calib_p->cal_debug_folder.data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 横向特征点数
	child = new TiXmlElement("sqXnum");
	text = new TiXmlText(itos(calib_p->sqXnum).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 纵向特征点数
	child = new TiXmlElement("sqYnum");
	text = new TiXmlText(itos(calib_p->sqYnum).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 特征点间距
	child = new TiXmlElement("TargetSize");
	text = new TiXmlText(ftos(calib_p->TargetSize).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 标定图案类型
	child = new TiXmlElement("PatternType");
	text = new TiXmlText(pattern_to_string(calib_p->PatternType).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// 当标定图案类型为CALIB_PATTERN_SPECIAL需要补充参数
	if (calib_p->PatternType == CALIB_PATTERN_SPECIAL) {
		// 起始标志点X坐标
		child = new TiXmlElement("origin_offset_x");
		text = new TiXmlText(itos(calib_p->origin_offset_x).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// 起始标志点Y坐标
		child = new TiXmlElement("origin_offset_y");
		text = new TiXmlText(itos(calib_p->origin_offset_y).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// 起始点到右方标志点距离
		child = new TiXmlElement("inner_pattern_width");
		text = new TiXmlText(itos(calib_p->inner_pattern_width).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// 起始点到上方标志点距离
		child = new TiXmlElement("inner_pattern_height");
		text = new TiXmlText(itos(calib_p->inner_pattern_height).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		

	}
	//// 输出到文件
	doc->SaveFile(location.data());
	return 1;
}

// ch:读取标定参数配置信息
bool Param_WR::ReadCalibParam(TiXmlDocument &calib_param, CalibPattern* calib_p)
{
	TiXmlElement* root = calib_param.RootElement();
	TiXmlElement* child = root->FirstChildElement();
	calib_p->image_folder = child->GetText();
	child = child->NextSiblingElement();
	calib_p->cal_debug_folder = child->GetText();
	child = child->NextSiblingElement();
	calib_p->sqXnum = atoi(child->GetText());
	child = child->NextSiblingElement();
	calib_p->sqYnum = atoi(child->GetText());
	child = child->NextSiblingElement();
	calib_p->TargetSize = atof(child->GetText());
	child = child->NextSiblingElement();
	string patterntype = child->GetText();
	calib_p->PatternType = string_to_target_type(patterntype);
	// 当标定图案类型为CALIB_PATTERN_SPECIAL需要补充参数
	if (calib_p->PatternType == CALIB_PATTERN_SPECIAL) {
		child = child->NextSiblingElement();
		calib_p->origin_offset_x = atoi(child->GetText());
		child = child->NextSiblingElement();
		calib_p->origin_offset_y = atoi(child->GetText());
		child = child->NextSiblingElement();
		calib_p->inner_pattern_width = atoi(child->GetText());
		child = child->NextSiblingElement();
		calib_p->inner_pattern_height = atoi(child->GetText());
	}
	else if (calib_p->PatternType == NO_SUCH_TARGET_TYPE)
		return 0;
	return 1;
}

// ch:写入多边形角点信息 
bool WritePolyline(string location, const vector<Point> &roi_polyline)
{
	return true;
}


// ch:读取多边形角点信息 
bool ReadPolyline(TiXmlDocument &polylinepoint, vector<Point> &roi_polyline)
{
	return true;
}


