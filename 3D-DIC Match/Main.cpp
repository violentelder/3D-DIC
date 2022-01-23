// 名称：形变测量程序
// 作者：吴文浩
// 创建时间：2021.10.12

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

#include "MvCamera.h"
#include "Param_WR.h"


using namespace std;
using namespace cv;


// 程序初始化参数
string camera_param_loc = "./camera_param.xml";			//相机参数路径
string algorithm_param_loc = "./algorithm_param.xml";	//算法参数路径
string intrinsic_filename = "./intrinsics.xml";			//内参文件路径
string extrinsic_filename = "./extrinsics.xml";			//外参文件路径
string output_loc = "./data/";							//拍摄图像输出路径
vector<string> ImageName;


// 相机存图参数
unsigned char* m_pSaveImageBuf[2];
unsigned int m_nSaveImageBufSize[2];
MV_FRAME_OUT_INFO_EX  m_stImageInfo[2];


// 相关匹配子区绘制参数
Mat temp_disp;
vector<Point> roi_polyline;
Point curpoint;
double sf = 1.0;
bool bFirstLButtenDown = true;
bool bDrawROIFinished = false;



bool g_bExit = false;
bool m_bFinishedShoot = false;
int imagenums = 1;
// 显示窗口
HWND g_hwnd[2] = { NULL, NULL };
Mat srcImage[2];
// 相机名称
string name[2] = { "左","右" };
string en_name[2] = { "left","right" };

// ch:等待按键输入 | en:Wait for key press
char WaitForKeyPress(void)
{
	while (!_kbhit())
	{
		Sleep(10);
	}
	return (char)_getch();
}

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

// ch:打印程序功能信息
void PrintOptions()
{
	cout << endl;
	cout << "*******请选择对应功能*******" << endl;
	cout << "1.形变检测在线模式" << endl;
	cout << "2.形变检测离线模式" << endl;
	cout << "3.配置相机参数" << endl;
	cout << "4.配置算法参数" << endl;
	cout << "q.退出标定程序" << endl << endl;
	cout << "选择功能：";
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
bool SaveImage(MvCamera *camera[], string Image_name)
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
		temp = en_name[i] + '_' + Image_name;
		sprintf_s(stSaveFileParam.pImagePath, 256, (output_loc + temp).c_str());
		int nRet = camera[i]->SaveImageToFile(&stSaveFileParam);
		if (MV_OK != nRet)
		{
			cout << "保存图片失败！" << endl;
			return FALSE;
		}

	}
	return TRUE;
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
			if (camera->index == 1)
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


// ch:子区ROI绘制鼠标事件
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
	case EVENT_RBUTTONDOWN://右键封闭多边形
		line(temp_disp, roi_polyline.back(), roi_polyline.front(), Scalar(0x00, 0x00, 0xff), 2, 3);
		bFirstLButtenDown = true;//wyj
		bDrawROIFinished = true;
		break;
	}
}






int main() {
	// 初始参数
	int nRet = MV_OK;

	cout << "**********************************************************" << endl;
	cout << "*                  双目DIC相机形变测量程序               *" << endl;
	cout << "*                      作者：吴文浩                      *" << endl;
	cout << "**********************************************************" << endl;

	
	char select;
	PrintOptions();
	create_directory(output_loc);
	create_directory("./output/");
	while (cin >> select) {
		if ((select >= '1' && select <= '4')) {
			// 返回初始菜单信号
			bool b_exit = false;
			switch (select - '0')
			{
			// 功能一：形变检测在线模式
			case 1:
			{
				// 设置尺度因子
				int scale = 1;
				// 读取内参文件
				FileStorage fs(intrinsic_filename, FileStorage::READ);
				string reinput = intrinsic_filename;
				while (!fs.isOpened()) {
					cout << "当前内参文件位置：" << reinput.c_str();
					cout << "无法打开内参文件，请检查文件位置，并重新输入（按q返回初始菜单）：";
					cin >> reinput;
					if (reinput.size() == 1 && reinput[0] == 'q')
					{
						b_exit = true;
						break;
					}
					else
						fs.open(reinput, FileStorage::READ);
				}
				// 退出返回主界面
				if (b_exit)
					break;
				// 更新内参文件地址，以便之后复用
				intrinsic_filename = reinput;
				// 加载内参
				Mat M1, D1, M2, D2;
				fs["M1"] >> M1;
				fs["D1"] >> D1;
				fs["M2"] >> M2;
				fs["D2"] >> D2;
				M1 *= scale;
				M2 *= scale;
				cout << "相机内参加载成功！" << endl;
				// 加载外参文件
				fs.open(extrinsic_filename, FileStorage::READ);
				reinput = extrinsic_filename;
				while (!fs.isOpened()) {
					cout << "当前外参文件位置：" << reinput.c_str();
					cout << "无法打开外参文件，请检查文件位置，并重新输入（按q返回初始菜单）：";
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
				// 更新外参文件地址，以便之后复用
				extrinsic_filename = reinput;
				// 加载外参
				Mat R, T, R1, P1, R2, P2;
				fs["R"] >> R;
				fs["T"] >> T;
				cout << "相机外参加载成功！" << endl;
				fs.release();



				cout << "------开始连接相机------" << endl;
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
							cout << "未找到匹配" + name[i] + "相机" << endl;
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

				// 拍摄参考图像
				do
				{
					g_bExit = false;

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
						cout << endl << "*****开始拍摄参考图像*******" << endl;
						cout << "(按N触发拍摄，按S存图并进入下一步)" << endl << endl;
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
									if (SaveImage(camera, "ref.bmp")) {
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
						cout << "(按N触发拍摄，按S存图并进入下一步)" << endl << endl;
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
									if (SaveImage(camera, "ref.bmp")) {
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
						break;
					}
					}
					cout << "参考图片拍摄完毕！" << endl;
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
				} while (0);

				// 参考图像子区绘制
				bool bSubstr_finished = true;
				do
				{
					bSubstr_finished = true;
					cout << endl << endl <<  "请选择子区绘制方式：" << endl;
					cout << "1.自适应全场测量" << endl;
					cout << "2.自行绘制感兴趣区域" << endl;
					cout << "请输入：";
					int sel_subalg;
					while (cin >> sel_subalg) {
						if (sel_subalg != 1 && sel_subalg != 2) {
							cout << "输入选项不合法，请重新输入！" << endl;
							cout << "请输入：";
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
						cout << endl << "*****开始绘制感兴趣区域*******" << endl;
						cout << "请选择感兴趣区域的图案样式：" << endl;
						cout << "1.矩形" << endl;
						cout << "2.多边形" << endl;
						cout << "请输入：";
						int sel_subtype;
						while (cin >> sel_subtype) {
							if (sel_subtype != 1 && sel_subtype != 2) {
								cout << "输入选项不合法，请重新输入！" << endl;
								cout << "请输入：";
								continue;
							}
							else
								break;
						}
						switch (sel_subtype)
						{
						case 1:			// 矩形模式
						{




							break;
						}
						case 2:			// 多边形模式
						{
							cout << "(按r重新绘制，按s存储当前多边形角点信息，按n进入下一步)" << endl << endl;
							namedWindow("绘制感兴趣区域");
							sf = 1020. / MAX(temp_disp.rows, temp_disp.cols);
							setMouseCallback("绘制感兴趣区域", MouseEventforROI, (void *)&temp_disp);
							bDrawROIFinished = false;
							Mat roi_disp;
							while (1) {
								resize(temp_disp, roi_disp, Size(), sf, sf, INTER_LINEAR_EXACT);
								if (!roi_polyline.empty() && !bDrawROIFinished)
								{
									Point temp_point = Point(roi_polyline.back().x * sf, roi_polyline.back().y * sf);
									line(roi_disp, temp_point, curpoint, Scalar(0x00, 0x00, 0xff), 2, 3);
								}

								imshow("绘制感兴趣区域", roi_disp);
								char sel_roi = waitKey(30);
								if (sel_roi == 'r' || sel_roi == 'R') {
									bDrawROIFinished = false;
									roi_polyline.clear();
									temp_disp = left_ref_image_disp.clone();
								}
								if (bDrawROIFinished) {
									if (sel_roi == 's' || sel_roi == 'S') {
										//Param_WR::WritePolyline("./output/polyline.xml", roi_polyline);
										cout << "多边形角点存储完毕" << endl;
									}
									else if (sel_roi == 'n' || sel_roi == 'N') {
										cout << "感兴趣区域绘制成功，进入下一步!" << endl;
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
					// 释放相机对象
					if (camera[i] != nullptr) {
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
			// 功能二：形变检测离线模式
			case 2:
			{
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
					if (Param_WR::SetCameraParam(camera_param_loc, left_camera, right_camera))
						cout << endl << endl << "创建相机参数文件成功！" << endl << endl;
					else
						cout << endl << endl << "创建相机参数文件失败！" << endl << endl;
				}
				break;
			}

			// 功能四：修改形变测量算法参数
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
			cout << "请输入正确选项！！" << endl << endl;
		}
		PrintOptions();
	}








	return 0;
}
































//int main()
//{
//	// 图像宽高
//	int width = 1400;
//	int height = 1000;
//	
//	// 边框大小
//	int thicknum = 2;
//	// 两个维度的特征圆点数目
//	int sqXnum = 14;
//	int sqYnum = 10;
//
//	// 根据输入的特征点数目，自适应计算圆点半径大小（此处预设两圆点圆心之间距离为4*半径）
//	int radius = min(width / (4 * sqXnum + 2), height / (4 * sqYnum + 2));
//	if (radius <= 0.01 * min(height, width)) {
//		cout << "警告：圆点过小，可能无法识别！" << endl;
//	}
//	int space = 4 * radius;
//	// 生成两个维度方向上的边缘空白
//	int x_st = (width - 2 * radius * (2 * sqXnum - 1)) / 2;
//	int y_st = (height - 2 * radius * (2 * sqYnum - 1)) / 2;
//
//	// 生成空白画布
//	Mat img(height + 2 * thicknum, width + 2 * thicknum, CV_8UC4, Scalar(255, 255, 255, 255));
//	// 生成起始点圆心坐标
//	int cir_x = x_st + radius + thicknum;
//	int cir_y = y_st + radius + thicknum;
//	// 绘制不标记的对称圆点
//	for (int i = 0; i < img.rows; i++) {
//		for (int j = 0; j < img.cols; j++) {
//			// 绘制边框
//			if (i < thicknum || i >= thicknum + height || j < thicknum || j >= thicknum + width) {
//				img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
//				continue;
//			}
//			// 绘制圆点
//			if (cir_y >= img.rows - y_st - thicknum) {
//				continue;
//			}
//			if (i == cir_y && j == cir_x) {
//				// 绘制圆点，LINE_AA得到的边缘最为光滑
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
//	// 绘制标记点
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
//	imshow("圆点标定图案", img);
//	waitKey(0);
//	return 0 ;
//}