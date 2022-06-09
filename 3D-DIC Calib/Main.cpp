// ���ƣ�����궨����
// ���ߣ����ĺ�
// ����ʱ�䣺2021.6.27

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


#include "CalibAlgorithm.h"
#include "MvCamera.h"
#include "Param_WR.h"


using namespace std;


// �����ʼ������
string camera_param_loc = "./camera_param.xml";	//�������·��
string calib_param_loc = "./calib_param.xml";	//�궨����·��
string output_loc = "./data/";					//����ͼ�����·��
vector<string> ImageName;

unsigned char* m_pSaveImageBuf[2];
unsigned int m_nSaveImageBufSize[2];
MV_FRAME_OUT_INFO_EX  m_stImageInfo[2];


bool g_bExit = false;
bool m_bFinishedShoot = false;
int imagenums = 1;
// ��ʾ����
HWND g_hwnd[2] = {NULL, NULL};
Mat srcImage[2];
// �������
string name[2] = { "��","��" };
string en_name[2] = { "left","right" };



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

// ch:�ȴ��������� | en:Wait for key press
char WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	return (char)_getch();
}

// ch:��ӡ��������Ϣ
void PrintOptions()
{
	cout << endl;
	cout << "*******��ѡ���Ӧ����*******" << endl;
	cout << "1.��������궨ͼ��" << endl;
	cout << "2.����˫Ŀϵͳ�궨" << endl;
	cout << "3.�����������" << endl;
	cout << "4.���ñ궨����" << endl;
	cout << "5.���ɱ궨ͼ��" << endl;
	cout << "q.�˳��궨����" << endl << endl;
	cout << "ѡ���ܣ�" ;
}

// ch:��ӡ�궨ͼ����Ϣ
void PrintPatternOptions()
{
	cout << endl;
	cout << "*******��ѡ������궨ͼ��*******" << endl;
	cout << "1.���̸�ͼ��" << endl;
	cout << "2.�Գ�Բ��ͼ�����ޱ�ǵ㣩" << endl;
	cout << "3.�Գ�Բ��ͼ�����б�ǵ㣩" << endl;
	cout << "q.�˳��궨ͼ������" << endl << endl;
	cout << "ѡ���ܣ�";
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
			if(camera->index == 1)
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



int main()
{
	// ��ʼ����
	int nRet = MV_OK;
	

	cout << "**********************************************************" << endl;
	cout << "*                  ˫ĿDIC����궨����                   *" << endl;
	cout << "*                      ���ߣ����ĺ�                      *" << endl;
	cout << "**********************************************************" << endl;
	char select;
	PrintOptions();
	while (cin >> select) {
		if ((select >= '1' && select <= '5')) {
			switch (select - '0')
			{
			// ����һ���������ͼ��ģʽ
			case 1:
			{
				create_directory("./data");
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
							cout << "δ�ҵ�ƥ��"  + name[i] + "���" << endl;
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
				do
				{
					g_bExit = false;
					imagenums = 1;
					// ���������ͼ����Ŀ
					cout << "����������궨���ͼ�����Ŀ[�����3������ɱ궨���㣡]��";
					int image_nums;
					while (cin >> image_nums) {
						if (image_nums <= 3) {
							cout << "���������3�����֣����������룺";
							continue;
						}
						else if (image_nums > 20) {
							cout << "����궨ͼ����Ŀ���˹������������룺";
							continue;
						}
						break;
					}

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
						cout << endl << "*****��ʼ����ͼ��*******" << endl;
						cout << "(��N�������㣬��S��ͼ��������һ��)" << endl << endl;
						for (int i = 0; i < image_nums; i++) {
							cout << "��" << i + 1 << '/' << image_nums << "�飺";
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
										if (SaveImage(camera)) {
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
						}
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
						for (int i = 0; i < image_nums; i++) {
							cout << "��" << i + 1 << '/' << image_nums << "�飺";
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
										if (SaveImage(camera)) {
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
						}

						break;
					}
					}
					cout << "ȫ��ͼƬ������ϣ�" << endl;
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
					// ����ͼƬ�б�
					if (!ImageName.empty() || ImageName.size() < image_nums * 2) {
						string temp = output_loc + "stereo_calib.xml";
						FileStorage fs(temp, FileStorage::WRITE); 
						fs << "imagelist" << ImageName;
						cout << "�ļ��б����ɳɹ���" << endl;
						fs.release();
					}
					else
						cout << "�ļ��б�����ʧ�ܣ�ͼƬȱʧ" << endl;
					ImageName.clear();
				} while (0);
				

				
				for (int i = 0; i < 2; i++) {
					// �ͷ��������
					if(camera[i] != nullptr) {
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

			// ���ܶ�������궨
			case 2:
			{
				TiXmlDocument calib_param;
				if (!calib_param.LoadFile(calib_param_loc.c_str()))
				{
					cout << "��ǰ�����ڱ궨���������ļ���" << endl;
					cout << "���Ƚ��б궨��������" << endl;
					break;
				}
				CalibPattern* p_calib_param = new CalibPattern;
				if (Param_WR::ReadCalibParam(calib_param, p_calib_param) == 0)
				{
					cout << "�޷���ȡ�궨�����ļ��������ļ��Ƿ��𻵻�ռ�ã�" << endl;
					break;
				}
					
				calib_algorithm *camera_calib = nullptr;
				switch (p_calib_param->PatternType)
				{
				case CALIB_PATTERN_CHECKERBOARD:
					camera_calib = new calib_checkerboard(*p_calib_param);
					break;
				case CALIB_PATTERN_CIRCULAR:
					camera_calib = new calib_circular(*p_calib_param);
					break;
				case CALIB_PATTERN_SPECIAL:
					camera_calib = new calib_circular_new(*p_calib_param);
					break;
				}
				// ������������ļ���
				camera_calib->image_folder = p_calib_param->image_folder ;
				camera_calib->cal_debug_folder = p_calib_param->cal_debug_folder;
				if (!camera_calib->readStringList()) {
					cout << "�޷��� " << camera_calib->image_folder + "/stereo_calib.xml" << " ���ļ������ڣ�" << endl;
					break;
				}
				camera_calib->ExtractTarget();
				camera_calib->StereoCalib();
				// ��������
				if (camera_calib) {
					delete camera_calib;
					camera_calib = nullptr;
				}
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
					if(Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "������������ļ��ɹ���" << endl << endl;
					else
						cout << endl << endl << "������������ļ�ʧ�ܣ�" << endl << endl;
				}		
				break;
			}
				
			// �����ģ��޸ı궨����
			case 4 :
			{
				TiXmlDocument calib_param;
				bool calib_param_flag = FALSE;
				CalibPattern* p_calib_param = new CalibPattern;
				if (calib_param.LoadFile(calib_param_loc.data()))
				{
					calib_param_flag = TRUE;
					Param_WR::ReadCalibParam(calib_param, p_calib_param);
					Param_WR::PrintCalibParam(p_calib_param);
				}
				cout << "������궨ͼ�������ļ��У�";
				cin >> p_calib_param->image_folder;
				cout << "���������Ŀ¼�����ڴ洢�궨�������ȡ�������ͼ�񣩣�";
				cin >> p_calib_param->cal_debug_folder;
				cout << "�����������������Ŀ��";
				cin >> p_calib_param->sqXnum;
				cout << "��������������������";
				cin >> p_calib_param->sqYnum;
				cout << "�������������ࣨʵ������߶ȣ���";
				cin >> p_calib_param->TargetSize;
				cout << "��ѡ��궨ͼ�����ͣ�" << endl;
				cout << "1.���̸�ͼ��" << endl;
				cout << "2.�Գ�Բ��ͼ�����ޱ�ǵ㣩" << endl;
				cout << "3.�Գ�Բ��ͼ�����б�ǵ㣩" << endl;
				cout << "ѡ��ͼ����";
				unsigned int select;
				while (cin >> select) {
					if (select >= 1 && select <= 3) {
						switch (select)
						{
						case 1:
						{
							p_calib_param->PatternType = CALIB_PATTERN_CHECKERBOARD;
							break;
						}
						case 2:
						{
							p_calib_param->PatternType = CALIB_PATTERN_CIRCULAR;
							break;
						}
						case 3:
						{
							p_calib_param->PatternType = CALIB_PATTERN_SPECIAL;
							cout << "��������ʼ��־��X���꣺";
							cin >> p_calib_param->origin_offset_x;
							cout << "��������ʼ��־��Y���꣺";
							cin >> p_calib_param->origin_offset_y;
							cout << "��������ʼ�㵽�ҷ���־����룺";
							cin >> p_calib_param->inner_pattern_width;
							cout << "��������ʼ�㵽�Ϸ���־����룺";
							cin >> p_calib_param->inner_pattern_height;
							
							break;
						}
						}
						break;
					}
					else {
						cout << "����������Ϸ�ѡ�" << endl;
					}
				}
				if (Param_WR::SetCalibParam(calib_param_loc, p_calib_param))
					cout << endl << endl << "�����궨�����ļ��ɹ���" << endl << endl;
				else
					cout << endl << endl << "�����궨�����ļ�ʧ�ܣ�" << endl << endl;

				break;
			}

			// �����壺���ɱ궨ͼ��
			case 5:
			{
				PrintPatternOptions();
				char pattern_sel;
				while (cin >> pattern_sel) {
					if ((pattern_sel >= '1' && pattern_sel <= '3')) {
						CalibPattern C_P;
						cout << "�������������ɵ�ͼ���ȣ���λ�����أ���";
						cin >> C_P.bkgWidth;
						cout << "�������������ɵ�ͼ��߶ȣ���λ�����أ���";
						cin >> C_P.bkgHeight;
						cout << "�������������ɵ�ͼ����������������";
						cin >> C_P.sqXnum;
						cout << "�������������ɵ�ͼ����������������Ĭ��Ϊ0ʱ�����������������һ�£���";
						cin >> C_P.sqYnum;
						cout << "�������������ɵ�ͼ���߿��С�����أ���";
						cin >> C_P.thickNum;
						cout << "�����뱣���ļ�·������ʽPNG/JPEG/BMP����";
						string save_loc;
						cin >> C_P.savePath;
						switch (pattern_sel - '0')
						{
						case 1:
						{
							C_P.PatternType = CALIB_PATTERN_CHECKERBOARD;
							if (C_P.check())
								calib_algorithm::MakeCailbPattern(C_P);
							break;
						}
						case 2:
						{
							C_P.PatternType = CALIB_PATTERN_CIRCULAR;
							if (C_P.check())
								calib_algorithm::MakeCailbPattern(C_P);
							break;
						}
						case 3:
						{
							C_P.PatternType = CALIB_PATTERN_SPECIAL;
							cout << "��������ʼ��־��X����(��С��0�����" << C_P.sqXnum <<  ")��";
							cin >> C_P.origin_offset_x;
							cout << "��������ʼ��־��Y����(��С��0�����" << C_P.sqYnum << ")��";
							cin >> C_P.origin_offset_y;
							cout << "��������ʼ�㵽�Ϸ���־����룺";
							cin >> C_P.inner_pattern_height;
							cout << "��������ʼ�㵽�ҷ���־����룺";
							cin >> C_P.inner_pattern_width;
							if (C_P.check())
								calib_algorithm::MakeCailbPattern(C_P);
							break;
						}
						}
						break;
					}
					else if (select == 'q' || select == 'Q') {
						break;
					}
					else {
						cout << "��������ȷѡ���" << endl << endl;
					}
					PrintPatternOptions();
				}
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



// ����У�����Դ���
//Mat testimg = imread("./data/CalB-sys2-0001_0.jpeg", 0);
//Mat testimg2 = imread("./data/CalB-sys2-0001_1.jpeg", 0);
//Mat M1, D1, M2, D2;
//readIntrinsic("./output/intrinsics.xml", M1, D1, M2, D2);
//Mat R, T, R1, P1, R2, P2, Q;
//readExtrinsic("./output/extrinsics.xml", R, T, R1, P1, R2, P2, Q);
//double sf = 1020. / MAX(testimg.rows, testimg.cols);
//
//Mat map11, map12, map21, map22;
//initUndistortRectifyMap(M1, D1, R1, P1, testimg.size(), CV_16SC2, map11, map12);
//initUndistortRectifyMap(M2, D2, R2, P2, testimg2.size(), CV_16SC2, map21, map22);
//
//Mat img1r, img2r;
//remap(testimg, img1r, map11, map12, INTER_CUBIC);
//remap(testimg2, img2r, map21, map22, INTER_CUBIC);
//
//Mat img_c;
//resize(testimg, img_c, Size(), sf, sf, INTER_LINEAR_EXACT);
//imshow("origin", img_c);
//
//Mat img_d;
//resize(img1r, img_d, Size(), sf, sf, INTER_LINEAR_EXACT);
//imshow("undist", img_d);
//
//waitKey();
//destroyAllWindows();



