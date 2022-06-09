// 名称：相机标定程序
// 作者：吴文浩
// 创建时间：2021.6.27

//系统依赖项
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


// 程序初始化参数
string camera_param_loc = "./camera_param.xml";	//相机参数路径
string calib_param_loc = "./calib_param.xml";	//标定参数路径
string output_loc = "./data/";					//拍摄图像输出路径
vector<string> ImageName;

unsigned char* m_pSaveImageBuf[2];
unsigned int m_nSaveImageBufSize[2];
MV_FRAME_OUT_INFO_EX  m_stImageInfo[2];


bool g_bExit = false;
bool m_bFinishedShoot = false;
int imagenums = 1;
// 显示窗口
HWND g_hwnd[2] = {NULL, NULL};
Mat srcImage[2];
// 相机名称
string name[2] = { "左","右" };
string en_name[2] = { "left","right" };



// ch:获取相机参数信息
bool GetCameraParam(CameraParam* camerap, MV_CC_DEVICE_INFO* pstDevInfo)
{
	
	int nRet = MV_OK;
	MvCamera* camera = new MvCamera;
	camera->Open(pstDevInfo);
	// ch:获取相机序列号
	MVCC_STRINGVALUE* SN = new MVCC_STRINGVALUE;
	nRet = camera->GetStringValue("DeviceSerialNumber", SN);
	memcpy(camerap->SerialNumber, SN->chCurValue, sizeof(camerap->SerialNumber));

	// ch:获取相机图像格式
	MVCC_ENUMVALUE* PF = new MVCC_ENUMVALUE;
	nRet = camera->GetEnumValue("PixelFormat", PF);
	camerap->PixelFormat = MV_CC_PixelFormat[PF->nCurValue];

	// ch:获取相机曝光时间
	MVCC_FLOATVALUE* exp_time = new MVCC_FLOATVALUE;
	nRet = camera->GetFloatValue("ExposureTime", exp_time);
	camerap->ExposureTime = exp_time->fCurValue;

	// ch:获取相机增益
	MVCC_FLOATVALUE* gain = new MVCC_FLOATVALUE;
	nRet = camera->GetFloatValue("Gain", gain);
	camerap->Gain = gain->fCurValue;

	// ch:关闭设备 | Close device
	nRet = camera->Close();
	if (MV_OK != nRet)
	{
		printf("ClosDevice fail! nRet [0x%x]\n", nRet);
		return false;
	}
	return true;
}

// ch:等待按键输入 | en:Wait for key press
char WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	return (char)_getch();
}

// ch:打印程序功能信息
void PrintOptions()
{
	cout << endl;
	cout << "*******请选择对应功能*******" << endl;
	cout << "1.拍摄相机标定图像" << endl;
	cout << "2.进行双目系统标定" << endl;
	cout << "3.配置相机参数" << endl;
	cout << "4.配置标定参数" << endl;
	cout << "5.生成标定图案" << endl;
	cout << "q.退出标定程序" << endl << endl;
	cout << "选择功能：" ;
}

// ch:打印标定图案信息
void PrintPatternOptions()
{
	cout << endl;
	cout << "*******请选择所需标定图案*******" << endl;
	cout << "1.棋盘格图案" << endl;
	cout << "2.对称圆点图案（无标记点）" << endl;
	cout << "3.对称圆点图案（有标记点）" << endl;
	cout << "q.退出标定图案生成" << endl << endl;
	cout << "选择功能：";
}

// ch:工作线程
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
			// 用于存图
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
				cout << "拍摄成功！" << endl;
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

// ch:窗口消息函数
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

// ch:显示窗口线程
static  unsigned int __stdcall CreateRenderWindow(void* pUser)
{
	MvCamera* camera = (MvCamera*)pUser;
	string classname = "RenderWindow" + to_string(camera->index);
	HINSTANCE hInstance = ::GetModuleHandle(NULL);              //获取应用程序的模块句柄
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;              //窗口的风格
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);    //图标风格
	wc.hIconSm = ::LoadIcon(NULL, IDI_APPLICATION);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);          //背景色
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);        //鼠标风格
	wc.lpfnWndProc = WndProc;                              //自定义消息处理函数
	wc.lpszMenuName = NULL;
	wc.lpszClassName = classname.c_str();                       //该窗口类的名称

	/*if (!RegisterClassEx(&wc))
	{
		return 0;
	}*/
	RegisterClassEx(&wc);

	DWORD style = WS_OVERLAPPEDWINDOW;
	DWORD styleEx = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	RECT rect = { 0, 0, 640, 480 };

	AdjustWindowRectEx(&rect, style, false, styleEx);

	string windowname = name[camera->index] + "相机";
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

// ch:保存图像
bool SaveImage(MvCamera *camera[])
{
	if (!m_bFinishedShoot) {
		cout << "请先进行拍摄" << endl;
		return FALSE;
	}
	MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
	memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));
	for (int i = 0; i < 2; i++) {
		if (m_pSaveImageBuf[i] == NULL || m_stImageInfo[i].enPixelType == 0)
		{
			cout << name[i] + "相机无法保存" << endl;
			return FALSE;
		}
		stSaveFileParam.enImageType = MV_Image_Bmp; // ch:需要保存的图像类型 | en:Image format to save
		stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
		stSaveFileParam.nWidth = m_stImageInfo[i].nWidth;         // ch:相机对应的宽 | en:Width
		stSaveFileParam.nHeight = m_stImageInfo[i].nHeight;          // ch:相机对应的高 | en:Height
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
			cout << "保存图片失败！" << endl;
			return FALSE;
		}
		
	}
	imagenums++;
	return TRUE;
}



int main()
{
	// 初始参数
	int nRet = MV_OK;
	

	cout << "**********************************************************" << endl;
	cout << "*                  双目DIC相机标定程序                   *" << endl;
	cout << "*                      作者：吴文浩                      *" << endl;
	cout << "**********************************************************" << endl;
	char select;
	PrintOptions();
	while (cin >> select) {
		if ((select >= '1' && select <= '5')) {
			switch (select - '0')
			{
			// 功能一：相机拍摄图像模式
			case 1:
			{
				create_directory("./data");
				TiXmlDocument camera_param;
				if (!camera_param.LoadFile(camera_param_loc.c_str()))
				{
					cout << "当前不存在相机参数配置文件！" << endl;
					cout << "请先进行相机参数配置" << endl;
					break;
				}
				// 左右相机参数结构体，相机对象，0：左相机；1：右相机
				
				CameraParam* camera_p[2];
				MvCamera* camera[2];
				for (int i = 0; i < 2; i++) {
					camera_p[i] = new CameraParam;
					camera[i] = nullptr;
				}
				Param_WR::ReadCameraParam(camera_param, camera_p[0], camera_p[1]);

				// 左右相机序列号指针
				unsigned char* SN[2];
				for (int i = 0; i < 2; i++) {
					SN[i] = camera_p[i]->SerialNumber;
				}
				// 枚举和连接相机
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
						cout << "相机连接成功" << endl;
						break;
					}
						
				}
				if (!connect_flag) {
					for (int i = 0; i < 2; i++) {
						if (camera[i] == nullptr) {
							cout << "未找到匹配"  + name[i] + "相机" << endl;
						}
						else {
							nRet = camera[i]->Close();
							delete(camera[i]);
							camera[i] = NULL;
						}
					}
					break;
				}

				// 设置相机触发模式
				cout << "请选择相机触发模式[0：软触发模式，1：主从模式]：";
				unsigned int mode;
				while (cin >> mode) {
					if (mode > 1) {
						cout << "请输入正确选项！请重新输入：";
						continue;
					}
					break;
				}
				// 配置相机参数
				int m_nTriggerMode = MV_TRIGGER_MODE_ON;
				bool m_bConfiguration = TRUE;
				for (int i = 0; i < 2; i++) {
					// 开启相机触发
					nRet = camera[i]->SetEnumValue("TriggerMode", m_nTriggerMode);
					if (MV_OK != nRet)
					{
						cout << "开启" + name[i] + "相机触发模式失败！" << endl;
						m_bConfiguration = FALSE;
					}
					// 配置相机图像格式
					nRet = camera[i]->SetEnumValueByString("PixelFormat", camera_p[i]->PixelFormat.data());
					if (MV_OK != nRet)
					{
						cout << "配置" + name[i] + "相机图像格式失败！" << endl;
						m_bConfiguration = FALSE;
					}
					// 配置相机曝光时间
					nRet = camera[i]->SetFloatValue("ExposureTime", camera_p[i]->ExposureTime);
					if (MV_OK != nRet)
					{
						cout << "配置" + name[i] + "相机曝光时间失败！" << endl;
						m_bConfiguration = FALSE;
					}
					// 配置相机增益
					nRet = camera[i]->SetFloatValue("Gain", camera_p[i]->Gain);
					if (MV_OK != nRet)
					{
						cout << "配置" + name[i] + "相机增益失败！" << endl;
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
					// 设置拍摄的图像数目
					cout << "请输入拍摄标定板的图像对数目[需大于3才能完成标定计算！]：";
					int image_nums;
					while (cin >> image_nums) {
						if (image_nums <= 3) {
							cout << "请输入大于3的数字！请重新输入：";
							continue;
						}
						else if (image_nums > 20) {
							cout << "拍摄标定图像数目不宜过大！请重新输入：";
							continue;
						}
						break;
					}

					// 初始化拍摄用对象
					int m_nCurCameraIndex = -1;
					void* m_hGrabThread[2];
					void* m_hCreateWindow[2];

					// ch:开始抓图， 创建取流线程
					for (int i = 0; i < 2; i++)
					{
						if (camera[i])
						{
							unsigned int nThreadID = i;
							m_hCreateWindow[i] = (void*)_beginthreadex(NULL, 0, CreateRenderWindow, camera[i], 0, &nThreadID);
							if (NULL == m_hCreateWindow[i])
							{
								cout << "创建" + name[i] + "相机窗口线程失败！" << endl;
								break;
							}
							nRet = camera[i]->StartGrabbing();
							if (MV_OK != nRet)
							{
								cout << name[i] + "开启采集失败！" << endl;
							}
							// ch:开始采集之后才创建workthread线程
							nThreadID = i;
							m_nCurCameraIndex = i;
							m_hGrabThread[i] = (void*)_beginthreadex(NULL, 0, WorkThread, camera[i], 0, &nThreadID);
							if (NULL == m_hGrabThread[i])
							{
								cout << "创建" + name[i] + "相机取流线程失败！" << endl;
								break;
							}
						}
						UpdateWindow(g_hwnd[i]);
						ShowWindow(g_hwnd[i], SW_SHOW);
					}			

					// 配置相机触发参数
					switch (mode)
					{
						// 模式一：软触发模式 
					case 0:
					{
						// 设置相机触发源
						int m_nTriggerSource = MV_TRIGGER_SOURCE_SOFTWARE;
						for (int i = 0; i < 2; i++) {
							nRet = camera[i]->SetEnumValue("TriggerSource", m_nTriggerSource);
							if (MV_OK != nRet)
							{
								cout << "配置" + name[i] + "相机触发源为软触发失败！" << endl;
							}
						}
						cout << endl << "*****开始拍摄图像*******" << endl;
						cout << "(按N触发拍摄，按S存图并进入下一组)" << endl << endl;
						for (int i = 0; i < image_nums; i++) {
							cout << "第" << i + 1 << '/' << image_nums << "组：";
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
												cout << name[j] + "相机软触发失败!" << endl;
												cout << "请尝试再次按N进行触发" << endl;
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
											cout << "存图成功!" << endl;
										}
										else {
											cout << "存图失败！请重新尝试保存或拍摄" << endl;
										}
										break;
									}

								}								
							} while (grab_flag);
						}
						break;
					}
					// 模式二：主从模式
					case 1:
					{
						cout << "该模式请确保相机触发线路连接完成，否则#从相机#无法拍摄到图像！" << endl;
						// 默认左相机作为主相机
						//// 设置主相机触发源为软触发
						nRet = camera[0]->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
						if (MV_OK != nRet)
						{
							cout << "配置主相机触发源为软触发失败！" << endl;
							break;
						}
						nRet = camera[0]->SetEnumValue("LineSelector", 1);
						nRet = camera[0]->SetBoolValue("StrobeEnable", TRUE);
						if (MV_OK != nRet)
						{
							cout << "配置主相机触发输出失败！" << endl;
							break;
						}
						
						nRet = camera[1]->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE2);
						if (MV_OK != nRet)
						{
							cout << "配置从相机触发源为line2硬触发失败！" << endl;
							break;
						}
						cout << endl << "*****开始拍摄图像*******" << endl;
						cout << "(按N触发拍摄，按S存图并进入下一组)" << endl << endl;
						for (int i = 0; i < image_nums; i++) {
							cout << "第" << i + 1 << '/' << image_nums << "组：";
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
											cout << name[0] + "相机软触发失败!" << endl;
											cout << "请尝试再次按N进行触发" << endl;
											break;
										}
										m_bFinishedShoot = true;
										Sleep(200);
										break;
									}

									else if (c == 'S' || c == 's') {
										if (SaveImage(camera)) {
											grab_flag = false;
											cout << "存图成功!" << endl;
										}
										else {
											cout << "存图失败！请重新尝试保存或拍摄" << endl;
										}
										break;
									}
								}
							} while (grab_flag);
						}

						break;
					}
					}
					cout << "全部图片拍摄完毕！" << endl;
					g_bExit = true;

					for (int i = 0; i < 2; i++) {
						nRet = camera[i]->StopGrabbing();
						if (MV_OK != nRet)
						{
							cout << name[i] + "相机停止取流失败！" << endl;
						}
						// 销毁取流线程
						WaitForSingleObject(m_hGrabThread[i], INFINITE);
						CloseHandle(m_hGrabThread[i]);
						m_hGrabThread[i] = NULL;
						// 销毁窗口对象
						CloseHandle(m_hCreateWindow[i]);
						PostMessage(g_hwnd[i], WM_CLOSE, 0, 0);
						m_hCreateWindow[i] = NULL;
					}
					// 生成图片列表
					if (!ImageName.empty() || ImageName.size() < image_nums * 2) {
						string temp = output_loc + "stereo_calib.xml";
						FileStorage fs(temp, FileStorage::WRITE); 
						fs << "imagelist" << ImageName;
						cout << "文件列表生成成功！" << endl;
						fs.release();
					}
					else
						cout << "文件列表生成失败！图片缺失" << endl;
					ImageName.clear();
				} while (0);
				

				
				for (int i = 0; i < 2; i++) {
					// 释放相机对象
					if(camera[i] != nullptr) {
						nRet = camera[i]->Close();
						delete(camera[i]);
						camera[i] = nullptr;	
					}
					// 释放缓存
					if (m_pSaveImageBuf[i])
					{
						free(m_pSaveImageBuf[i]);
						m_pSaveImageBuf[i] = NULL;
					}
					m_nSaveImageBufSize[i] = 0;
				}
				break;
			}	

			// 功能二：相机标定
			case 2:
			{
				TiXmlDocument calib_param;
				if (!calib_param.LoadFile(calib_param_loc.c_str()))
				{
					cout << "当前不存在标定参数配置文件！" << endl;
					cout << "请先进行标定参数配置" << endl;
					break;
				}
				CalibPattern* p_calib_param = new CalibPattern;
				if (Param_WR::ReadCalibParam(calib_param, p_calib_param) == 0)
				{
					cout << "无法读取标定参数文件，请检查文件是否损坏或占用！" << endl;
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
				// 配置输入输出文件夹
				camera_calib->image_folder = p_calib_param->image_folder ;
				camera_calib->cal_debug_folder = p_calib_param->cal_debug_folder;
				if (!camera_calib->readStringList()) {
					cout << "无法打开 " << camera_calib->image_folder + "/stereo_calib.xml" << " 或文件不存在！" << endl;
					break;
				}
				camera_calib->ExtractTarget();
				camera_calib->StereoCalib();
				// 对象析构
				if (camera_calib) {
					delete camera_calib;
					camera_calib = nullptr;
				}
				break;
			}

			// 功能三：修改相机参数
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
					cout << "                  当前相机参数信息" << endl;
					cout << "左相机参数：" << endl;
					Param_WR::PrintCameraParam(left_camera);
					cout << "右相机参数：" << endl;
					Param_WR::PrintCameraParam(right_camera);
					cout << "**********************************************************" << endl;
				}
				// ch:枚举设备 | en:Enum device
				MV_CC_DEVICE_INFO_LIST stDeviceList;
				if (!MvCamera::EnumDevice(stDeviceList)) {
					if (!camera_param_flag) {
						cout << "只有连接相机后才能创建相机参数配置文件！" << endl;
						break;
					}
					cout << "当前无连接相机，请问是否要修改相机参数配置文件（0：不, 1：是）：";
					bool rec_flag = false;
					cin >> rec_flag;
					if (!rec_flag)
						break;
					// 左相机参数配置
					cout << "请修改左相机参数设置!" << endl;
					cout << "相机序列号（当前值：" << left_camera->SerialNumber << ")：";
					cin >> left_camera->SerialNumber;
					cout << "相机图像格式（当前值：" << left_camera->PixelFormat << ")[可输入项：Mono8，Mono10，Mono10Packed，Mono12，Mono12Packed]：";
					string PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							left_camera->PixelFormat = PF;
							break;
						}
						cout << "输入格式不匹配，请重新输入：";
					}
					cout << "相机曝光时间（当前值：" << left_camera->ExposureTime << ")：";
					cin >> left_camera->ExposureTime;
					cout << "相机增益（当前值：" << left_camera->Gain << ")：";
					cin >> left_camera->Gain;
					// 右相机参数配置
					cout << "请修改右相机参数设置!" << endl;
					cout << "相机序列号（当前值：" << right_camera->SerialNumber << ")：";
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
							cout << "序列号冲突，左右相机不能为同一个，请重新输入！" << endl;
						}
					}
					cout << "相机图像格式（当前值：" << right_camera->PixelFormat << ")[可输入项：Mono8，Mono10，Mono10Packed，Mono12，Mono12Packed]：";
					PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							if (PF == left_camera->PixelFormat) {
								right_camera->PixelFormat = PF;
								break;
							}
							else
								cout << "图像格式需保证左右相机一致，请重新输入：";

						}
						else {
							cout << "输入格式不匹配，请重新输入：";
						}
					}
					cout << "相机曝光时间（当前值：" << right_camera->ExposureTime << ")：";
					cin >> right_camera->ExposureTime;
					cout << "相机增益（当前值：" << right_camera->Gain << ")：";
					cin >> right_camera->Gain;
					if (Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "修改相机参数文件成功！" << endl << endl;
					else
						cout << endl << endl << "修改相机参数文件失败！" << endl << endl;	
				}
				else {
					// 左相机参数配置
					cout << "*******请选择左相机******" << endl;
					printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
					unsigned int nIndex = 0;
					while (cin >> nIndex)
					{
						if (nIndex >= stDeviceList.nDeviceNum)
						{
							cout << "Input error!" << endl;
							cout << "请重新输入：";
							continue;
						}
						break;
					}
					GetCameraParam(left_camera, stDeviceList.pDeviceInfo[nIndex]);
					cout << "请修改左相机参数设置!" << endl;
					cout << "相机图像格式（当前值：" << left_camera->PixelFormat << ")[可输入项：Mono8，Mono10，Mono10Packed，Mono12，Mono12Packed]：";
					string PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							left_camera->PixelFormat = PF;
							break;
						}
						cout << "输入格式不匹配，请重新输入：";
					}
					cout << "相机曝光时间（当前值：" << left_camera->ExposureTime << ")：";
					cin >> left_camera->ExposureTime;
					cout << "相机增益（当前值：" << left_camera->Gain << ")：";
					cin >> left_camera->Gain;

					// 右相机参数配置
					nIndex = 0;
					while (1) {
						cout << "*******请选择右相机******" << endl;
						printf("Please Input camera index(0-%d):", stDeviceList.nDeviceNum - 1);
						while (cin >> nIndex)
						{
							if (nIndex >= stDeviceList.nDeviceNum)
							{
								cout << "Input error!" << endl;
								cout << "请重新输入：";
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
							cout << "序列号冲突，左右相机不能为同一个，请重新选择！" << endl;
						}
					}
					GetCameraParam(right_camera, stDeviceList.pDeviceInfo[nIndex]);
					cout << "请修改右相机参数设置!" << endl;
					cout << "相机图像格式（当前值：" << right_camera->PixelFormat << ")[可输入项：Mono8，Mono10，Mono10Packed，Mono12，Mono12Packed]：";
					PF = "";
					while (cin >> PF) {
						if (PF == "Mono8" || PF == "Mono10" || PF == "Mono10Packed" || PF == "Mono12" || PF == "Mono12Packed") {
							if (PF == left_camera->PixelFormat) {
								right_camera->PixelFormat = PF;
								break;
							}
							else
								cout << "图像格式需保证左右相机一致，请重新输入：";
							
						}
						else {
							cout << "输入格式不匹配，请重新输入：";
						}
					}
					cout << "相机曝光时间（当前值：" << right_camera->ExposureTime << ")：";
					cin >> right_camera->ExposureTime;
					cout << "相机增益（当前值：" << right_camera->Gain << ")：";
					cin >> right_camera->Gain;
					if(Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "创建相机参数文件成功！" << endl << endl;
					else
						cout << endl << endl << "创建相机参数文件失败！" << endl << endl;
				}		
				break;
			}
				
			// 功能四：修改标定参数
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
				cout << "请输入标定图像输入文件夹：";
				cin >> p_calib_param->image_folder;
				cout << "请输入调试目录（用于存储标定结果及提取特征点的图像）：";
				cin >> p_calib_param->cal_debug_folder;
				cout << "请输入横向特征点数目：";
				cin >> p_calib_param->sqXnum;
				cout << "请输入纵向特征点数：";
				cin >> p_calib_param->sqYnum;
				cout << "请输入特征点间距（实际物理尺度）：";
				cin >> p_calib_param->TargetSize;
				cout << "请选择标定图案类型：" << endl;
				cout << "1.棋盘格图案" << endl;
				cout << "2.对称圆点图案（无标记点）" << endl;
				cout << "3.对称圆点图案（有标记点）" << endl;
				cout << "选择图案：";
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
							cout << "请输入起始标志点X坐标：";
							cin >> p_calib_param->origin_offset_x;
							cout << "请输入起始标志点Y坐标：";
							cin >> p_calib_param->origin_offset_y;
							cout << "请输入起始点到右方标志点距离：";
							cin >> p_calib_param->inner_pattern_width;
							cout << "请输入起始点到上方标志点距离：";
							cin >> p_calib_param->inner_pattern_height;
							
							break;
						}
						}
						break;
					}
					else {
						cout << "请重新输入合法选项！" << endl;
					}
				}
				if (Param_WR::SetCalibParam(calib_param_loc, p_calib_param))
					cout << endl << endl << "创建标定参数文件成功！" << endl << endl;
				else
					cout << endl << endl << "创建标定参数文件失败！" << endl << endl;

				break;
			}

			// 功能五：生成标定图像
			case 5:
			{
				PrintPatternOptions();
				char pattern_sel;
				while (cin >> pattern_sel) {
					if ((pattern_sel >= '1' && pattern_sel <= '3')) {
						CalibPattern C_P;
						cout << "请输入所需生成的图像宽度（单位：像素）：";
						cin >> C_P.bkgWidth;
						cout << "请输入所需生成的图像高度（单位：像素）：";
						cin >> C_P.bkgHeight;
						cout << "请输入所需生成的图案横向特征点数：";
						cin >> C_P.sqXnum;
						cout << "请输入所需生成的图案纵向特征点数（默认为0时，横向与纵向格子数一致）：";
						cin >> C_P.sqYnum;
						cout << "请输入所需生成的图案边框大小（像素）：";
						cin >> C_P.thickNum;
						cout << "请输入保存文件路径（格式PNG/JPEG/BMP）：";
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
							cout << "请输入起始标志点X坐标(最小：0，最大：" << C_P.sqXnum <<  ")：";
							cin >> C_P.origin_offset_x;
							cout << "请输入起始标志点Y坐标(最小：0，最大：" << C_P.sqYnum << ")：";
							cin >> C_P.origin_offset_y;
							cout << "请输入起始点到上方标志点距离：";
							cin >> C_P.inner_pattern_height;
							cout << "请输入起始点到右方标志点距离：";
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
						cout << "请输入正确选项！！" << endl << endl;
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
			cout << "请输入正确选项！！" << endl << endl;
		}
		PrintOptions();
	}
	return 0;
}



// 畸变校正测试代码
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



