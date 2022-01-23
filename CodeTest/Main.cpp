// 绘制多边形
//#define _CRT_SECURE_NO_WARNINGS
//#include "opencv2/calib3d/calib3d.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/core/utility.hpp"
//#include <string>
//#include <list>
//#include <vector>
//#include <cmath>
//
//using namespace std;
//using namespace cv;
//
//bool bFirstLButtenDown = true;//wyj
//bool bDrawLineEnding = false;//wyj
//
//
//void MyMouseCallbackForEnclosedArea(int event, int x, int y, int flags, void *param);
//void MouseEventforROI(int event, int x, int y, int flags, void *param);
//Point point_1, point_2, point_3;
//int ymin = 0;
//int ymax = 0;
//bool drawing_switch = false;
//bool bDrawROIFinished = false;
//Point curpoint;
//vector<Point> roi_polyline;
//vector<Point> point;
//
//Mat image = imread("C:\\images\\baboon.jpg", 0);
//Mat temp;
//
//void draw_line(Mat&img) //画线
//{
//	line(img, point_1, point_2, Scalar(0x00, 0x00, 0xff), 2, 3); /*red*/ //函数主体是line函数
//}
//
//
//typedef struct tagEDGE
//{
//	double xi;
//	double dx;
//	int ymax;
//	bool operator <(tagEDGE &e)
//	{
//		return xi<e.xi;
//	}
//}EDGE;//边
//
//void GetMinMax(vector<Point> &py, int &ymin, int &ymax);
//void CreateTable(vector< list<EDGE> > &nTable, int ymin, int ymax);
//void Painting(Mat&img, vector< list<EDGE> > &nTable, int ymin, int ymax);
//
//int main()
//{
//	point_1 = Point(0, 0);
//	point_2 = Point(0, 0);
//	point_3 = Point(0, 0);//进行点的初始化
//
//	namedWindow("Box Example");
//	//根据画线的需要将鼠标的回调函数设置为画封闭区域和直线
//	cvtColor(image, image, COLOR_GRAY2RGB);
//	setMouseCallback("Box Example", MyMouseCallbackForEnclosedArea, (void *)&image);
//	while (1)
//	{
//		/*cap >> image;*/
//		image.copyTo(temp);
//		if (drawing_switch)
//			draw_line(temp);
//			//line(temp, roi_polyline.back(), curpoint, Scalar(0x00, 0x00, 0xff), 2, 3);
//		imshow("Box Example", temp);
//		waitKey(30);
//	}
//
//	//imshow("Box Example", image);
//	//waitKey();
//
//	return 0;
//}
//
//void MouseEventforROI(int event, int x, int y, int flags, void *param)
//{
//	if (bDrawROIFinished)
//		return;
//	switch (event)
//	{
//	case EVENT_MOUSEMOVE:
//		if (!roi_polyline.empty()) curpoint = Point(x, y);
//		break;
//	case EVENT_LBUTTONDOWN:
//
//		if (bFirstLButtenDown)
//		{
//			roi_polyline.push_back(Point(x, y));
//			curpoint = Point(x, y);
//			bFirstLButtenDown = false;
//		}
//		else
//		{
//			curpoint = Point(x, y);
//		}
//		line(image, roi_polyline.back(), curpoint, Scalar(0x00, 0x00, 0xff), 2, 3);
//		roi_polyline.push_back(curpoint);
//		circle(image, roi_polyline.back(), 1, Scalar(0, 255, 0));
//		break;
//	case EVENT_RBUTTONDOWN://右键封闭多边形
//		line(image, roi_polyline.back(), roi_polyline.front(), Scalar(0x00, 0x00, 0xff), 2, 3);
//		bFirstLButtenDown = true;//wyj
//		bDrawROIFinished = true;
//		break;
//	}
//}
//
//
//
//
//void MyMouseCallbackForEnclosedArea(int event, int x, int y, int flags, void *param)
//
//{
//	switch (event)
//	{
//	case EVENT_MOUSEMOVE:
//		if (drawing_switch) point_2 = Point(x, y);
//		break;
//	case EVENT_LBUTTONDOWN:
//
//		if (bFirstLButtenDown)
//		{
//			point_1 = Point(x, y);//wyj
//			point_2 = point_1;//wyj
//			bFirstLButtenDown = false;
//		}
//		else
//		{
//			point_2 = point_1;
//			point_1 = Point(x, y);
//		}
//		draw_line(image);
//		//point_2 = point_1;
//		//point_1 = Point(x, y);
//		point.push_back(point_1);
//		if (!drawing_switch)point_3 = point_1;
//		circle(image, point_1, 1, Scalar(0, 255, 0));
//		drawing_switch = true;
//		break;
//	case EVENT_RBUTTONDOWN://右键封闭多边形
//		point_2 = point_3;
//		draw_line(image);
//		bFirstLButtenDown = true;//wyj
//		drawing_switch = false;
//		break;
//	}
//}
//
//
//
//void GetMinMax(vector<Point> &py, int &ymin, int &ymax)
////获取多边形的最小最大y值
//{
//
//	for (vector<Point>::iterator iter = py.begin(); iter != py.end(); ++iter)
//	{
//
//		if ((*iter).y<ymin)
//			ymin = (*iter).y;
//		if ((*iter).y>ymax)ymax = (*iter).y;
//
//	}
//
//}
//
//
//void CreateTable(vector< list<EDGE> > &nTable, int ymin, int ymax)
////创建每一扫描线所对应的边，并排序
//{
//
//	EDGE e;
//	for (int y = ymin + 1; y <= ymax; y++)
//	{
//
//		for (list<EDGE>::iterator iter = nTable[y - 1].begin(); iter != nTable[y - 1].end(); iter++)
//		{
//
//			if ((*iter).ymax>y)
//			{
//
//				e.xi = (*iter).xi + (*iter).dx;
//				e.dx = (*iter).dx;
//				e.ymax = (*iter).ymax;
//				nTable[y].push_back(e);
//
//			}
//
//		}
//		nTable[y - 1].sort();
//
//	}
//
//}
//
//void Painting(Mat&img, vector< list<EDGE> > &nTable, int ymin, int ymax)
////上色
//{
//
//	for (int y = ymin; y <= ymax; y++)
//	{
//
//		for (list<EDGE>::iterator iter = nTable[y].begin(); iter != nTable[y].end(); ++iter)
//		{
//
//			int x1 = int(ceil((*iter).xi));
//			++iter;
//			int x2 = int(floor((*iter).xi));
//			for (int i = x1; i <= x2; ++i)
//				circle(img, Point(i, y), 1, Scalar(0, 0, 255));
//
//		}
//
//	}
//
//}










// 立体匹配测试代码
//#define _CRT_SECURE_NO_WARNINGS
//#include "opencv2/calib3d/calib3d.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
//#include "opencv2/highgui.hpp"
//#include "opencv2/core/utility.hpp"
//
//#include <stdio.h>
//
//using namespace cv;
//
//static void print_help(char** argv)
//{
//	printf("\nDemo stereo matching converting L and R images into disparity and point clouds\n");
//	printf("\nUsage: %s <left_image> <right_image> [--algorithm=bm|sgbm|hh|sgbm3way] [--blocksize=<block_size>]\n"
//		"[--max-disparity=<max_disparity>] [--scale=scale_factor>] [-i=<intrinsic_filename>] [-e=<extrinsic_filename>]\n"
//		"[--no-display] [-o=<disparity_image>] [-p=<point_cloud_file>]\n", argv[0]);
//}
//
//static void saveXYZ(const char* filename, const Mat& mat)
//{
//	const double max_z = 1.0e4;
//	FILE* fp = fopen(filename, "wt");
//	for (int y = 0; y < mat.rows; y++)
//	{
//		for (int x = 0; x < mat.cols; x++)
//		{
//			Vec3f point = mat.at<Vec3f>(y, x);
//			if (fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z) continue;
//			fprintf(fp, "%f %f %f\n", point[0], point[1], point[2]);
//		}
//	}
//	fclose(fp);
//}
//
//int main(int argc, char** argv)
//{
//	std::string img1_filename = "";
//	std::string img2_filename = "";
//	std::string intrinsic_filename = "";
//	std::string extrinsic_filename = "";
//	std::string disparity_filename = "";
//	std::string point_cloud_filename = "";
//
//	enum { STEREO_BM = 0, STEREO_SGBM = 1, STEREO_HH = 2, STEREO_VAR = 3, STEREO_3WAY = 4 };
//	int alg = STEREO_SGBM;
//	int SADWindowSize, numberOfDisparities;
//	bool no_display;
//	float scale;
//
//	Ptr<StereoBM> bm = StereoBM::create(16, 9);
//	Ptr<StereoSGBM> sgbm = StereoSGBM::create(0, 16, 3);
//	cv::CommandLineParser parser(argc, argv,
//		"{@arg1|aloeL.jpg|}{@arg2|aloeR.jpg|}{help h||}{algorithm|bm|}{max-disparity|128|}{blocksize|5|}{no-display||}{scale|1|}{i|intrinsics.xml|}{e|extrinsics.xml|}{o|disparity.bmp|}{p|kkk.xyz|}");
//	if (parser.has("help"))
//	{
//		print_help(argv);
//		return 0;
//	}
//	img1_filename = samples::findFile(parser.get<std::string>(0));
//	img2_filename = samples::findFile(parser.get<std::string>(1));
//	if (parser.has("algorithm"))
//	{
//		std::string _alg = parser.get<std::string>("algorithm");
//		alg = _alg == "bm" ? STEREO_BM :
//			_alg == "sgbm" ? STEREO_SGBM :
//			_alg == "hh" ? STEREO_HH :
//			_alg == "var" ? STEREO_VAR :
//			_alg == "sgbm3way" ? STEREO_3WAY : -1;
//	}
//	numberOfDisparities = parser.get<int>("max-disparity");
//	SADWindowSize = parser.get<int>("blocksize");
//	scale = parser.get<float>("scale");
//	no_display = parser.has("no-display");
//	if (parser.has("i"))
//		intrinsic_filename = parser.get<std::string>("i");
//	if (parser.has("e"))
//		extrinsic_filename = parser.get<std::string>("e");
//	if (parser.has("o"))
//		disparity_filename = parser.get<std::string>("o");
//	if (parser.has("p"))
//		point_cloud_filename = parser.get<std::string>("p");
//	if (!parser.check())
//	{
//		parser.printErrors();
//		return 1;
//	}
//	if (alg < 0)
//	{
//		printf("Command-line parameter error: Unknown stereo algorithm\n\n");
//		print_help(argv);
//		return -1;
//	}
//	if (numberOfDisparities < 1 || numberOfDisparities % 16 != 0)
//	{
//		printf("Command-line parameter error: The max disparity (--maxdisparity=<...>) must be a positive integer divisible by 16\n");
//		print_help(argv);
//		return -1;
//	}
//	if (scale < 0)
//	{
//		printf("Command-line parameter error: The scale factor (--scale=<...>) must be a positive floating-point number\n");
//		return -1;
//	}
//	if (SADWindowSize < 1 || SADWindowSize % 2 != 1)
//	{
//		printf("Command-line parameter error: The block size (--blocksize=<...>) must be a positive odd number\n");
//		return -1;
//	}
//	if (img1_filename.empty() || img2_filename.empty())
//	{
//		printf("Command-line parameter error: both left and right images must be specified\n");
//		return -1;
//	}
//	if ((!intrinsic_filename.empty()) ^ (!extrinsic_filename.empty()))
//	{
//		printf("Command-line parameter error: either both intrinsic and extrinsic parameters must be specified, or none of them (when the stereo pair is already rectified)\n");
//		return -1;
//	}
//
//	if (extrinsic_filename.empty() && !point_cloud_filename.empty())
//	{
//		printf("Command-line parameter error: extrinsic and intrinsic parameters must be specified to compute the point cloud\n");
//		return -1;
//	}
//
//	int color_mode = alg == STEREO_BM ? 0 : -1;
//	Mat img1 = imread(img1_filename, color_mode);
//	Mat img2 = imread(img2_filename, color_mode);
//
//	if (img1.empty())
//	{
//		printf("Command-line parameter error: could not load the first input image file\n");
//		return -1;
//	}
//	if (img2.empty())
//	{
//		printf("Command-line parameter error: could not load the second input image file\n");
//		return -1;
//	}
//
//	if (scale != 1.f)
//	{
//		Mat temp1, temp2;
//		int method = scale < 1 ? INTER_AREA : INTER_CUBIC;
//		resize(img1, temp1, Size(), scale, scale, method);
//		img1 = temp1;
//		resize(img2, temp2, Size(), scale, scale, method);
//		img2 = temp2;
//	}
//
//	Size img_size = img1.size();
//
//	Rect roi1, roi2;
//	Mat Q;
//
//	if (!intrinsic_filename.empty())
//	{
//		// reading intrinsic parameters
//		FileStorage fs(intrinsic_filename, FileStorage::READ);
//		if (!fs.isOpened())
//		{
//			printf("Failed to open file %s\n", intrinsic_filename.c_str());
//			return -1;
//		}
//
//		Mat M1, D1, M2, D2;
//		fs["M1"] >> M1;
//		fs["D1"] >> D1;
//		fs["M2"] >> M2;
//		fs["D2"] >> D2;
//
//		M1 *= scale;
//		M2 *= scale;
//
//		fs.open(extrinsic_filename, FileStorage::READ);
//		if (!fs.isOpened())
//		{
//			printf("Failed to open file %s\n", extrinsic_filename.c_str());
//			return -1;
//		}
//
//		Mat R, T, R1, P1, R2, P2;
//		fs["R"] >> R;
//		fs["T"] >> T;
//
//		stereoRectify(M1, D1, M2, D2, img_size, R, T, R1, R2, P1, P2, Q, CALIB_ZERO_DISPARITY, -1, img_size, &roi1, &roi2);
//
//		Mat map11, map12, map21, map22;
//		initUndistortRectifyMap(M1, D1, R1, P1, img_size, CV_16SC2, map11, map12);
//		initUndistortRectifyMap(M2, D2, R2, P2, img_size, CV_16SC2, map21, map22);
//
//		Mat img1r, img2r;
//		remap(img1, img1r, map11, map12, INTER_LINEAR);
//		remap(img2, img2r, map21, map22, INTER_LINEAR);
//
//		img1 = img1r;
//		img2 = img2r;
//	}
//
//	numberOfDisparities = numberOfDisparities > 0 ? numberOfDisparities : ((img_size.width / 8) + 15) & -16;
//
//	bm->setROI1(roi1);
//	bm->setROI2(roi2);
//	bm->setPreFilterCap(31);
//	bm->setBlockSize(SADWindowSize > 0 ? SADWindowSize : 9);
//	bm->setMinDisparity(0);
//	bm->setNumDisparities(numberOfDisparities);
//	bm->setTextureThreshold(10);
//	bm->setUniquenessRatio(15);
//	bm->setSpeckleWindowSize(100);
//	bm->setSpeckleRange(32);
//	bm->setDisp12MaxDiff(1);
//
//	sgbm->setPreFilterCap(63);
//	int sgbmWinSize = SADWindowSize > 0 ? SADWindowSize : 3;
//	sgbm->setBlockSize(sgbmWinSize);
//
//	int cn = img1.channels();
//
//	sgbm->setP1(8 * cn*sgbmWinSize*sgbmWinSize);
//	sgbm->setP2(32 * cn*sgbmWinSize*sgbmWinSize);
//	sgbm->setMinDisparity(0);
//	sgbm->setNumDisparities(numberOfDisparities);
//	sgbm->setUniquenessRatio(10);
//	sgbm->setSpeckleWindowSize(100);
//	sgbm->setSpeckleRange(32);
//	sgbm->setDisp12MaxDiff(1);
//	if (alg == STEREO_HH)
//		sgbm->setMode(StereoSGBM::MODE_HH);
//	else if (alg == STEREO_SGBM)
//		sgbm->setMode(StereoSGBM::MODE_SGBM);
//	else if (alg == STEREO_3WAY)
//		sgbm->setMode(StereoSGBM::MODE_SGBM_3WAY);
//
//	Mat disp, disp8;
//	//Mat img1p, img2p, dispp;
//	//copyMakeBorder(img1, img1p, 0, 0, numberOfDisparities, 0, IPL_BORDER_REPLICATE);
//	//copyMakeBorder(img2, img2p, 0, 0, numberOfDisparities, 0, IPL_BORDER_REPLICATE);
//
//	int64 t = getTickCount();
//	float disparity_multiplier = 1.0f;
//	if (alg == STEREO_BM)
//	{
//		bm->compute(img1, img2, disp);
//		if (disp.type() == CV_16S)
//			disparity_multiplier = 16.0f;
//	}
//	else if (alg == STEREO_SGBM || alg == STEREO_HH || alg == STEREO_3WAY)
//	{
//		sgbm->compute(img1, img2, disp);
//		if (disp.type() == CV_16S)
//			disparity_multiplier = 16.0f;
//	}
//	t = getTickCount() - t;
//	printf("Time elapsed: %fms\n", t * 1000 / getTickFrequency());
//
//	//disp = dispp.colRange(numberOfDisparities, img1p.cols);
//	if (alg != STEREO_VAR)
//		disp.convertTo(disp8, CV_8U, 255 / (numberOfDisparities*16.));
//	else
//		disp.convertTo(disp8, CV_8U);
//	if (!no_display)
//	{
//		namedWindow("left", 1);
//		imshow("left", img1);
//		namedWindow("right", 1);
//		imshow("right", img2);
//		namedWindow("disparity", 0);
//		imshow("disparity", disp8);
//		printf("press any key to continue...");
//		fflush(stdout);
//		waitKey();
//		printf("\n");
//	}
//
//	if (!disparity_filename.empty())
//		imwrite(disparity_filename, disp8);
//
//	if (!point_cloud_filename.empty())
//	{
//		printf("storing the point cloud...");
//		fflush(stdout);
//		Mat xyz;
//		Mat floatDisp;
//		disp.convertTo(floatDisp, CV_32F, 1.0f / disparity_multiplier);
//		reprojectImageTo3D(floatDisp, xyz, Q, true);
//		saveXYZ(point_cloud_filename.c_str(), xyz);
//		printf("\n");
//	}
//
//	return 0;
//}





//// 圆点特征点提取测试代码
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/opencv.hpp>
//
//using namespace cv;
//using namespace std;
//
//int main() {
//	// Blob算子参数
//	SimpleBlobDetector::Params params;
//	/*params.minThreshold = 10;
//	params.maxThreshold = 200;*/
//	params.maxArea = 10e4;
//	params.minArea = 10;
//	params.filterByArea = true;
//	/*params.minDistBetweenBlobs = 5;
//	params.filterByInertia = false;
//	params.minInertiaRatio = 0.5;*/
//	Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);
//
//	Mat img = imread("right01_circulars.jpg", 0);
//	vector<Point2f> centers;
//	Size patternSize(9, 6);	
//	
//	// 提取圆点特征的圆心
//
//	// 无法提取到圆心
//	//bool found = findCirclesGrid(img, patternSize, centers, CALIB_CB_SYMMETRIC_GRID, blobDetector);
//
//	bool found = findCirclesGrid(img, patternSize, centers, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, blobDetector);
//	Mat cimg;
//	cvtColor(img, cimg, COLOR_GRAY2BGR);
//	drawChessboardCorners(cimg, patternSize, centers, found);
//
//	double sf = 960. / MAX(img.rows, img.cols);
//	resize(img, img, Size(), sf, sf, INTER_LINEAR_EXACT);
//	resize(cimg, cimg, Size(), sf, sf, INTER_LINEAR_EXACT);
//
//	imshow("原始图像", img);
//	imshow("corners", cimg);
//
//	waitKey();
//	return 0;
//}


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main()
{
	// 图像宽高
	int width = 2800;
	int height = 2000;
	
	// 边框大小
	int thicknum = 0;
	// 标定图案两个维度的大小
	int sqXnum = 11;
	int sqYnum = 4;		// 非对称圆点图案的一行实质是一组非对称圆点行

	sqXnum = sqXnum / 2 + 1;
	sqYnum = 2 * sqYnum;
	// 根据输入的特征点数目，自适应计算圆点半径大小（此处预设两圆点圆心之间距离为4*半径）
	int radius = min(width / (4 * sqXnum + 2), height / (2 * sqYnum + 2));
	if (radius <= 0.01 * min(height, width)) {
		cout << "警告：圆点过小，可能无法识别！" << endl;
	}
	
	int space = 4 * radius;
	// 生成两个维度方向上的边缘空白
	int x_st = (width - 2 * radius * (2 * sqXnum - 1)) / 2;
	int y_st = (height - radius * 2 * sqYnum) / 2;

	// 生成空白画布
	Mat img(height + 2 * thicknum, width + 2 * thicknum, CV_8UC4, Scalar(255, 255, 255, 255));
	// 生成起始点圆心坐标
	int cir_x = x_st + radius + thicknum;
	int cir_y = y_st + radius + thicknum;
	// 用于做非对称的偏移 
	int Asym_offset = 0;
	int y_count = 0;
	// 绘制非对称圆点图案
	for (int i = 0; i < img.rows; i++) {
		if (y_count % 2 == 0)
			Asym_offset = space / 2;
		else
			Asym_offset = 0;
		for (int j = 0; j < img.cols; j++) {
			// 绘制边框
			if (i < thicknum || i >= thicknum + height || j < thicknum || j >= thicknum + width) {
				img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
				continue;
			}
			// 绘制圆点
			if (cir_y >= img.rows - y_st - thicknum) {
				continue;
			}
			if (i == cir_y && j == cir_x) {
				// 绘制圆点，LINE_AA得到的边缘最为光滑
				circle(img, Point(j, i), radius, cv::Scalar(0, 0, 0, 255), -1, LINE_AA);
				cir_x += space;
			}
			if (cir_x >= img.cols - x_st - thicknum) {
				cir_x = Asym_offset + x_st + radius + thicknum;
				cir_y += space/2;
				y_count++;
			}

		}
		
	}

	//int origin_offset_x = 0;
	//int origin_offset_y = 0;
	//int inner_pattern_height = 6;
	//int inner_pattern_width = 10;
	//// 绘制标记点
	//cir_x = x_st + radius + thicknum + origin_offset_x * space;
	//cir_y = y_st + radius + thicknum + (sqYnum - origin_offset_y - inner_pattern_height) * space;
	//circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);
	//cir_y += (inner_pattern_height - 1) * space;
	//circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);
	//cir_x += (inner_pattern_width - 1) * space;
	//circle(img, Point(cir_x, cir_y), radius / 2, cv::Scalar(255, 255, 255, 255), -1, LINE_AA);


	imwrite("asymmetric_dot_calib.png", img);
	imshow("非对称圆点标定图案", img);
	waitKey(0);
	return 0 ;
}