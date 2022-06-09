// 名称：常用函数库
// 作者：吴文浩
// 创建时间：2021.7.29

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

// OpenCV依赖项
#include "opencv2/calib3d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

// 相机依赖项
#include "MvCameraControl.h"

using namespace std;
using namespace cv;

#define DEBUG_MSG(x) cout << "[DEBUG]" << x << endl;
#define WARN_MSG(x) cout << "[WARNING]" << x << endl;
#define PI		3.14159265358979323846
#define TWOPI	6.28318530717958647692

// 标定图案类型
enum calib_pattern
{
	CALIB_PATTERN_CHECKERBOARD = 0,		/// 棋盘格图案
	CALIB_PATTERN_CIRCULAR,				/// 对称无标记圆点图案
	CALIB_PATTERN_SPECIAL,				/// 对称有标记圆点图案
	// 请勿在MAX后新增图案类型
	MAX_TARGET_TYPE,
	NO_SUCH_TARGET_TYPE
};



//#define CALIB_PATTERN_CHECKERBOARD		0x0			/// 棋盘格图案
//#define CALIB_PATTERN_CIRCULAR			0x1			/// 对称无标记圆点图案
//#define CALIB_PATTERN_SPECIAL				0x2			/// 对称有标记圆点图案


// ch:标定图案结构体
typedef struct CalibPattern
{
	calib_pattern		PatternType;				// 标定图案类型
	unsigned int		sqXnum;						// 横向特征点数（棋盘格：角点，圆点：圆心）
	unsigned int		sqYnum = 0;					// 纵向特征点数（默认为0时，横向与纵向格子数一致）
	
	// 标定图案类型为CALIB_PATTERN_SPECIAL时需要输入
	int				origin_offset_x = -1;			// 起始标志点X坐标
	int				origin_offset_y = -1;			// 起始标志点Y坐标
	int				inner_pattern_width = -1;		// 起始点到右方标志点距离
	int				inner_pattern_height = -1;		// 起始点到上方标志点距离
	

	// 生成标定图案时需要输入
	unsigned int		bkgWidth;					// 图案宽（像素）
	unsigned int		bkgHeight;					// 图案高（像素）
	unsigned int		thickNum = 0;				// 图案边框大小（像素）
	string				savePath = "";				// 保存路径

	// 进行标定时需要输入
	float			TargetSize = 1.0;				// 特征点间距（单位根据实际坐标系尺度）
	string			image_folder;					// 标定图像文件夹
	string			cal_debug_folder;				// 调试路径
	int				cailb_option;					// 标定算法选项


	// 判断输入的结构体数值是否正确(仅在绘制图像时进行判断)
	bool check() {
		if (savePath == "")
		{
			cout << "保存路径不能为空" << endl;
			return false;
		}
		if (PatternType == CALIB_PATTERN_SPECIAL) {
			if (origin_offset_x < 0 || origin_offset_y < 0 ||
				inner_pattern_height < 0 || inner_pattern_width < 0 ||
				origin_offset_x + inner_pattern_height > sqYnum ||
				origin_offset_y + origin_offset_y > sqXnum)
			{
				cout << "标志点参数不合法" << endl;
				return false;
			}
		}
		return true;
	}
}CalibPattern;

// float转string
string ftos(const float &Num);

// int转string
string itos(const int &Num);

// 生成文件目录
void create_directory(const string & folder);

// 计算两点间的距离
float dist2(KeyPoint pnt1, KeyPoint pnt2);

// 根据坐标值计算角度
float angle_2d(const float & x1, const float & y1, const float & x2, const float & y2);


//将三个距离由大到小排序
void order_dist3(vector<float> & dist, vector<int> & dist_order);

// 网格到图像转换
void grid_to_image(IN const float & grid_x,
	IN const float & grid_y,
	OUT float & img_x,
	OUT float & img_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h);

// 图像到网格转换
void image_to_grid(IN const float & img_x,
	IN const float & img_y,
	OUT float & grid_x,
	OUT float & grid_y,
	IN const vector<float> & img_to_grdx,
	IN const vector<float> & img_to_grdy);

// 创建边界框
void create_bounding_box(OUT vector<float> & box_x,
	OUT vector<float> & box_y,
	IN const int num_fiducials_x,
	IN const int num_fiducials_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h);

//是四边形内的一个点
bool is_in_quadrilateral(const float & x,
	const float & y,
	const vector<float> & box_x,
	const vector<float> & box_y);

// ch:将数据流转换为OpenCV支持的Mat格式
bool Convert2Mat(const MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char *pData, Mat &srcImage);


// 从文件中读取内参信息
bool readIntrinsic(IN string filename, Mat &M1, Mat &D1, Mat &M2, Mat &D2);

// 从文件中读取外参信息
bool readExtrinsic(IN string filename, Mat &R, Mat &T, Mat &R1, Mat &P1, Mat &R2, Mat &P2, Mat &Q);

// 将opencv中的MAT保存成xls到本地
bool MatToxls(IN Mat &img, IN string filename);

// 将string转换为calib_pattern
calib_pattern string_to_target_type(string & in);

// 将标定图案类型转换为string
string pattern_to_string(calib_pattern in);


