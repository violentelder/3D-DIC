// ���ƣ����ú�����
// ���ߣ����ĺ�
// ����ʱ�䣺2021.7.29

#pragma once
#include <iostream>
#include <fstream>
#include <ctype.h>
#include <sstream>
#include <string>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <unordered_map>
#include<cstdlib>
#include<ctime>

// OpenCV������
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// ���������
#include "MvCameraControl.h"

using namespace std;
using namespace cv;

#define DEBUG_MSG(x) cout << "[DEBUG]" << x << endl;
#define WARN_MSG(x) cout << "[WARNING]" << x << endl;
#define PI		3.14159265358979323846
#define TWOPI	6.28318530717958647692

// �궨ͼ������
enum calib_pattern
{
	CALIB_PATTERN_CHECKERBOARD = 0,		/// ���̸�ͼ��
	CALIB_PATTERN_CIRCULAR,				/// �Գ��ޱ��Բ��ͼ��
	CALIB_PATTERN_SPECIAL,				/// �Գ��б��Բ��ͼ��
	// ������MAX������ͼ������
	MAX_TARGET_TYPE,
	NO_SUCH_TARGET_TYPE
};



//#define CALIB_PATTERN_CHECKERBOARD		0x0			/// ���̸�ͼ��
//#define CALIB_PATTERN_CIRCULAR			0x1			/// �Գ��ޱ��Բ��ͼ��
//#define CALIB_PATTERN_SPECIAL				0x2			/// �Գ��б��Բ��ͼ��


// ch:�궨ͼ���ṹ��
typedef struct CalibPattern
{
	calib_pattern		PatternType;				// �궨ͼ������
	unsigned int		sqXnum;						// �����������������̸񣺽ǵ㣬Բ�㣺Բ�ģ�
	unsigned int		sqYnum = 0;					// ��������������Ĭ��Ϊ0ʱ�����������������һ�£�
	
	// �궨ͼ������ΪCALIB_PATTERN_SPECIALʱ��Ҫ����
	int				origin_offset_x = -1;			// ��ʼ��־��X����
	int				origin_offset_y = -1;			// ��ʼ��־��Y����
	int				inner_pattern_width = -1;		// ��ʼ�㵽�ҷ���־�����
	int				inner_pattern_height = -1;		// ��ʼ�㵽�Ϸ���־�����
	

	// ���ɱ궨ͼ��ʱ��Ҫ����
	unsigned int		bkgWidth;					// ͼ�������أ�
	unsigned int		bkgHeight;					// ͼ���ߣ����أ�
	unsigned int		thickNum = 0;				// ͼ���߿��С�����أ�
	string				savePath = "";				// ����·��

	// ���б궨ʱ��Ҫ����
	float			TargetSize = 1.0;				// �������ࣨ��λ����ʵ������ϵ�߶ȣ�
	string			image_folder;					// �궨ͼ���ļ���
	string			cal_debug_folder;				// ����·��
	int				cailb_option;					// �궨�㷨ѡ��


	// �ж�����Ľṹ����ֵ�Ƿ���ȷ(���ڻ���ͼ��ʱ�����ж�)
	bool check() {
		if (savePath == "")
		{
			cout << "����·������Ϊ��" << endl;
			return false;
		}
		if (PatternType == CALIB_PATTERN_SPECIAL) {
			if (origin_offset_x < 0 || origin_offset_y < 0 ||
				inner_pattern_height < 0 || inner_pattern_width < 0 ||
				origin_offset_x + inner_pattern_height > sqYnum ||
				origin_offset_y + origin_offset_y > sqXnum)
			{
				cout << "��־��������Ϸ�" << endl;
				return false;
			}
		}
		return true;
	}
}CalibPattern;

// floatתstring
string ftos(const float &Num);

// intתstring
string itos(const int &Num);

// �����ļ�Ŀ¼
void create_directory(const string & folder);

// ���������ľ���
float dist2(KeyPoint pnt1, KeyPoint pnt2);

// ��������ֵ����Ƕ�
float angle_2d(const float & x1, const float & y1, const float & x2, const float & y2);


//�����������ɴ�С����
void order_dist3(vector<float> & dist, vector<int> & dist_order);

// ����ͼ��ת��
void grid_to_image(IN const float & grid_x,
	IN const float & grid_y,
	OUT float & img_x,
	OUT float & img_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h);

// ͼ������ת��
void image_to_grid(IN const float & img_x,
	IN const float & img_y,
	OUT float & grid_x,
	OUT float & grid_y,
	IN const vector<float> & img_to_grdx,
	IN const vector<float> & img_to_grdy);

// �����߽��
void create_bounding_box(OUT vector<float> & box_x,
	OUT vector<float> & box_y,
	IN const int num_fiducials_x,
	IN const int num_fiducials_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h);

//���ı����ڵ�һ����
bool is_in_quadrilateral(const float & x,
	const float & y,
	const vector<float> & box_x,
	const vector<float> & box_y);

// ch:��������ת��ΪOpenCV֧�ֵ�Mat��ʽ
bool Convert2Mat(const MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char *pData, Mat &srcImage);


// ���ļ��ж�ȡ�ڲ���Ϣ
bool readIntrinsic(IN string filename, Mat &M1, Mat &D1, Mat &M2, Mat &D2);

// ���ļ��ж�ȡ�����Ϣ
bool readExtrinsic(IN string filename, Mat &R, Mat &T, Mat &R1, Mat &P1, Mat &R2, Mat &P2, Mat &Q);

// ��opencv�е�MAT�����xls������
bool MatToxls(IN Mat &img, IN string filename);

// ��stringת��Ϊcalib_pattern
calib_pattern string_to_target_type(string & in);

// ���궨ͼ������ת��Ϊstring
string pattern_to_string(calib_pattern in);


