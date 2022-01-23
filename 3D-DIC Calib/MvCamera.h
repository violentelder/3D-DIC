// 名称：相机操作函数
// 作者：吴文浩
// 创建时间：2021.7.5
#pragma once

// 常用函数库
#include "CommonFunction.h"

#ifndef MV_NULL
#define MV_NULL    0
#endif

using namespace std;


class MvCamera
{
public:
	MvCamera();
	~MvCamera();

	int index;

	// ch:枚举设备 | en:Enumerate Device
	static bool EnumDevice(MV_CC_DEVICE_INFO_LIST &stDeviceList, bool PrintList = true);

	// ch:判断设备是否可达 | en:Is the device accessible
	static bool IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode);

	// ch:打开设备 | en:Open Device
	int Open(MV_CC_DEVICE_INFO* pstDeviceInfo);

	// ch:关闭设备 | en:Close Device
	int Close();

	// ch:开启抓图 | en:Start Grabbing
	int StartGrabbing();

	// ch:停止抓图 | en:Stop Grabbing
	int StopGrabbing();

	// ch:主动获取一帧图像数据 | en:Get one frame initiatively
	int GetImageBuffer(MV_FRAME_OUT* pFrame, int nMsec);

	// ch:释放图像缓存 | en:Free image buffer
	int FreeImageBuffer(MV_FRAME_OUT* pFrame);

	// ch:获取设备信息 | en:Get device information
	int GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo);

	// ch:获取GEV相机的统计信息 | en:Get detect info of GEV camera
	int GetGevAllMatchInfo(MV_MATCH_INFO_NET_DETECT* pMatchInfoNetDetect);

	// ch:获取U3V相机的统计信息 | en:Get detect info of U3V camera
	int GetU3VAllMatchInfo(MV_MATCH_INFO_USB_DETECT* pMatchInfoUSBDetect);

	// ch:获取和设置Int型参数，如 Width和Height，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
	// en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX *pIntValue);
	int SetIntValue(IN const char* strKey, IN int64_t nValue);

	// ch:获取和设置Enum型参数，如 PixelFormat，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
	// en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE *pEnumValue);
	int SetEnumValue(IN const char* strKey, IN unsigned int nValue);
	int SetEnumValueByString(IN const char* strKey, IN const char* sValue);

	// ch:获取和设置Float型参数，如 ExposureTime和Gain，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
	// en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE *pFloatValue);
	int SetFloatValue(IN const char* strKey, IN float fValue);

	// ch:获取和设置Bool型参数，如 ReverseX，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
	// en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int GetBoolValue(IN const char* strKey, OUT bool *pbValue);
	int SetBoolValue(IN const char* strKey, IN bool bValue);

	// ch:获取和设置String型参数，如 DeviceUserID，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件UserSetSave
	// en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int GetStringValue(IN const char* strKey, MVCC_STRINGVALUE *pStringValue);
	int SetStringValue(IN const char* strKey, IN const char * strValue);

	// ch:执行一次Command型命令，如 UserSetSave，详细内容参考SDK安装目录下的 MvCameraNode.xlsx 文件
	// en:Execute Command once, such as UserSetSave, for details please refer to MvCameraNode.xlsx file under SDK installation directory
	int CommandExecute(IN const char* strKey);

	// ch:像素格式转换 | en:Pixel format conversion
	int ConvertPixelType(MV_CC_PIXEL_CONVERT_PARAM* pstCvtParam);

	// ch:保存图片 | en:save image
	int SaveImage(MV_SAVE_IMAGE_PARAM_EX* pstParam);
	int SaveImageToFile(MV_SAVE_IMG_TO_FILE_PARAM* pstSaveFileParam);

	// ch:显示一帧图像 | en:Display one frame image
	int DisplayOneFrame(MV_DISPLAY_FRAME_INFO* pDisplayInfo);

protected:
	// ch:打印设备信息
	static bool PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo);

private:
	// 相机句柄
	void*               m_hDevHandle;
	// 相机信息结构体
	MV_CC_DEVICE_INFO	m_stDeviceInfo;
};

