// 名称：相机立体标定算法
// 作者：吴文浩
// 创建时间：2021.7.4
#pragma once

// 常用函数库
#include "CommonFunction.h"

using namespace cv;
using namespace std;


class calib_algorithm
{
public:
	// 构造函数
	calib_algorithm()
		: draw_intersection(true)
		, useCalibrated(true)
		, showRectified(true)
		, endtime(0.0)
		, num_fiducials_x(10)
		, num_fiducials_y(8)
		, cal_target_spacing_size(3.5)
	{
		create_directory("output");
		cailb_option = CALIB_FIX_ASPECT_RATIO +
			CALIB_ZERO_TANGENT_DIST +
			CALIB_USE_INTRINSIC_GUESS +
			CALIB_RATIONAL_MODEL +
			CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5;
	};

	calib_algorithm(CalibPattern Calib_P)
		: draw_intersection(true)
		, useCalibrated(true)
		, showRectified(true)
		, endtime(0)
	{
		create_directory(Calib_P.cal_debug_folder);
		num_fiducials_x = Calib_P.sqXnum;
		num_fiducials_y = Calib_P.sqYnum == 0 ? Calib_P.sqXnum : Calib_P.sqYnum;
		cal_target_spacing_size = Calib_P.TargetSize;
		cailb_option = CALIB_FIX_ASPECT_RATIO +
			CALIB_ZERO_TANGENT_DIST +
			CALIB_USE_INTRINSIC_GUESS +
			CALIB_RATIONAL_MODEL +
			CALIB_FIX_K3 + CALIB_FIX_K4 + CALIB_FIX_K5;
	}

	// 虚析构函数
	virtual ~calib_algorithm() {};

	// 计时变量
	double endtime;

	string image_folder;
	string cal_debug_folder;
	vector<string> imagelist;

	// 每对图像的均方根重投影误差
	Mat perViewErrors;

	int nimages;
	bool useCalibrated;
	bool draw_intersection;
	bool showRectified;
	// 张正友标定算法选项
	int cailb_option;

	// 相机标定矩阵
	Mat cameraMatrix[2], distCoeffs[2];

	// 旋转举证、平移向量、本征矩阵、基本矩阵
	Mat R, T, E, F;	

	// 外参
	Mat R1, R2, P1, P2, Q;

	// 读取标定图像列表
	bool readStringList();	

	// 计算重投影误差
	static double computeReprojectionErrors(
		const vector<vector<Point3f> >& objectPoints,
		const vector<vector<Point2f> >& imagePoints,
		const vector<Mat>& rvecs, const vector<Mat>& tvecs,
		const Mat& cameraMatrix, const Mat& distCoeffs,
		vector<float>& perViewErrors);

	// 提取特征点
	virtual void ExtractTarget() = 0;

	// 标定函数
	void StereoCalib();

	// 生成标定图像
	static bool MakeCailbPattern(CalibPattern &Calib_P);

protected:
	// 编号0为左相机，编号1为右相机
	vector<vector<Point2f> > imagePoints[2];
	vector<vector<Point3f> > objectPoints;
	Size imageSize;
	vector<string> goodImageList;
	// 图案特征点数
	int num_fiducials_x;
	int num_fiducials_y;
	// 特征点大小（棋盘格：方格大小；圆点：两个圆心之间的距离）
	float cal_target_spacing_size;
	
};

// 棋盘格标定算法
class calib_checkerboard : public calib_algorithm 
{
public:
	calib_checkerboard()
		: boardSize(9, 6){};

	calib_checkerboard(CalibPattern Calib_P)
		: calib_algorithm(Calib_P)
		, boardSize(num_fiducials_x, num_fiducials_y){};

	~calib_checkerboard() { };

	
	Size boardSize;
	

	// 提取棋盘格角点
	void ExtractTarget();

protected:
	Mat cimg, cimg1;
	const int maxScale = 2;
	
	
};

// 圆点标定算法
class calib_circular : public calib_algorithm
{
public:
	calib_circular() 
		:patternSize(9, 6) {};
	calib_circular(CalibPattern Calib_P)
		: calib_algorithm(Calib_P)
		, patternSize(num_fiducials_x, num_fiducials_y) {};


	~calib_circular() {};

	// 提取圆点
	void ExtractTarget();

	Size patternSize;

private:
	Mat cimg, cimg1;
	const int maxScale = 2;

};

// 带标记点的圆点标定算法
class calib_circular_new : public calib_algorithm
{
public:
	// 构造函数
	calib_circular_new() 
		: origin_loc_x(2)
		, origin_loc_y(2)
		, num_fiducials_origin_to_x_marker(6)
		, num_fiducials_origin_to_y_marker(4){
		cailb_option = CALIB_ZERO_TANGENT_DIST + CALIB_USE_INTRINSIC_GUESS;
	};
	calib_circular_new(CalibPattern Calib_P) 
		:calib_algorithm(Calib_P){
		origin_loc_x = Calib_P.origin_offset_x;
		origin_loc_y = Calib_P.origin_offset_y;
		num_fiducials_origin_to_x_marker = Calib_P.inner_pattern_width;
		num_fiducials_origin_to_y_marker = Calib_P.inner_pattern_height;
		cailb_option = CALIB_ZERO_TANGENT_DIST + CALIB_USE_INTRINSIC_GUESS;

	};
	// 析构函数
	~calib_circular_new() {};
	
	// 提取算法默认参数（可修改）
	//// 提取到的特征点坐标与预期位置的可接受差异（网格坐标系）
	const double dot_tol = 0.25;
	//// 图像二值化参数
	int threshold_start = 20;
	int threshold_end = 250;
	int threshold_step = 5;
	bool preview_thresh = false;
	int threshold_mode = 0;
	//// 自适应二值化参数
	const bool use_adaptive = false;
	const int filter_mode = 1;
	//// 圆点提取算法参数
	const int block_size_default = 75; 
	const int min_blob_size_default = 100;
	//// 左右相机图像包含的共同点数量比例
	const float image_set_tol = 0.75;

	// 提取带标记点的圆点标定算法
	void ExtractTarget();

	// 获取其他圆点的像素坐标
	int get_dot_targets(Mat & img,
		vector<cv::KeyPoint> & key_points,
		vector<cv::KeyPoint> & img_points,
		vector<cv::KeyPoint> & grd_points,
		int & return_thresh);

protected:
	int origin_loc_x;
	int origin_loc_y;
	int num_fiducials_origin_to_x_marker;
	int num_fiducials_origin_to_y_marker;
	int patternType = CALIB_PATTERN_SPECIAL;

	// 获取特殊标记点的像素坐标
	void get_dot_markers(IN Mat img,
		OUT vector<KeyPoint> & keypoints,
		IN int thresh,
		IN bool invert,
		IN const int min_size);

	// 将关键点重新排序为原点、x、y 顺序
	void reorder_keypoints(vector<KeyPoint> & keypoints);

	// 计算转换系数
	void calc_trans_coeff(IN vector<cv::KeyPoint> & imgpoints,
		IN vector<cv::KeyPoint> & grdpoints,
		OUT vector<float> & img_to_grdx,
		OUT vector<float> & img_to_grdy,
		OUT vector<float> & grd_to_imgx,
		OUT vector<float> & grd_to_imgy);

	// 根据平均大小和点是否落在中心框内过滤数据
	void filter_dot_markers(vector<cv::KeyPoint>  dots,
		vector<cv::KeyPoint> & img_points,
		vector<cv::KeyPoint> & grd_points,
		const vector<float> & grd_to_imgx,
		const vector<float> & grd_to_imgy,
		const vector<float> & img_to_grdx,
		const vector<float> & img_to_grdy,
		float dot_tol,
		Mat img,
		bool draw);


private:
	vector<vector<vector<vector<Point2f> > > > image_points_;
	vector<bool> goodImage;
};
