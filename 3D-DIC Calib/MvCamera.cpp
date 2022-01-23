#include "MvCamera.h"
MvCamera::MvCamera()
{
	m_hDevHandle = MV_NULL;
	index = -1;
}


MvCamera::~MvCamera()
{
	if (m_hDevHandle)
	{
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = MV_NULL;
	}
}

// ch:ö���豸
bool MvCamera::EnumDevice(MV_CC_DEVICE_INFO_LIST &stDeviceList, bool PrintList)
{
	int nRet = MV_OK;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		cout << "ö���豸ʧ�ܣ������룺" << nRet << endl;
		return false;
	}
	if (PrintList) {
		if (stDeviceList.nDeviceNum > 0)
		{
			for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
			{
				printf("[device %d]:\n", i);
				MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
				if (NULL == pDeviceInfo)
				{
					break;
				}
				PrintDeviceInfo(pDeviceInfo);
			}
		}
		else
		{
			cout << "δ�ҵ������豸" << endl;
			return false;
		}
	}
	return true;
}

// ch:�ж��豸�Ƿ�ɴ� | en:Is the device accessible
bool MvCamera::IsDeviceAccessible(MV_CC_DEVICE_INFO* pstDevInfo, unsigned int nAccessMode)
{
	return MV_CC_IsDeviceAccessible(pstDevInfo, nAccessMode);
}

// ch:���豸 | en:Open Device
int MvCamera::Open(MV_CC_DEVICE_INFO* pstDeviceInfo)
{
	if (MV_NULL == pstDeviceInfo)
	{
		return MV_E_PARAMETER;
	}

	if (m_hDevHandle)
	{
		return MV_E_CALLORDER;
	}

	int nRet = MV_CC_CreateHandle(&m_hDevHandle, pstDeviceInfo);
	if (MV_OK != nRet)
	{
		return nRet;
	}

	nRet = MV_CC_OpenDevice(m_hDevHandle);
	if (MV_OK != nRet)
	{
		MV_CC_DestroyHandle(m_hDevHandle);
		m_hDevHandle = MV_NULL;
	}

	memcpy(&m_stDeviceInfo, pstDeviceInfo, sizeof(MV_CC_DEVICE_INFO));

	return nRet;
}

// ch:�ر��豸 | en:Close Device
int MvCamera::Close()
{
	if (MV_NULL == m_hDevHandle)
	{
		return MV_E_HANDLE;
	}

	MV_CC_CloseDevice(m_hDevHandle);

	int nRet = MV_CC_DestroyHandle(m_hDevHandle);
	m_hDevHandle = MV_NULL;

	return nRet;
}

// ch:����ץͼ | en:Start Grabbing
int MvCamera::StartGrabbing()
{
	return MV_CC_StartGrabbing(m_hDevHandle);
}

// ch:ֹͣץͼ | en:Stop Grabbing
int MvCamera::StopGrabbing()
{
	return MV_CC_StopGrabbing(m_hDevHandle);
}

// ch:������ȡһ֡ͼ������ | en:Get one frame initiatively
int MvCamera::GetImageBuffer(MV_FRAME_OUT* pFrame, int nMsec)
{
	return MV_CC_GetImageBuffer(m_hDevHandle, pFrame, nMsec);
}

// ch:�ͷ�ͼ�񻺴� | en:Free image buffer
int MvCamera::FreeImageBuffer(MV_FRAME_OUT* pFrame)
{
	return MV_CC_FreeImageBuffer(m_hDevHandle, pFrame);
}

// ch:��ȡ�豸��Ϣ | en:Get device information
int MvCamera::GetDeviceInfo(MV_CC_DEVICE_INFO* pstDevInfo)
{
	if (pstDevInfo)
	{
		memcpy(pstDevInfo, &m_stDeviceInfo, sizeof(MV_CC_DEVICE_INFO));

		return MV_OK;
	}

	return MV_E_PARAMETER;
}

// ch:��ȡGEV�����ͳ����Ϣ | en:Get detect info of GEV camera
int MvCamera::GetGevAllMatchInfo(MV_MATCH_INFO_NET_DETECT* pMatchInfoNetDetect)
{
	if (MV_NULL == pMatchInfoNetDetect)
	{
		return MV_E_PARAMETER;
	}

	MV_CC_DEVICE_INFO stDevInfo = { 0 };
	GetDeviceInfo(&stDevInfo);
	if (stDevInfo.nTLayerType != MV_GIGE_DEVICE)
	{
		return MV_E_SUPPORT;
	}

	MV_ALL_MATCH_INFO struMatchInfo = { 0 };

	struMatchInfo.nType = MV_MATCH_TYPE_NET_DETECT;
	struMatchInfo.pInfo = pMatchInfoNetDetect;
	struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_NET_DETECT);
	memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_NET_DETECT));

	return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// ch:��ȡU3V�����ͳ����Ϣ | en:Get detect info of U3V camera
int MvCamera::GetU3VAllMatchInfo(MV_MATCH_INFO_USB_DETECT* pMatchInfoUSBDetect)
{
	if (MV_NULL == pMatchInfoUSBDetect)
	{
		return MV_E_PARAMETER;
	}

	MV_CC_DEVICE_INFO stDevInfo = { 0 };
	GetDeviceInfo(&stDevInfo);
	if (stDevInfo.nTLayerType != MV_USB_DEVICE)
	{
		return MV_E_SUPPORT;
	}

	MV_ALL_MATCH_INFO struMatchInfo = { 0 };

	struMatchInfo.nType = MV_MATCH_TYPE_USB_DETECT;
	struMatchInfo.pInfo = pMatchInfoUSBDetect;
	struMatchInfo.nInfoSize = sizeof(MV_MATCH_INFO_USB_DETECT);
	memset(struMatchInfo.pInfo, 0, sizeof(MV_MATCH_INFO_USB_DETECT));

	return MV_CC_GetAllMatchInfo(m_hDevHandle, &struMatchInfo);
}

// ch:��ȡ������Int�Ͳ������� Width��Height����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Int type parameters, such as Width and Height, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::GetIntValue(IN const char* strKey, OUT MVCC_INTVALUE_EX *pIntValue)
{
	return MV_CC_GetIntValueEx(m_hDevHandle, strKey, pIntValue);
}

int MvCamera::SetIntValue(IN const char* strKey, IN int64_t nValue)
{
	return MV_CC_SetIntValueEx(m_hDevHandle, strKey, nValue);
}

// ch:��ȡ������Enum�Ͳ������� PixelFormat����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Enum type parameters, such as PixelFormat, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::GetEnumValue(IN const char* strKey, OUT MVCC_ENUMVALUE *pEnumValue)
{
	return MV_CC_GetEnumValue(m_hDevHandle, strKey, pEnumValue);
}

int MvCamera::SetEnumValue(IN const char* strKey, IN unsigned int nValue)
{
	return MV_CC_SetEnumValue(m_hDevHandle, strKey, nValue);
}

int MvCamera::SetEnumValueByString(IN const char* strKey, IN const char* sValue)
{
	return MV_CC_SetEnumValueByString(m_hDevHandle, strKey, sValue);
}

// ch:��ȡ������Float�Ͳ������� ExposureTime��Gain����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Float type parameters, such as ExposureTime and Gain, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::GetFloatValue(IN const char* strKey, OUT MVCC_FLOATVALUE *pFloatValue)
{
	return MV_CC_GetFloatValue(m_hDevHandle, strKey, pFloatValue);
}

int MvCamera::SetFloatValue(IN const char* strKey, IN float fValue)
{
	return MV_CC_SetFloatValue(m_hDevHandle, strKey, fValue);
}

// ch:��ȡ������Bool�Ͳ������� ReverseX����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Get Bool type parameters, such as ReverseX, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::GetBoolValue(IN const char* strKey, OUT bool *pbValue)
{
	return MV_CC_GetBoolValue(m_hDevHandle, strKey, pbValue);
}

int MvCamera::SetBoolValue(IN const char* strKey, IN bool bValue)
{
	return MV_CC_SetBoolValue(m_hDevHandle, strKey, bValue);
}

// ch:��ȡ������String�Ͳ������� DeviceUserID����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�UserSetSave
// en:Get String type parameters, such as DeviceUserID, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::GetStringValue(IN const char* strKey, MVCC_STRINGVALUE *pStringValue)
{
	return MV_CC_GetStringValue(m_hDevHandle, strKey, pStringValue);
}

int MvCamera::SetStringValue(IN const char* strKey, IN const char* strValue)
{
	return MV_CC_SetStringValue(m_hDevHandle, strKey, strValue);
}

// ch:ִ��һ��Command������� UserSetSave����ϸ���ݲο�SDK��װĿ¼�µ� MvCameraNode.xlsx �ļ�
// en:Execute Command once, such as UserSetSave, for details please refer to MvCameraNode.xlsx file under SDK installation directory
int MvCamera::CommandExecute(IN const char* strKey)
{
	return MV_CC_SetCommandValue(m_hDevHandle, strKey);
}

// ch:���ظ�ʽת�� | en:Pixel format conversion
int MvCamera::ConvertPixelType(MV_CC_PIXEL_CONVERT_PARAM* pstCvtParam)
{
	return MV_CC_ConvertPixelType(m_hDevHandle, pstCvtParam);
}

// ch:����ͼƬ | en:save image
int MvCamera::SaveImage(MV_SAVE_IMAGE_PARAM_EX* pstParam)
{
	return MV_CC_SaveImageEx2(m_hDevHandle, pstParam);
}

int MvCamera::SaveImageToFile(MV_SAVE_IMG_TO_FILE_PARAM* pstSaveFileParam)
{
	return MV_CC_SaveImageToFile(m_hDevHandle, pstSaveFileParam);
}

// ch:������ʾ���ھ�� | en:Set Display Window Handle
int MvCamera::DisplayOneFrame(MV_DISPLAY_FRAME_INFO* pDisplayInfo)
{
	return MV_CC_DisplayOneFrame(m_hDevHandle, pDisplayInfo);
}


// ch:��ӡ�豸��Ϣ
bool MvCamera::PrintDeviceInfo(MV_CC_DEVICE_INFO* pstMVDevInfo)
{
	if (NULL == pstMVDevInfo)
	{
		printf("The Pointer of pstMVDevInfo is NULL!\n");
		return false;
	}
	if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
	{
		int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
		int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
		int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
		int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

		// ch:��ӡ��ǰ���ip���û��Զ������� | en:print current ip and user defined name
		printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
		printf("UserDefinedName: %s\n\n", pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
	}
	else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
	{
		printf("UserDefinedName: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
		printf("Serial Number: %s\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
		printf("Device Number: %d\n\n", pstMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);
	}
	else
	{
		printf("Not support.\n");
	}

	return true;
}
