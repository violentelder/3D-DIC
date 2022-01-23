// ���ƣ��α��������
// ���ߣ����ĺ�
// ����ʱ�䣺2021.10.12

//ϵͳ������
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <Windows.h>
#include <process.h>
#include <conio.h>
#include <iostream>
#include <vector>

#include "MvCamera.h"
#include "Param_WR.h"


using namespace std;
using namespace cv;


// �����ʼ������
string camera_param_loc = "./camera_param.xml";			//�������·��
string algorithm_param_loc = "./algorithm_param.xml";	//�㷨����·��
string intrinsic_filename = "./intrinsics.xml";			//�ڲ��ļ�·��
string extrinsic_filename = "./extrinsics.xml";			//����ļ�·��
string output_loc = "./data/";							//����ͼ�����·��
vector<string> ImageName;


// �����ͼ����
unsigned char* m_pSaveImageBuf[2];
unsigned int m_nSaveImageBufSize[2];
MV_FRAME_OUT_INFO_EX  m_stImageInfo[2];


// ���ƥ���������Ʋ���
Mat temp_disp;
vector<Point> roi_polyline;
Point curpoint;
double sf = 1.0;
bool bFirstLButtenDown = true;
bool bDrawROIFinished = false;



bool g_bExit = false;
bool m_bFinishedShoot = false;
int imagenums = 1;
// ��ʾ����
HWND g_hwnd[2] = { NULL, NULL };
Mat srcImage[2];
// �������
string name[2] = { "��","��" };
string en_name[2] = { "left","right" };

// ch:�ȴ��������� | en:Wait for key press
char WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	return (char)_getch();
}

// ch:��ȡ���������Ϣ
bool GetCameraParam(CameraParam* camerap, MV_CC_DEVICE_INFO* pstDevInfo)
{

	int nRet = MV_OK;
	MvCamera* camera = new MvCamera;
	camera->Open(pstDevInfo);
	// ch:��ȡ������к�
	MVCC_STRINGVALUE* SN = new MVCC_STRINGVALUE;
	nRet = camera->GetStringValue("DeviceSerialNumber", SN);
	memcpy(camerap->SerialNumber, SN->chCurValue, sizeof(camerap->SerialNumber));

	// ch:��ȡ���ͼ���ʽ
	MVCC_ENUMVALUE* PF = new MVCC_ENUMVALUE;
	nRet = camera->GetEnumValue("PixelFormat", PF);
	camerap->PixelFormat = MV_CC_PixelFormat[PF->nCurValue];

	// ch:��ȡ����ع�ʱ��
	MVCC_FLOATVALUE* exp_time = new MVCC_FLOATVALUE;
	nRet = camera->GetFloatValue("ExposureTime", exp_time);
	camerap->ExposureTime = exp_time->fCurValue;

	// ch:��ȡ�������
	MVCC_FLOATVALUE* gain = new MVCC_FLOATVALUE;
	nRet = camera->GetFloatValue("Gain", gain);
	camerap->Gain = gain->fCurValue;

	// ch:�ر��豸 | Close device
	nRet = camera->Close();
	if (MV_OK != nRet)
	{
		printf("ClosDevice fail! nRet [0x%x]\n", nRet);
		return false;
	}
	return true;
}

// ch:��ӡ��������Ϣ
void PrintOptions()
{
	cout << endl;
	cout << "*******��ѡ���Ӧ����*******" << endl;
	cout << "1.�α�������ģʽ" << endl;
	cout << "2.�α�������ģʽ" << endl;
	cout << "3.�����������" << endl;
	cout << "4.�����㷨����" << endl;
	cout << "q.�˳��궨����" << endl << endl;
	cout << "ѡ���ܣ�";
}

// ch:����ͼ��
bool SaveImage(MvCamera *camera[])
{
	if (!m_bFinishedShoot) {
		cout << "���Ƚ�������" << endl;
		return FALSE;
	}
	MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
	memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));
	for (int i = 0; i < 2; i++) {
		if (m_pSaveImageBuf[i] == NULL || m_stImageInfo[i].enPixelType == 0)
		{
			cout << name[i] + "����޷�����" << endl;
			return FALSE;
		}
		stSaveFileParam.enImageType = MV_Image_Bmp; // ch:��Ҫ�����ͼ������ | en:Image format to save
		stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:�����Ӧ�����ظ�ʽ | en:Camera pixel type
		stSaveFileParam.nWidth = m_stImageInfo[i].nWidth;         // ch:�����Ӧ�Ŀ� | en:Width
		stSaveFileParam.nHeight = m_stImageInfo[i].nHeight;          // ch:�����Ӧ�ĸ� | en:Height
		stSaveFileParam.nDataLen = m_stImageInfo[i].nFrameLen;
		stSaveFileParam.pData = m_pSaveImageBuf[i];
		stSaveFileParam.iMethodValue = 0;
		string temp;
		if (imagenums< 10) {
			temp = en_name[i] + '0' + to_string(imagenums) + ".bmp";
		}
		else {
			temp = en_name[i] + to_string(imagenums) + ".bmp";
		}
		ImageName.push_back('\"' + temp + '\"');
		sprintf_s(stSaveFileParam.pImagePath, 256, (output_loc + temp).c_str());
		int nRet = camera[i]->SaveImageToFile(&stSaveFileParam);
		if (MV_OK != nRet)
		{
			cout << "����ͼƬʧ�ܣ�" << endl;
			return FALSE;
		}

	}
	imagenums++;
	return TRUE;
}
bool SaveImage(MvCamera *camera[], string Image_name)
{
	if (!m_bFinishedShoot) {
		cout << "���Ƚ�������" << endl;
		return FALSE;
	}
	MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
	memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));
	for (int i = 0; i < 2; i++) {
		if (m_pSaveImageBuf[i] == NULL || m_stImageInfo[i].enPixelType == 0)
		{
			cout << name[i] + "����޷�����" << endl;
			return FALSE;
		}
		stSaveFileParam.enImageType = MV_Image_Bmp; // ch:��Ҫ�����ͼ������ | en:Image format to save
		stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:�����Ӧ�����ظ�ʽ | en:Camera pixel type
		stSaveFileParam.nWidth = m_stImageInfo[i].nWidth;         // ch:�����Ӧ�Ŀ� | en:Width
		stSaveFileParam.nHeight = m_stImageInfo[i].nHeight;          // ch:�����Ӧ�ĸ� | en:Height
		stSaveFileParam.nDataLen = m_stImageInfo[i].nFrameLen;
		stSaveFileParam.pData = m_pSaveImageBuf[i];
		stSaveFileParam.iMethodValue = 0;
		string temp;
		temp = en_name[i] + '_' + Image_name;
		sprintf_s(stSaveFileParam.pImagePath, 256, (output_loc + temp).c_str());
		int nRet = camera[i]->SaveImageToFile(&stSaveFileParam);
		if (MV_OK != nRet)
		{
			cout << "����ͼƬʧ�ܣ�" << endl;
			return FALSE;
		}

	}
	return TRUE;
}

// ch:�����߳�
static  unsigned int __stdcall WorkThread(void* pUser)
{
	int nRet = MV_OK;
	MV_FRAME_OUT stImageInfo = { 0 };
	MV_DISPLAY_FRAME_INFO stDisplayInfo = { 0 };
	MvCamera* camera = (MvCamera*)pUser;
	while (1)
	{
		nRet = camera->GetImageBuffer(&stImageInfo, 1000000);
		if (nRet == MV_OK)
		{
			// ���ڴ�ͼ
			if (NULL == m_pSaveImageBuf[camera->index] || stImageInfo.stFrameInfo.nFrameLen > m_nSaveImageBufSize[camera->index])
			{
				if (m_pSaveImageBuf[camera->index])
				{
					free(m_pSaveImageBuf[camera->index]);
					m_pSaveImageBuf[camera->index] = NULL;
				}

				m_pSaveImageBuf[camera->index] = (unsigned char *)malloc(sizeof(unsigned char) * stImageInfo.stFrameInfo.nFrameLen);
				if (m_pSaveImageBuf[camera->index] == NULL)
				{
					return 0;
				}
				m_nSaveImageBufSize[camera->index] = stImageInfo.stFrameInfo.nFrameLen;
			}
			memcpy(m_pSaveImageBuf[camera->index], stImageInfo.pBufAddr, stImageInfo.stFrameInfo.nFrameLen);
			memcpy(&(m_stImageInfo[camera->index]), &(stImageInfo.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
			if (g_hwnd[camera->index])
			{
				stDisplayInfo.hWnd = g_hwnd[camera->index];
				stDisplayInfo.pData = stImageInfo.pBufAddr;
				stDisplayInfo.nDataLen = stImageInfo.stFrameInfo.nFrameLen;
				stDisplayInfo.nWidth = stImageInfo.stFrameInfo.nWidth;
				stDisplayInfo.nHeight = stImageInfo.stFrameInfo.nHeight;
				stDisplayInfo.enPixelType = stImageInfo.stFrameInfo.enPixelType;

				camera->DisplayOneFrame(&stDisplayInfo);
			}

			nRet = camera->FreeImageBuffer(&stImageInfo);
			if (nRet != MV_OK)
			{
				printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
			}
			if (camera->index == 1)
				cout << "����ɹ���" << endl;
		}
		else
		{
			Sleep(5);
		}
		if (g_bExit)
		{
			break;
		}
	}

	return 0;
}


// ch:������Ϣ����
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		hWnd = NULL;
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ch:��ʾ�����߳�
static  unsigned int __stdcall CreateRenderWindow(void* pUser)
{
	MvCamera* camera = (MvCamera*)pUser;
	string classname = "RenderWindow" + to_string(camera->index);
	HINSTANCE hInstance = ::GetModuleHandle(NULL);              //��ȡӦ�ó����ģ����
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;              //���ڵķ��
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);    //ͼ����
	wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);          //����ɫ
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);        //�����
	wc.lpfnWndProc = WndProc;                              //�Զ�����Ϣ������
	wc.lpszMenuName = NULL;
	wc.lpszClassName = classname.c_str();                       //�ô����������

																/*if (!RegisterClassEx(&wc))
																{
																return 0;
																}*/
	RegisterClassEx(&wc);

	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	RECT rect = { 0, 0, 640, 480 };

	AdjustWindowRectEx(&rect, style, false, styleEx);

	string windowname = name[camera->index] + "���";
	HWND hWnd = CreateWindowEx(styleEx, classname.c_str(), windowname.c_str(), style, 0, 0,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL, hInstance, NULL);
	if (hWnd == NULL)
	{
		return 0;
	}

	::UpdateWindow(hWnd);
	::ShowWindow(hWnd, SW_SHOW);

	g_hwnd[camera->index] = hWnd;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}


// ch:����ROI��������¼�
void MouseEventforROI(int event, int x, int y, int flags, void *param)
{
	if (bDrawROIFinished)
		return;
	switch (event)
	{
	case EVENT_MOUSEMOVE:
		if (!roi_polyline.empty()) curpoint = Point(x, y);
		break;
	case EVENT_LBUTTONDOWN:

		if (bFirstLButtenDown)
		{
			roi_polyline.push_back(Point(x / sf, y / sf));
			curpoint = Point(x/ sf, y/ sf);
			bFirstLButtenDown = false;
		}
		else
		{
			curpoint = Point(x / sf, y / sf);
		}
		line(temp_disp, roi_polyline.back(), curpoint, Scalar(0x00, 0x00, 0xff), 2, 3);
		roi_polyline.push_back(curpoint);
		circle(temp_disp, roi_polyline.back(), 1, Scalar(0, 255, 0));
		break;
	case EVENT_RBUTTONDOWN://�Ҽ���ն����
		line(temp_disp, roi_polyline.back(), roi_polyline.front(), Scalar(0x00, 0x00, 0xff), 2, 3);
		bFirstLButtenDown = true;//wyj
		bDrawROIFinished = true;
		break;
	}
}






int main() {
	// ��ʼ����
	int nRet = MV_OK;

	cout << "**********************************************************" << endl;
	cout << "*                  ˫ĿDIC����α��������               *" << endl;
	cout << "*                      ���ߣ����ĺ�                      *" << endl;
	cout << "**********************************************************" << endl;

	
	char select;
	PrintOptions();
	create_directory(output_loc);
	create_directory("./output/");
	while (cin >> select) {
		if ((select >= '1' && select <= '4')) {
			// ���س�ʼ�˵��ź�
			bool b_exit = false;
			switch (select - '0')
			{
			// ����һ���α�������ģʽ
			case 1:
			{
				// ���ó߶�����
				int scale = 1;
				// ��ȡ�ڲ��ļ�
				FileStorage fs(intrinsic_filename, FileStorage::READ);
				string reinput = intrinsic_filename;
				while (!fs.isOpened()) {
					cout << "��ǰ�ڲ��ļ�λ�ã�" << reinput.c_str();
					cout << "�޷����ڲ��ļ��������ļ�λ�ã����������루��q���س�ʼ�˵�����";
					cin >> reinput;
					if (reinput.size() == 1 && reinput[0] == 'q')
					{
						b_exit = true;
						break;
					}
					else
						fs.open(reinput, FileStorage::READ);
				}
				// �˳�����������
				if (b_exit)
					break;
				// �����ڲ��ļ���ַ���Ա�֮����
				intrinsic_filename = reinput;
				// �����ڲ�
				Mat M1, D1, M2, D2;
				fs["M1"] >> M1;
				fs["D1"] >> D1;
				fs["M2"] >> M2;
				fs["D2"] >> D2;
				M1 *= scale;
				M2 *= scale;
				cout << "����ڲμ��سɹ���" << endl;
				// ��������ļ�
				fs.open(extrinsic_filename, FileStorage::READ);
				reinput = extrinsic_filename;
				while (!fs.isOpened()) {
					cout << "��ǰ����ļ�λ�ã�" << reinput.c_str();
					cout << "�޷�������ļ��������ļ�λ�ã����������루��q���س�ʼ�˵�����";
					cin >> reinput;
					if (reinput.size() == 1 && reinput[0] == 'q')
					{
						b_exit = true;
						break;
					}
					else
						fs.open(reinput, FileStorage::READ);
				}
				if (b_exit)
					break;
				// ��������ļ���ַ���Ա�֮����
				extrinsic_filename = reinput;
				// �������
				Mat R, T, R1, P1, R2, P2;
				fs["R"] >> R;
				fs["T"] >> T;
				cout << "�����μ��سɹ���" << endl;
				fs.release();



				cout << "------��ʼ�������------" << endl;
				TiXmlDocument camera_param;
				if (!camera_param.LoadFile(camera_param_loc.c_str()))
				{
					cout << "��ǰ������������������ļ���" << endl;
					cout << "���Ƚ��������������" << endl;
					break;
				}
				// ������������ṹ�壬�������0���������1�������

				CameraParam* camera_p[2];
				MvCamera* camera[2];
				for (int i = 0; i < 2; i++) {
					camera_p[i] = new CameraParam;
					camera[i] = nullptr;
				}
				Param_WR::ReadCameraParam(camera_param, camera_p[0], camera_p[1]);
				// ����������к�ָ��
				unsigned char* SN[2];
				for (int i = 0; i < 2; i++) {
					SN[i] = camera_p[i]->SerialNumber;
				}
				// ö�ٺ��������
				MV_CC_DEVICE_INFO_LIST stDeviceList;
				MvCamera::EnumDevice(stDeviceList, false);
				bool connect_flag = false;
				for (int i = 0; i < stDeviceList.nDeviceNum; i++) {
					unsigned char* SN_C = stDeviceList.pDeviceInfo[i]->SpecialInfo.stUsb3VInfo.chSerialNumber;
					for (int j = 0; j < 2; j++) {
						if (!camera[j] && static_cast<string>((LPCSTR)SN[j]) == static_cast<string>((LPCSTR)SN_C)) {
							camera[j] = new MvCamera;
							camera[j]->index = j;
							camera[j]->Open(stDeviceList.pDeviceInfo[i]);
						}
					}
					if (camera[0] != nullptr && camera[1] != nullptr)
					{
						connect_flag = true;
						cout << "������ӳɹ�" << endl;
						break;
					}

				}
				if (!connect_flag) {
					for (int i = 0; i < 2; i++) {
						if (camera[i] == nullptr) {
							cout << "δ�ҵ�ƥ��" + name[i] + "���" << endl;
						}
						else {
							nRet = camera[i]->Close();
							delete(camera[i]);
							camera[i] = NULL;
						}
					}
					break;
				}
				// �����������ģʽ
				cout << "��ѡ���������ģʽ[0������ģʽ��1������ģʽ]��";
				unsigned int mode;
				while (cin >> mode) {
					if (mode > 1) {
						cout << "��������ȷѡ����������룺";
						continue;
					}
					break;
				}
				// �����������
				int m_nTriggerMode = MV_TRIGGER_MODE_ON;
				bool m_bConfiguration = TRUE;
				for (int i = 0; i < 2; i++) {
					// �����������
					nRet = camera[i]->SetEnumValue("TriggerMode", m_nTriggerMode);
					if (MV_OK != nRet)
					{
						cout << "����" + name[i] + "�������ģʽʧ�ܣ�" << endl;
						m_bConfiguration = FALSE;
					}
					// �������ͼ���ʽ
					nRet = camera[i]->SetEnumValueByString("PixelFormat", camera_p[i]->PixelFormat.data());
					if (MV_OK != nRet)
					{
						cout << "����" + name[i] + "���ͼ���ʽʧ�ܣ�" << endl;
						m_bConfiguration = FALSE;
					}
					// ��������ع�ʱ��
					nRet = camera[i]->SetFloatValue("ExposureTime", camera_p[i]->ExposureTime);
					if (MV_OK != nRet)
					{
						cout << "����" + name[i] + "����ع�ʱ��ʧ�ܣ�" << endl;
						m_bConfiguration = FALSE;
					}
					// �����������
					nRet = camera[i]->SetFloatValue("Gain", camera_p[i]->Gain);
					if (MV_OK != nRet)
					{
						cout << "����" + name[i] + "�������ʧ�ܣ�" << endl;
						m_bConfiguration = FALSE;
					}
				}
				if (!m_bConfiguration) {
					for (int i = 0; i < 2; i++) {
						nRet = camera[i]->Close();
						delete(camera[i]);
						camera[i] = NULL;
					}
					break;
				}

				// ����ο�ͼ��
				do
				{
					g_bExit = false;

					// ��ʼ�������ö���
					int m_nCurCameraIndex = -1;
					void* m_hGrabThread[2];
					void* m_hCreateWindow[2];

					// ch:��ʼץͼ�� ����ȡ���߳�
					for (int i = 0; i < 2; i++)
					{
						if (camera[i])
						{
							unsigned int nThreadID = i;
							m_hCreateWindow[i] = (void*)_beginthreadex(NULL, 0, CreateRenderWindow, camera[i], 0, &nThreadID);
							if (NULL == m_hCreateWindow[i])
							{
								cout << "����" + name[i] + "��������߳�ʧ�ܣ�" << endl;
								break;
							}
							nRet = camera[i]->StartGrabbing();
							if (MV_OK != nRet)
							{
								cout << name[i] + "�����ɼ�ʧ�ܣ�" << endl;
							}
							// ch:��ʼ�ɼ�֮��Ŵ���workthread�߳�
							nThreadID = i;
							m_nCurCameraIndex = i;
							m_hGrabThread[i] = (void*)_beginthreadex(NULL, 0, WorkThread, camera[i], 0, &nThreadID);
							if (NULL == m_hGrabThread[i])
							{
								cout << "����" + name[i] + "���ȡ���߳�ʧ�ܣ�" << endl;
								break;
							}
						}
						UpdateWindow(g_hwnd[i]);
						ShowWindow(g_hwnd[i], SW_SHOW);
					}

					// ���������������
					switch (mode)
					{
						// ģʽһ������ģʽ 
					case 0:
					{
						// �����������Դ
						int m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
						for (int i = 0; i < 2; i++) {
							nRet = camera[i]->SetEnumValue("TriggerSource", m_nTriggerSource);
							if (MV_OK != nRet)
							{
								cout << "����" + name[i] + "�������ԴΪ����ʧ�ܣ�" << endl;
							}
						}
						cout << endl << "*****��ʼ����ο�ͼ��*******" << endl;
						cout << "(��N�������㣬��S��ͼ��������һ��)" << endl << endl;
						bool grab_flag = true;
						m_bFinishedShoot = false;
						do
						{
							while (char c = WaitForKeyPress()) {
								if (c == 'N' || c == 'n') {
									for (int j = 0; j < 2; j++) {
										nRet = camera[j]->CommandExecute("TriggerSoftware");
										if (MV_OK != nRet)
										{
											m_bFinishedShoot = false;
											cout << name[j] + "�������ʧ��!" << endl;
											cout << "�볢���ٴΰ�N���д���" << endl;
											break;
										}
										m_bFinishedShoot = true;
									}
									Sleep(200);
									break;
								}
								else if (c == 'S' || c == 's') {
									if (SaveImage(camera, "ref.bmp")) {
										grab_flag = false;
										cout << "��ͼ�ɹ�!" << endl;
									}
									else {
										cout << "��ͼʧ�ܣ������³��Ա��������" << endl;
									}
									break;
								}

							}
						} while (grab_flag);
						break;
					}
					// ģʽ��������ģʽ
					case 1:
					{
						cout << "��ģʽ��ȷ�����������·������ɣ�����#�����#�޷����㵽ͼ��" << endl;
						// Ĭ���������Ϊ�����
						//// �������������ԴΪ����
						nRet = camera[0]->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
						if (MV_OK != nRet)
						{
							cout << "�������������ԴΪ����ʧ�ܣ�" << endl;
							break;
						}
						nRet = camera[0]->SetEnumValue("LineSelector", 1);
						nRet = camera[0]->SetBoolValue("StrobeEnable", TRUE);
						if (MV_OK != nRet)
						{
							cout << "����������������ʧ�ܣ�" << endl;
							break;
						}

						nRet = camera[1]->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE2);
						if (MV_OK != nRet)
						{
							cout << "���ô��������ԴΪline2Ӳ����ʧ�ܣ�" << endl;
							break;
						}
						cout << endl << "*****��ʼ����ͼ��*******" << endl;
						cout << "(��N�������㣬��S��ͼ��������һ��)" << endl << endl;
						bool grab_flag = true;
						m_bFinishedShoot = false;
						do
						{
							while (char c = WaitForKeyPress()) {
								if (c == 'N' || c == 'n') {
									nRet = camera[0]->CommandExecute("TriggerSoftware");
									if (MV_OK != nRet)
									{
										m_bFinishedShoot = false;
										cout << name[0] + "�������ʧ��!" << endl;
										cout << "�볢���ٴΰ�N���д���" << endl;
										break;
									}
									m_bFinishedShoot = true;
									Sleep(200);
									break;
								}

								else if (c == 'S' || c == 's') {
									if (SaveImage(camera, "ref.bmp")) {
										grab_flag = false;
										cout << "��ͼ�ɹ�!" << endl;
									}
									else {
										cout << "��ͼʧ�ܣ������³��Ա��������" << endl;
									}
									break;
								}
							}
						} while (grab_flag);
						break;
					}
					}
					cout << "�ο�ͼƬ������ϣ�" << endl;
					g_bExit = true;

					for (int i = 0; i < 2; i++) {
						nRet = camera[i]->StopGrabbing();
						if (MV_OK != nRet)
						{
							cout << name[i] + "���ֹͣȡ��ʧ�ܣ�" << endl;
						}
						// ����ȡ���߳�
						WaitForSingleObject(m_hGrabThread[i], INFINITE);
						CloseHandle(m_hGrabThread[i]);
						m_hGrabThread[i] = NULL;
						// ���ٴ��ڶ���
						CloseHandle(m_hCreateWindow[i]);
						PostMessage(g_hwnd[i], WM_CLOSE, 0, 0);
						m_hCreateWindow[i] = NULL;
					}
				} while (0);

				// �ο�ͼ����������
				bool bSubstr_finished = true;
				do
				{
					bSubstr_finished = true;
					cout << endl << endl <<  "��ѡ���������Ʒ�ʽ��" << endl;
					cout << "1.����Ӧȫ������" << endl;
					cout << "2.���л��Ƹ���Ȥ����" << endl;
					cout << "�����룺";
					int sel_subalg;
					while (cin >> sel_subalg) {
						if (sel_subalg != 1 && sel_subalg != 2) {
							cout << "����ѡ��Ϸ������������룡" << endl;
							cout << "�����룺";
							continue;
						}
						else
							break;
					}
					switch (sel_subalg)
					{

					case 1:
						break;
					case 2:
					{
						roi_polyline.clear();
						Mat left_ref_image = imread((output_loc + en_name[0] + "_ref.bmp").c_str(), 0);
						Mat right_ref_image = imread((output_loc + en_name[0] + "_ref.bmp").c_str(), 0);
						Mat left_ref_image_disp, right_ref_image_disp;
						cvtColor(left_ref_image, left_ref_image_disp, COLOR_GRAY2RGB);
						cvtColor(right_ref_image, right_ref_image_disp, COLOR_GRAY2RGB);
						temp_disp = left_ref_image_disp.clone();
						cout << endl << "*****��ʼ���Ƹ���Ȥ����*******" << endl;
						cout << "��ѡ�����Ȥ�����ͼ����ʽ��" << endl;
						cout << "1.����" << endl;
						cout << "2.�����" << endl;
						cout << "�����룺";
						int sel_subtype;
						while (cin >> sel_subtype) {
							if (sel_subtype != 1 && sel_subtype != 2) {
								cout << "����ѡ��Ϸ������������룡" << endl;
								cout << "�����룺";
								continue;
							}
							else
								break;
						}
						switch (sel_subtype)
						{
						case 1:			// ����ģʽ
						{




							break;
						}
						case 2:			// �����ģʽ
						{
							cout << "(��r���»��ƣ���s�洢��ǰ����νǵ���Ϣ����n������һ��)" << endl << endl;
							namedWindow("���Ƹ���Ȥ����");
							sf = 1020. / MAX(temp_disp.rows, temp_disp.cols);
							setMouseCallback("���Ƹ���Ȥ����", MouseEventforROI, (void *)&temp_disp);
							bDrawROIFinished = false;
							Mat roi_disp;
							while (1) {
								resize(temp_disp, roi_disp, Size(), sf, sf, INTER_LINEAR_EXACT);
								if (!roi_polyline.empty() && !bDrawROIFinished)
								{
									Point temp_point = Point(roi_polyline.back().x * sf, roi_polyline.back().y * sf);
									line(roi_disp, temp_point, curpoint, Scalar(0x00, 0x00, 0xff), 2, 3);
								}

								imshow("���Ƹ���Ȥ����", roi_disp);
								char sel_roi = waitKey(30);
								if (sel_roi == 'r' || sel_roi == 'R') {
									bDrawROIFinished = false;
									roi_polyline.clear();
									temp_disp = left_ref_image_disp.clone();
								}
								if (bDrawROIFinished) {
									if (sel_roi == 's' || sel_roi == 'S') {
										//Param_WR::WritePolyline("./output/polyline.xml", roi_polyline);
										cout << "����νǵ�洢���" << endl;
									}
									else if (sel_roi == 'n' || sel_roi == 'N') {
										cout << "����Ȥ������Ƴɹ���������һ��!" << endl;
										destroyAllWindows();
										break;
									}
								}
							}

							break;
						}
						}					
					}
					}



				} while (!bSubstr_finished);


				






				for (int i = 0; i < 2; i++) {
					// �ͷ��������
					if (camera[i] != nullptr) {
						nRet = camera[i]->Close();
						delete(camera[i]);
						camera[i] = nullptr;
					}
					// �ͷŻ���
					if (m_pSaveImageBuf[i])
					{
						free(m_pSaveImageBuf[i]);
						m_pSaveImageBuf[i] = NULL;
					}
					m_nSaveImageBufSize[i] = 0;
				}
				break;
			}
			// ���ܶ����α�������ģʽ
			case 2:
			{
				break;
			}
			// ���������޸��������
			case 3:
			{
				TiXmlDocument camera_param;
				bool camera_param_flag = FALSE;
				CameraParam* left_camera = new CameraParam;
				CameraParam* right_camera = new CameraParam;
				if (camera_param.LoadFile(camera_param_loc.data()))
				{
					camera_param_flag = TRUE;
					Param_WR::ReadCameraParam(camera_param, left_camera, right_camera);
					cout << endl;
					cout << "**********************************************************" << endl;
					cout << "                  ��ǰ���������Ϣ" << endl;
					cout << "�����������" << endl;
					Param_WR::PrintCameraParam(left_camera);
					cout << "�����������" << endl;
					Param_WR::PrintCameraParam(right_camera);
					cout << "**********************************************************" << endl;
				}
				// ch:ö���豸 | en:Enum device
				MV_CC_DEVICE_INFO_LIST stDeviceList;
				if (!MvCamera::EnumDevice(stDeviceList)) {
					if (!camera_param_flag) {
						cout << "ֻ�������������ܴ���������������ļ���" << endl;
						break;
					}
					cout << "��ǰ����������������Ƿ�Ҫ�޸�������������ļ���0����, 1���ǣ���";
					bool rec_flag = false;
					cin >> rec_flag;
					if (!rec_flag)
						break;
					// �������������
					cout << "���޸��������������!" << endl;
					cout << "������кţ���ǰֵ��" << left_camera->SerialNumber << ")��";
					cin >> left_camera->SerialNumber;
					cout << "���ͼ���ʽ����ǰֵ��" << left_camera->PixelFormat << ")[�������Mono8��Mono10��Mono10Packed��Mono12��Mono12Packed]��";
					string PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							left_camera->PixelFormat = PF;
							break;
						}
						cout << "�����ʽ��ƥ�䣬���������룺";
					}
					cout << "����ع�ʱ�䣨��ǰֵ��" << left_camera->ExposureTime << ")��";
					cin >> left_camera->ExposureTime;
					cout << "������棨��ǰֵ��" << left_camera->Gain << ")��";
					cin >> left_camera->Gain;
					// �������������
					cout << "���޸��������������!" << endl;
					cout << "������кţ���ǰֵ��" << right_camera->SerialNumber << ")��";
					while (1) {
						cin >> right_camera->SerialNumber;
						unsigned char* SN_L = left_camera->SerialNumber;
						unsigned char* SN_R = right_camera->SerialNumber;
						if (static_cast<string>((LPCSTR)SN_L) != static_cast<string>((LPCSTR)SN_R)) {
							SN_L = nullptr;
							SN_R = nullptr;
							break;
						}
						else {
							cout << "���кų�ͻ�������������Ϊͬһ�������������룡" << endl;
						}
					}
					cout << "���ͼ���ʽ����ǰֵ��" << right_camera->PixelFormat << ")[�������Mono8��Mono10��Mono10Packed��Mono12��Mono12Packed]��";
					PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							if (PF == left_camera->PixelFormat) {
								right_camera->PixelFormat = PF;
								break;
							}
							else
								cout << "ͼ���ʽ�豣֤�������һ�£����������룺";

						}
						else {
							cout << "�����ʽ��ƥ�䣬���������룺";
						}
					}
					cout << "����ع�ʱ�䣨��ǰֵ��" << right_camera->ExposureTime << ")��";
					cin >> right_camera->ExposureTime;
					cout << "������棨��ǰֵ��" << right_camera->Gain << ")��";
					cin >> right_camera->Gain;
					if (Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "�޸���������ļ��ɹ���" << endl << endl;
					else
						cout << endl << endl << "�޸���������ļ�ʧ�ܣ�" << endl << endl;
				}
				else {
					// �������������
					cout << "*******��ѡ�������******" << endl;
					printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
					unsigned int nIndex = 0;
					while (cin >> nIndex)
					{
						if (nIndex >= stDeviceList.nDeviceNum)
						{
							cout << "Input error!" << endl;
							cout << "���������룺";
							continue;
						}
						break;
					}
					GetCameraParam(left_camera, stDeviceList.pDeviceInfo[nIndex]);
					cout << "���޸��������������!" << endl;
					cout << "���ͼ���ʽ����ǰֵ��" << left_camera->PixelFormat << ")[�������Mono8��Mono10��Mono10Packed��Mono12��Mono12Packed]��";
					string PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							left_camera->PixelFormat = PF;
							break;
						}
						cout << "�����ʽ��ƥ�䣬���������룺";
					}
					cout << "����ع�ʱ�䣨��ǰֵ��" << left_camera->ExposureTime << ")��";
					cin >> left_camera->ExposureTime;
					cout << "������棨��ǰֵ��" << left_camera->Gain << ")��";
					cin >> left_camera->Gain;

					// �������������
					nIndex = 0;
					while (1) {
						cout << "*******��ѡ�������******" << endl;
						printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
						while (cin >> nIndex)
						{
							if (nIndex >= stDeviceList.nDeviceNum)
							{
								cout << "Input error!" << endl;
								cout << "���������룺";
								continue;
							}
							break;
						}
						unsigned char* SN_L = left_camera->SerialNumber;
						unsigned char* SN_R = stDeviceList.pDeviceInfo[nIndex]->SpecialInfo.stUsb3VInfo.chSerialNumber;
						if (static_cast<string>((LPCSTR)SN_L) != static_cast<string>((LPCSTR)SN_R)) {
							SN_L = nullptr;
							SN_R = nullptr;
							break;
						}
						else {
							cout << "���кų�ͻ�������������Ϊͬһ����������ѡ��" << endl;
						}
					}
					GetCameraParam(right_camera, stDeviceList.pDeviceInfo[nIndex]);
					cout << "���޸��������������!" << endl;
					cout << "���ͼ���ʽ����ǰֵ��" << right_camera->PixelFormat << ")[�������Mono8��Mono10��Mono10Packed��Mono12��Mono12Packed]��";
					PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							if (PF == left_camera->PixelFormat) {
								right_camera->PixelFormat = PF;
								break;
							}
							else
								cout << "ͼ���ʽ�豣֤�������һ�£����������룺";

						}
						else {
							cout << "�����ʽ��ƥ�䣬���������룺";
						}
					}
					cout << "����ع�ʱ�䣨��ǰֵ��" << right_camera->ExposureTime << ")��";
					cin >> right_camera->ExposureTime;
					cout << "������棨��ǰֵ��" << right_camera->Gain << ")��";
					cin >> right_camera->Gain;
					if (Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "������������ļ��ɹ���" << endl << endl;
					else
						cout << endl << endl << "������������ļ�ʧ�ܣ�" << endl << endl;
				}
				break;
			}

			// �����ģ��޸��α�����㷨����
			case 4:
			{
				break;
			}
			}
		}
		else if (select == 'q' || select == 'Q') {
			break;
		}
		else {
			cout << "��������ȷѡ���" << endl << endl;
		}
		PrintOptions();
	}








	return 0;
}
































//int main()
//{
//	// ͼ����
//	int width = 1400;
//	int height = 1000;
//	
//	// �߿��С
//	int thicknum = 2;
//	// ����ά�ȵ�����Բ����Ŀ
//	int sqXnum = 14;
//	int sqYnum = 10;
//
//	// �����������������Ŀ������Ӧ����Բ��뾶��С���˴�Ԥ����Բ��Բ��֮�����Ϊ4*�뾶��
//	int radius = min(width / (4 * sqXnum + 2), height / (4 * sqYnum + 2));
//	if (radius <= 0.01 * min(height, width)) {
//		cout << "���棺Բ���С�������޷�ʶ��" << endl;
//	}
//	int space = 4 * radius;
//	// ��������ά�ȷ����ϵı�Ե�հ�
//	int x_st = (width - 2 * radius * (2 * sqXnum - 1)) / 2;
//	int y_st = (height - 2 * radius * (2 * sqYnum - 1)) / 2;
//
//	// ���ɿհ׻���
//	Mat img(height + 2 * thicknum, width + 2 * thicknum, CV_8UC4, Scalar(255, 255, 255, 255));
//	// ������ʼ��Բ������
//	int cir_x = x_st + radius + thicknum;
//	int cir_y = y_st + radius + thicknum;
//	// ���Ʋ���ǵĶԳ�Բ��
//	for (int i = 0; i < img.rows; i++) {
//		for (int j = 0; j < img.cols; j++) {
//			// ���Ʊ߿�
//			if (i < thicknum || i >= thicknum + height || j < thicknum || j >= thicknum + width) {
//				img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
//				continue;
//			}
//			// ����Բ��
//			if (cir_y >= img.rows - y_st - thicknum) {
//				continue;
//			}
//			if (i == cir_y && j == cir_x) {
//				// ����Բ�㣬LINE_AA�õ��ı�Ե��Ϊ�⻬
//				circle(img, Point(j, i), radius, cv::Scalar(0, 0, 0, 255), -1, LINE_AA);
//				cir_x += space;
//			}
//			if (cir_x >= img.cols - x_st - thicknum) {
//				cir_x = x_st + radius + thicknum;
//				cir_y += space;
//			}
//
//		}
//	}
//
//	int origin_offset_x = 0;
//	int origin_offset_y = 0;
//	int inner_pattern_height = 6;
//	int inner_pattern_width = 10;
//	// ���Ʊ�ǵ�
//	cir_x = x_st + radius + thicknum + origin_offset_x * space;
//	cir_y = y_st + radius + thicknum + (sqYnum - origin_offset_y - inner_pattern_height) * space;
//	circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);
//	cir_y += (inner_pattern_height - 1) * space;
//	circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);
//	cir_x += (inner_pattern_width - 1) * space;
//	circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);
//
//
//	imwrite("dot_calib.png", img);
//	imshow("Բ��궨ͼ��", img);
//	waitKey(0);
//	return 0 ;
//}