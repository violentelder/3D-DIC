// ���ƣ��������궨�㷨
// ���ߣ����ĺ�
// ����ʱ�䣺2021.7.4
#pragma once

// ���ú�����
#include "CommonFunction.h"

using namespace cv;
using namespace std;


class calib_algorithm
{
public:
	// ���캯��
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

	// ����������
	virtual ~calib_algorithm() {};

	// ��ʱ����
	double endtime;

	string image_folder;
	string cal_debug_folder;
	vector<string> imagelist;

	// ÿ��ͼ��ľ�������ͶӰ���
	Mat perViewErrors;

	int nimages;
	bool useCalibrated;
	bool draw_intersection;
	bool showRectified;
	// �����ѱ궨�㷨ѡ��
	int cailb_option;

	// ����궨����
	Mat cameraMatrix[2], distCoeffs[2];

	// ��ת��֤��ƽ���������������󡢻�������
	Mat R, T, E, F;	

	// ���
	Mat R1, R2, P1, P2, Q;

	// ��ȡ�궨ͼ���б�
	bool readStringList();	

	// ������ͶӰ���
	static double computeReprojectionErrors(
		const vector<vector<Point3f> >& objectPoints,
		const vector<vector<Point2f> >& imagePoints,
		const vector<Mat>& rvecs, const vector<Mat>& tvecs,
		const Mat& cameraMatrix, const Mat& distCoeffs,
		vector<float>& perViewErrors);

	// ��ȡ������
	virtual void ExtractTarget() = 0;

	// �궨����
	void StereoCalib();

	// ���ɱ궨ͼ��
	static bool MakeCailbPattern(CalibPattern &Calib_P);

protected:
	// ���0Ϊ����������1Ϊ�����
	vector<vector<Point2f> > imagePoints[2];
	vector<vector<Point3f> > objectPoints;
	Size imageSize;
	vector<string> goodImageList;
	// ͼ����������
	int num_fiducials_x;
	int num_fiducials_y;
	// �������С�����̸񣺷����С��Բ�㣺����Բ��֮��ľ��룩
	float cal_target_spacing_size;
	
};

// ���̸�궨�㷨
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
	

	// ��ȡ���̸�ǵ�
	void ExtractTarget();

protected:
	Mat cimg, cimg1;
	const int maxScale = 2;
	
	
};

// Բ��궨�㷨
class calib_circular : public calib_algorithm
{
public:
	calib_circular() 
		:patternSize(9, 6) {};
	calib_circular(CalibPattern Calib_P)
		: calib_algorithm(Calib_P)
		, patternSize(num_fiducials_x, num_fiducials_y) {};


	~calib_circular() {};

	// ��ȡԲ��
	void ExtractTarget();

	Size patternSize;

private:
	Mat cimg, cimg1;
	const int maxScale = 2;

};

// ����ǵ��Բ��궨�㷨
class calib_circular_new : public calib_algorithm
{
public:
	// ���캯��
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
	// ��������
	~calib_circular_new() {};
	
	// ��ȡ�㷨Ĭ�ϲ��������޸ģ�
	//// ��ȡ����������������Ԥ��λ�õĿɽ��ܲ��죨��������ϵ��
	const double dot_tol = 0.25;
	//// ͼ���ֵ������
	int threshold_start = 20;
	int threshold_end = 250;
	int threshold_step = 5;
	bool preview_thresh = false;
	int threshold_mode = 0;
	//// ����Ӧ��ֵ������
	const bool use_adaptive = false;
	const int filter_mode = 1;
	//// Բ����ȡ�㷨����
	const int block_size_default = 75; 
	const int min_blob_size_default = 100;
	//// �������ͼ������Ĺ�ͬ����������
	const float image_set_tol = 0.75;

	// ��ȡ����ǵ��Բ��궨�㷨
	void ExtractTarget();

	// ��ȡ����Բ�����������
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

	// ��ȡ�����ǵ����������
	void get_dot_markers(IN Mat img,
		OUT vector<KeyPoint> & keypoints,
		IN int thresh,
		IN bool invert,
		IN const int min_size);

	// ���ؼ�����������Ϊԭ�㡢x��y ˳��
	void reorder_keypoints(vector<KeyPoint> & keypoints);

	// ����ת��ϵ��
	void calc_trans_coeff(IN vector<cv::KeyPoint> & imgpoints,
		IN vector<cv::KeyPoint> & grdpoints,
		OUT vector<float> & img_to_grdx,
		OUT vector<float> & img_to_grdy,
		OUT vector<float> & grd_to_imgx,
		OUT vector<float> & grd_to_imgy);

	// ����ƽ����С�͵��Ƿ��������Ŀ��ڹ�������
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
