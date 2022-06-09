#include "Param_WR.h"

using namespace std;


Param_WR::Param_WR()
{
}


Param_WR::~Param_WR()
{
}

// ch:��ӡ�������������Ϣ
void Param_WR::PrintCameraParam(const CameraParam* const camera_p)
{
	cout << "������кţ�" << camera_p->SerialNumber << endl;
	cout << "���ͼ���ʽ��" << camera_p->PixelFormat << endl;
	cout << "����ع�ʱ�䣺" << camera_p->ExposureTime << endl;
	cout << "������棺" << camera_p->Gain << endl;
	cout << endl;
}

// ch:д���������������Ϣ
bool Param_WR::SetCameraParam(string location, CameraParam* left_camera, CameraParam* right_camera)
{
	cout << endl;
	cout << "**********************************************************" << endl;
	cout << "                  ��ǰ���������Ϣ" << endl;
	cout << "�����������" << endl;
	PrintCameraParam(left_camera);
	cout << "�����������" << endl;
	PrintCameraParam(right_camera);
	cout << "**********************************************************" << endl;
	unsigned char* SN_L = left_camera->SerialNumber;
	unsigned char* SN_R = right_camera->SerialNumber;
	// ����XML�ĵ�ָ��
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc == NULL)
	{
		cout << "xml�ļ�ָ�봴��ʧ�ܣ�" << endl;
		return 0;
	}
	// ����XML
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "GBK", "");
	if (decl == NULL)
	{
		return 0;
	}
	doc->LinkEndChild(decl);
	// ����XML��ע
	TiXmlComment* pComment = new TiXmlComment("�洢�������������Ϣ");
	doc->LinkEndChild(pComment);

	TiXmlElement *root, *left, *child_l, *right, *child_r;
	TiXmlText *text;
	// �������ڵ�
	root = new TiXmlElement("Parameters");
	doc->LinkEndChild(root);

	// ���������д��
	left = new TiXmlElement("Left");
	left->SetAttribute("SN", (char*)SN_L);
	root->LinkEndChild(left);
	//// ͼ���ʽ
	child_l = new TiXmlElement("PixelFormat");
	text = new TiXmlText(left_camera->PixelFormat.data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);
	//// �ع�ʱ��
	child_l = new TiXmlElement("ExposureTime");
	text = new TiXmlText(ftos(left_camera->ExposureTime).data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);
	//// ����
	child_l = new TiXmlElement("Gain");
	text = new TiXmlText(ftos(left_camera->Gain).data());
	child_l->LinkEndChild(text);
	left->LinkEndChild(child_l);

	// ���������д��
	right = new TiXmlElement("Right");
	right->SetAttribute("SN", (char*)SN_R);
	root->LinkEndChild(right);
	//// ͼ���ʽ
	child_r = new TiXmlElement("PixelFormat");
	text = new TiXmlText(right_camera->PixelFormat.data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	//// �ع�ʱ��
	child_r = new TiXmlElement("ExposureTime");
	text = new TiXmlText(ftos(right_camera->ExposureTime).data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	//// ����
	child_r = new TiXmlElement("Gain");
	text = new TiXmlText(ftos(right_camera->Gain).data());
	child_r->LinkEndChild(text);
	right->LinkEndChild(child_r);
	// ������ļ�
	doc->SaveFile(location.data());
	return 1;
}

// ch:��ȡ�������������Ϣ
void Param_WR::ReadCameraParam(TiXmlDocument &camera_param, CameraParam* left_camera, CameraParam* right_camera)
{
	TiXmlElement* root = camera_param.RootElement();
	// ��ȡ���������
	TiXmlElement* left = root->FirstChildElement("Left");
	const char* SN_L = left->Attribute("SN");
	memcpy(left_camera->SerialNumber, SN_L, sizeof(left_camera->SerialNumber));
	TiXmlElement* child_l = left->FirstChildElement();
	left_camera->PixelFormat = child_l->GetText();
	child_l = child_l->NextSiblingElement();
	left_camera->ExposureTime = atof(child_l->GetText());
	child_l = child_l->NextSiblingElement();
	left_camera->Gain = atof(child_l->GetText());

	// ��ȡ���������
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

// ch:��ӡ�궨����������Ϣ
void Param_WR::PrintCalibParam(const CalibPattern* const calib_p)
{
	cout << endl;
	cout << "**********************************************************" << endl;
	cout << "                  ��ǰ�궨������Ϣ" << endl;
	cout << "ͼ�������ļ��У�" << calib_p->image_folder << endl;
	cout << "����Ŀ¼��" << calib_p->cal_debug_folder << endl;
	cout << "��������������" << calib_p->sqXnum << endl;
	cout << "��������������" << calib_p->sqYnum << endl;
	cout << "�������ࣨʵ������߶ȣ���" << calib_p->TargetSize << endl;
	cout << "�궨ͼ�����ͣ�";
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
		cout << "��ʼ��־��X���꣺" << calib_p->origin_offset_x << endl;
		cout << "��ʼ��־��Y���꣺" << calib_p->origin_offset_y << endl;
		cout << "��ʼ�㵽�ҷ���־����룺" << calib_p->inner_pattern_width << endl;
		cout << "��ʼ�㵽�Ϸ���־����룺" << calib_p->inner_pattern_height << endl;
		break;
	}
	default:
		cout << "NO_SUCH_TARGET_TYPE" << endl;
		break;
	}
	cout << endl;
	cout << "**********************************************************" << endl;
}

// ch:д��궨����������Ϣ
bool Param_WR::SetCalibParam(string location, CalibPattern* calib_p)
{
	
	PrintCalibParam(calib_p);
	// ����XML�ĵ�ָ��
	TiXmlDocument* doc = new TiXmlDocument();
	if (doc == NULL)
	{
		cout << "xml�ļ�ָ�봴��ʧ�ܣ�" << endl;
		return 0;
	}
	// ����XML
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "GBK", "");
	if (decl == NULL)
	{
		return 0;
	}
	doc->LinkEndChild(decl);
	// ����XML��ע
	TiXmlComment* pComment = new TiXmlComment("�洢�������������Ϣ");
	doc->LinkEndChild(pComment);
	TiXmlElement *root, *child;
	TiXmlText *text;
	// �������ڵ�
	root = new TiXmlElement("Parameters");
	doc->LinkEndChild(root);

	// ͼ�������ļ���
	child = new TiXmlElement("Image_Folder");
	text = new TiXmlText(calib_p->image_folder.data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// ����Ŀ¼
	child = new TiXmlElement("Cal_Debug_Folder");
	text = new TiXmlText(calib_p->cal_debug_folder.data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// ������������
	child = new TiXmlElement("sqXnum");
	text = new TiXmlText(itos(calib_p->sqXnum).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// ������������
	child = new TiXmlElement("sqYnum");
	text = new TiXmlText(itos(calib_p->sqYnum).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// ��������
	child = new TiXmlElement("TargetSize");
	text = new TiXmlText(ftos(calib_p->TargetSize).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// �궨ͼ������
	child = new TiXmlElement("PatternType");
	text = new TiXmlText(pattern_to_string(calib_p->PatternType).data());
	child->LinkEndChild(text);
	root->LinkEndChild(child);
	// ���궨ͼ������ΪCALIB_PATTERN_SPECIAL��Ҫ�������
	if (calib_p->PatternType == CALIB_PATTERN_SPECIAL) {
		// ��ʼ��־��X����
		child = new TiXmlElement("origin_offset_x");
		text = new TiXmlText(itos(calib_p->origin_offset_x).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// ��ʼ��־��Y����
		child = new TiXmlElement("origin_offset_y");
		text = new TiXmlText(itos(calib_p->origin_offset_y).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// ��ʼ�㵽�ҷ���־�����
		child = new TiXmlElement("inner_pattern_width");
		text = new TiXmlText(itos(calib_p->inner_pattern_width).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		// ��ʼ�㵽�Ϸ���־�����
		child = new TiXmlElement("inner_pattern_height");
		text = new TiXmlText(itos(calib_p->inner_pattern_height).data());
		child->LinkEndChild(text);
		root->LinkEndChild(child);
		

	}
	//// ������ļ�
	doc->SaveFile(location.data());
	return 1;
}

// ch:��ȡ�궨����������Ϣ
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
	// ���궨ͼ������ΪCALIB_PATTERN_SPECIAL��Ҫ�������
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

// ch:д�����νǵ���Ϣ 
bool WritePolyline(string location, const vector<Point> &roi_polyline)
{
	return true;
}


// ch:��ȡ����νǵ���Ϣ 
bool ReadPolyline(TiXmlDocument &polylinepoint, vector<Point> &roi_polyline)
{
	return true;
}


