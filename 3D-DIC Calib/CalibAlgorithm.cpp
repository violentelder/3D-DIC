#include "CalibAlgorithm.h"

// 读取标定图像列表
bool calib_algorithm::readStringList()
{
	imagelist.resize(0);
	FileStorage fs(image_folder + "/stereo_calib.xml", FileStorage::READ);
	if (!fs.isOpened())
		return false;
	FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != FileNode::SEQ)
		return false;
	FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		imagelist.push_back((string)*it);
	return true;
}

// 生成标定图像
bool calib_algorithm::MakeCailbPattern(CalibPattern &Calib_P)
{
	if (Calib_P.PatternType > 2) {
		cout << "不支持的图案格式！" << endl;
		return false;
	}
	if (Calib_P.sqYnum == 0) {
		Calib_P.sqYnum = Calib_P.sqXnum;
	}
	Mat img(Calib_P.bkgHeight + Calib_P.thickNum * 2, Calib_P.bkgWidth + Calib_P.thickNum * 2, CV_8UC4, Scalar(255, 255, 255, 255));
	switch (Calib_P.PatternType)
	{
	// 生成棋盘格图案
	case CALIB_PATTERN_CHECKERBOARD:
	{
		int checkboardX = 0;//棋盘x坐标
		int checkboardY = 0;//棋盘y坐标
		int xLen = Calib_P.bkgWidth / (Calib_P.sqXnum + 1);//x方格长度
		int yLen = Calib_P.bkgHeight / (Calib_P.sqYnum + 1);//y方格长度
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				// 绘制边框
				if (i < Calib_P.thickNum || i >= Calib_P.thickNum + Calib_P.bkgHeight || j < Calib_P.thickNum || j >= Calib_P.thickNum + Calib_P.bkgWidth) {
					img.at<Vec<uchar, 4>>(i, j) = Scalar(255, 255, 255, 255);
					continue;
				}
				// 绘制方格
				checkboardX = j - Calib_P.thickNum;
				checkboardY = i - Calib_P.thickNum;
				if (checkboardY / yLen % 2 == 0) {
					if ((checkboardX) / xLen % 2 == 0) {
						img.at<Vec<uchar, 4>>(i, j) = Scalar(255, 255, 255, 255);
					}
					else {
						img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
					}
				}
				else {
					if ((checkboardX) / xLen % 2 != 0) {
						img.at<Vec<uchar, 4>>(i, j) = Scalar(255, 255, 255, 255);
					}
					else {
						img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
					}
				}
			}
		}
		break;
	}
	// 生成对称无标记圆点图案
	case CALIB_PATTERN_CIRCULAR:
	{
		// 根据输入的特征点数目，自适应计算圆点半径大小（此处预设两圆点圆心之间距离为4*半径）
		int radius = min(Calib_P.bkgWidth / (4 * Calib_P.sqXnum + 2), Calib_P.bkgHeight / (4 * Calib_P.sqYnum + 2));
		if (radius <= 0.01 * min(Calib_P.bkgHeight, Calib_P.bkgWidth)) {
			cout << "警告：圆点过小，可能无法识别！" << endl;
		}
		int space = 4 * radius;
		// 生成两个维度方向上的边缘空白
		int x_st = (Calib_P.bkgWidth - 2 * radius * (2 * Calib_P.sqXnum - 1)) / 2;
		int y_st = (Calib_P.bkgHeight - 2 * radius * (2 * Calib_P.sqYnum - 1)) / 2;

		// 生成起始点圆心坐标
		int cir_x = x_st + radius + Calib_P.thickNum;
		int cir_y = y_st + radius + Calib_P.thickNum;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				// 绘制边框
				if (i < Calib_P.thickNum || i >= Calib_P.thickNum + Calib_P.bkgHeight || j < Calib_P.thickNum || j >= Calib_P.thickNum + Calib_P.bkgWidth) {
					img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
					continue;
				}
				// 绘制圆点
				if (cir_y >= img.rows - y_st - Calib_P.thickNum) {
					continue;
				}
				if (i == cir_y && j == cir_x) {
					// 绘制圆点，LINE_AA得到的边缘最为光滑
					circle(img, Point(j, i), radius, Scalar(0, 0, 0, 255), -1, LINE_AA);
					cir_x += space;
				}
				if (cir_x >= img.cols - x_st - Calib_P.thickNum) {
					cir_x = x_st + radius + Calib_P.thickNum;
					cir_y += space;
				}

			}
		}
		break;
	}
	// 生成对称有标记圆点图案
	case CALIB_PATTERN_SPECIAL:
	{
		// 根据输入的特征点数目，自适应计算圆点半径大小（此处预设两圆点圆心之间距离为4*半径）
		int radius = min(Calib_P.bkgWidth / (4 * Calib_P.sqXnum + 2), Calib_P.bkgHeight / (4 * Calib_P.sqYnum + 2));
		if (radius <= 0.01 * min(Calib_P.bkgHeight, Calib_P.bkgWidth)) {
			cout << "警告：圆点过小，可能无法识别！" << endl;
		}
		int space = 4 * radius;
		// 生成两个维度方向上的边缘空白
		int x_st = (Calib_P.bkgWidth - 2 * radius * (2 * Calib_P.sqXnum - 1)) / 2;
		int y_st = (Calib_P.bkgHeight - 2 * radius * (2 * Calib_P.sqYnum - 1)) / 2;

		// 生成起始点圆心坐标
		int cir_x = x_st + radius + Calib_P.thickNum;
		int cir_y = y_st + radius + Calib_P.thickNum;
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				// 绘制边框
				if (i < Calib_P.thickNum || i >= Calib_P.thickNum + Calib_P.bkgHeight || j < Calib_P.thickNum || j >= Calib_P.thickNum + Calib_P.bkgWidth) {
					img.at<Vec<uchar, 4>>(i, j) = Scalar(0, 0, 0, 255);
					continue;
				}
				// 绘制圆点
				if (cir_y >= img.rows - y_st - Calib_P.thickNum) {
					continue;
				}
				if (i == cir_y && j == cir_x) {
					// 绘制圆点，LINE_AA得到的边缘最为光滑
					circle(img, Point(j, i), radius, Scalar(0, 0, 0, 255), -1, LINE_AA);
					cir_x += space;
				}
				if (cir_x >= img.cols - x_st - Calib_P.thickNum) {
					cir_x = x_st + radius + Calib_P.thickNum;
					cir_y += space;
				}

			}
		}
		// 绘制标记点
		cir_x = x_st + radius + Calib_P.thickNum + Calib_P.origin_offset_x * space;
		cir_y = y_st + radius + Calib_P.thickNum + (Calib_P.sqYnum - Calib_P.origin_offset_y - Calib_P.inner_pattern_height) * space;
		circle(img, Point(cir_x, cir_y), radius / 2, Scalar(255, 255, 255, 255), -1, LINE_AA);
		cir_y += (Calib_P.inner_pattern_height - 1) * space;
		circle(img, Point(cir_x, cir_y), radius / 2, Scalar(255, 255, 255, 255), -1, LINE_AA);
		cir_x += (Calib_P.inner_pattern_width - 1) * space;
		circle(img, Point(cir_x, cir_y), radius / 2, Scalar(255, 255, 255, 255), -1, LINE_AA);

		break;
	}
	}
	if (!imwrite(Calib_P.savePath, img)) {
		cout << "标定图像保存失败，请检查格式是否正确" << endl;
		return false;
	}
	printf("生成成功 标定图像大小为 %d * %d \n", Calib_P.bkgWidth + Calib_P.thickNum, Calib_P.bkgHeight + Calib_P.thickNum);
	return true;
}

// 计算重投影误差
double calib_algorithm::computeReprojectionErrors(
	const vector<vector<Point3f> >& objectPoints,
	const vector<vector<Point2f> >& imagePoints,
	const vector<Mat>& rvecs, const vector<Mat>& tvecs,
	const Mat& cameraMatrix, const Mat& distCoeffs,
	vector<float>& perViewErrors)
{
	vector<Point2f> imagePoints2;
	int i, totalPoints = 0;
	double totalErr = 0, err;
	perViewErrors.resize(objectPoints.size());

	for (i = 0; i < (int)objectPoints.size(); i++)
	{
		projectPoints(Mat(objectPoints[i]), rvecs[i], tvecs[i],
			cameraMatrix, distCoeffs, imagePoints2);
		err = norm(Mat(imagePoints[i]), Mat(imagePoints2), NORM_L2);
		int n = (int)objectPoints[i].size();
		perViewErrors[i] = (float)sqrt(err*err / n);
		totalErr += err*err;
		totalPoints += n;
	}

	return sqrt(totalErr / totalPoints);
}



// 张正友标定算法
void calib_algorithm::StereoCalib()
{
	nimages = (int)goodImageList.size() / 2;
	if (nimages < 2) {
		destroyAllWindows();
		cout << "错误: 提取到特征点的图片对数过少不足以完成标定\n";
		return;
	}
	cout << "Running stereo calibration ...\n";
	//// 调试代码
	//cout << "物点情况：" << endl;
	//for (int i = 0; i < objectPoints.size(); i++) {
	//	cout << endl << i << endl;
	//	for (auto object : objectPoints[i])
	//		cout << '[' << object.x << ',' << object.y << ']';
	//}
	//cout << endl << endl;


	//cout << "像点情况（cam1）：" << endl;
	//for (int i = 0; i < imagePoints[0].size(); i++) {
	//	cout << endl << i << endl;
	//	for (auto point : imagePoints[0][i])
	//		cout << '[' << point.x << ',' << point.y << ']';
	//}
	//cout << endl << endl;


	//cout << "像点情况（cam2）：" << endl;
	//for (int i = 0; i < imagePoints[1].size(); i++) {
	//	cout << endl << i << endl;
	//	for (auto point : imagePoints[1][i])
	//		cout << '[' << point.x << ',' << point.y << ']';
	//}
	//cout << endl << endl;


	clock_t start_t = clock();
	cameraMatrix[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
	cameraMatrix[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);
	double rms = stereoCalibrate(objectPoints, imagePoints[0], imagePoints[1],
		cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, E, F, perViewErrors,
		cailb_option,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 30, 1e-5));
	clock_t end_t = clock();
	endtime += (double)(end_t - start_t) / CLOCKS_PER_SEC;
	cout << "本次标定耗时：" << endtime * 1000 << "ms" << endl;
	if (rms < 0.0) {
		cout << "\n*** 错误, 标定失败" << endl;
		return;
	}
	cout << "标定完成!\n";
	cout << "RMS error=" << rms << endl;
	if (rms > 1.0) {
		cout << "RMS过大，建议重新标定，当前RMS：" << rms << "(RMS应当在0.5以下)" << endl;
	}

	// 保存各对图像的均方根重投影误差
	Scalar REmean;  //均值
	Scalar REstddev;  //标准差
	meanStdDev(perViewErrors, REmean, REstddev);
	FileStorage RMS_perViewErrors(cal_debug_folder + "/rms reprojection error.xml", FileStorage::WRITE);
	if (RMS_perViewErrors.isOpened())
	{
		RMS_perViewErrors << "perViewErrors" << perViewErrors << "Mean" << (float)REmean.val[0] << "Std" << (float)REstddev.val[0];
		RMS_perViewErrors.release();
	}
	else
		cout << "错误: 无法保存均方根重投影误差\n";



	// CALIBRATION QUALITY CHECK
	// because the output fundamental matrix implicitly
	// includes all the output information,
	// we can check the quality of calibration using the
	// epipolar geometry constraint: m2^t*F*m1=0
	// 设置对极误差结果存储文件
	fstream epipolar_file(cal_debug_folder + "/cal_errors.txt", ios_base::out);
	// 计算对极误差
	double err = 0;
	int npoints = 0;
	vector<Vec3f> lines[2];
	for (int i = 0; i < nimages; i++)
	{
		int npt = (int)imagePoints[0][i].size();
		Mat imgpt[2];
		for (int k = 0; k < 2; k++)
		{
			imgpt[k] = Mat(imagePoints[k][i]);
			undistortPoints(imgpt[k], imgpt[k], cameraMatrix[k], distCoeffs[k], Mat(), cameraMatrix[k]);
			computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]);
		}
		double imgErr = 0.0;
		for (int j = 0; j < npt; j++)
		{
			double errij = fabs(imagePoints[0][i][j].x*lines[1][j][0] +
				imagePoints[0][i][j].y*lines[1][j][1] + lines[1][j][2]) +
				fabs(imagePoints[1][i][j].x*lines[0][j][0] +
					imagePoints[1][i][j].y*lines[0][j][1] + lines[0][j][2]);
			err += errij;
			imgErr += errij;
		}
		double epipolar = imgErr / npt;
		epipolar_file << epipolar << endl;
		npoints += npt;
	}
	epipolar_file.close();
	cout << "average epipolar err = " << err / npoints << endl;

	// 保存内参
	FileStorage fs(cal_debug_folder + "/intrinsics.xml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "M1" << cameraMatrix[0] << "D1" << distCoeffs[0] <<
			"M2" << cameraMatrix[1] << "D2" << distCoeffs[1] << 
			"RMS" << rms << "AverageEpipolar" << err / npoints;
		fs.release();
	}
	else
		cout << "错误: 无法保存相机内部参数\n";

	// 保存外参
	Rect validRoi[2];

	stereoRectify(cameraMatrix[0], distCoeffs[0],
		cameraMatrix[1], distCoeffs[1],
		imageSize, R, T, R1, R2, P1, P2, Q,
		CALIB_ZERO_DISPARITY, 1, imageSize, &validRoi[0], &validRoi[1]);

	fs.open(cal_debug_folder + "/extrinsics.xml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "R" << R << "T" << T << "R1" << R1 << "R2" << R2 << 
			"P1" << P1 << "P2" << P2 << "Q" << Q;
		fs.release();
	}
	else
		cout << "错误: 无法保存外部参数\n";
	cout << "Stereo calibration end" << endl;


	// 判断是否是竖直双目
	bool isVerticalStereo = fabs(P2.at<double>(1, 3)) > fabs(P2.at<double>(0, 3));

	// 计算并显示校正结果
	if (!showRectified)
		return;

	Mat rmap[2][2];
	// 已标定校正(BOUGUET'S METHOD)
	if (useCalibrated)
	{
		// we already computed everything

	}
	// 未标定校正(HARTLEY'S METHOD)
	else
		// use intrinsic parameters of each camera, but
		// compute the rectification transformation directly
		// from the fundamental matrix
	{
		vector<Point2f> allimgpt[2];
		for (int k = 0; k < 2; k++)
		{
			for (int i = 0; i < nimages; i++)
				copy(imagePoints[k][i].begin(), imagePoints[k][i].end(), back_inserter(allimgpt[k]));
		}
		F = findFundamentalMat(Mat(allimgpt[0]), Mat(allimgpt[1]), FM_8POINT, 0, 0);
		Mat H1, H2;
		stereoRectifyUncalibrated(Mat(allimgpt[0]), Mat(allimgpt[1]), F, imageSize, H1, H2, 3);

		R1 = cameraMatrix[0].inv()*H1*cameraMatrix[0];
		R2 = cameraMatrix[1].inv()*H2*cameraMatrix[1];
		P1 = cameraMatrix[0];
		P2 = cameraMatrix[1];
	}

	// 计算校正映射，进行畸变校正
	initUndistortRectifyMap(cameraMatrix[0], distCoeffs[0], Mat(), getOptimalNewCameraMatrix(cameraMatrix[0], distCoeffs[0], 
		imageSize, 1, imageSize, 0), imageSize, CV_16SC2, rmap[0][0], rmap[0][1]);
	initUndistortRectifyMap(cameraMatrix[1], distCoeffs[1], Mat(), getOptimalNewCameraMatrix(cameraMatrix[1], distCoeffs[1],
		imageSize, 1, imageSize, 0), imageSize, CV_16SC2, rmap[1][0], rmap[1][1]);

	Mat canvas;
	double sf;
	int w, h;
	if (!isVerticalStereo)
	{
		sf = 600. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		canvas.create(h, w * 2, CV_8UC3);
	}
	else
	{
		sf = 300. / MAX(imageSize.width, imageSize.height);
		w = cvRound(imageSize.width*sf);
		h = cvRound(imageSize.height*sf);
		canvas.create(h * 2, w, CV_8UC3);
	}

	for (int i = 0; i < nimages; i++)
	{
		for (int k = 0; k < 2; k++)
		{
			Mat img = imread(image_folder + "/" + goodImageList[i * 2 + k], 0), rimg, cimg;
			remap(img, rimg, rmap[k][0], rmap[k][1], INTER_LINEAR);
			waitKey(100);
			bool writed = imwrite(cal_debug_folder + "/undistort/" + goodImageList[i * 2 + k].substr(0, goodImageList[i * 2 + k].size() - 4) + "_undistorted.jpg", rimg);
			if (!writed) {
				cout << goodImageList[i * 2 + k] << "畸变校正图像存储失败\n";
				destroyAllWindows();
				return;
			}
			cvtColor(rimg, cimg, COLOR_GRAY2BGR);
			Mat canvasPart = !isVerticalStereo ? canvas(Rect(w*k, 0, w, h)) : canvas(Rect(0, h*k, w, h));
			resize(cimg, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
			if (useCalibrated)
			{
				Rect vroi(cvRound(validRoi[k].x*sf), cvRound(validRoi[k].y*sf),
					cvRound(validRoi[k].width*sf), cvRound(validRoi[k].height*sf));
				rectangle(canvasPart, vroi, Scalar(0, 0, 255), 3, 8);
			}
		}

		if (!isVerticalStereo)
			for (int j = 0; j < canvas.rows; j += 16)
				line(canvas, Point(0, j), Point(canvas.cols, j), Scalar(0, 255, 0), 1, 8);
		else
			for (int j = 0; j < canvas.cols; j += 16)
				line(canvas, Point(j, 0), Point(j, canvas.rows), Scalar(0, 255, 0), 1, 8);
		imshow("rectified", canvas);
		char c = (char)waitKey(100);
		if (c == 27 || c == 'q' || c == 'Q')
			break;
	}
	destroyAllWindows();
	return;
}

// 棋盘格角点提取
void calib_checkerboard::ExtractTarget()
{
	imagePoints[0].clear();
	imagePoints[1].clear();
	objectPoints.clear();
	endtime = 0;

	if (imagelist.size() % 2 != 0)
	{
		cout << "Error: 图片列表中数目为奇数，请重新查验\n";
		return;
	}


	// 创建文件目录
	if (draw_intersection)
	{
		create_directory(cal_debug_folder + "/corners");
	}
	create_directory(cal_debug_folder + "/undistort/");

	clock_t start_t = clock();
	nimages = (int)imagelist.size() / 2;
	int i, j, k = (int)imagelist.size() / 2;

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);

	for (i = j = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			const string& filename = imagelist[i * 2 + k];
			Mat img = imread(image_folder + '/' + filename, 0);
			if (img.empty())
				break;
			if (imageSize == Size())
				imageSize = img.size();
			else if (img.size() != imageSize)
			{
				cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
				break;
			}
			bool found = false;
			// 声明引用，对corners的修改就是对imagePoints的修改
			vector<Point2f>& corners = imagePoints[k][j];
			for (int scale = 1; scale <= maxScale; scale++)
			{
				Mat timg;
				if (scale == 1)
					timg = img;
				else
					resize(img, timg, Size(), scale, scale, INTER_LINEAR_EXACT);
				found = findChessboardCorners(timg, boardSize, corners,
					CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_NORMALIZE_IMAGE);
				if (found)
				{
					if (scale > 1)
					{
						Mat cornersMat(corners);
						cornersMat *= 1. / scale;
					}
					break;
				}
			}
			if (draw_intersection)
			{
				cout << filename << endl;
				cvtColor(img, cimg, COLOR_GRAY2BGR);
				drawChessboardCorners(cimg, boardSize, corners, found);
				double sf = 640. / MAX(img.rows, img.cols);
				resize(cimg, cimg1, Size(), sf, sf, INTER_LINEAR_EXACT);
				imshow("corners", cimg1);
				bool writed = imwrite(cal_debug_folder + "/corners/" + filename.substr(0, filename.size() - 4) + "_corners.jpg", cimg1);
				if (!writed) {
					cout << filename << "角点检测图像存储失败\n";
				}
				char c = (char)waitKey(100);
				if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
					exit(-1);
			}
			else
				putchar('.');
			if (!found)
				break;
			// 亚像素角点检测优化
			cornerSubPix(img, corners, Size(11, 11), Size(-1, -1),
				TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
					30, 0.01));
		}
		if (k == 2)
		{
			goodImageList.push_back(imagelist[i * 2]);
			goodImageList.push_back(imagelist[i * 2 + 1]);
			j++;
		}
	}
	cout << j << "对图像成功被检测到！\n";
	nimages = j;
	if (nimages < 2)
	{
		cout << "错误: 图片对数过少不足以完成标定\n";
		return;
	}

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);

	for (i = 0; i < nimages; i++)
	{
		for (j = 0; j < boardSize.height; j++)
			for (k = 0; k < boardSize.width; k++)
				objectPoints[i].push_back(Point3f(k*cal_target_spacing_size, j*cal_target_spacing_size, 0));
	}
	clock_t end_t = clock();
	endtime += (double)(end_t - start_t) / CLOCKS_PER_SEC;
	return;
}

// 对称圆点提取
void calib_circular::ExtractTarget()
{
	imagePoints[0].clear();
	imagePoints[1].clear();
	objectPoints.clear();
	
	endtime = 0;


	if (imagelist.size() % 2 != 0)
	{
		cout << "Error: 图片列表中数目为奇数，请重新查验\n";
		return;
	}


	// 创建文件目录
	if (draw_intersection)
	{
		create_directory(cal_debug_folder + "/corners");
	}
	create_directory(cal_debug_folder + "/undistort/");

	clock_t start_t = clock();
	nimages = (int)imagelist.size() / 2;
	int i, j, k = (int)imagelist.size() / 2;

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	


	SimpleBlobDetector::Params params;
	params.filterByArea = true;
	params.maxArea = 10e4;
	params.minArea = 100;
	Ptr<FeatureDetector> blobDetector = SimpleBlobDetector::create(params);

	for (i = j = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			const string& filename = imagelist[i * 2 + k];
			Mat img = imread(image_folder + '/' + filename, 0);
			if (img.empty())
				break;
			if (imageSize == Size())
				imageSize = img.size();
			else if (img.size() != imageSize)
			{
				cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
				break;
			}
			bool found = false;
			// 声明引用，对corners的修改就是对imagePoints的修改
			vector<Point2f>& centers = imagePoints[k][j];
			for (int scale = 1; scale <= maxScale; scale++)
			{
				Mat timg;
				if (scale == 1)
					timg = img;
				else
					resize(img, timg, Size(), scale, scale, INTER_LINEAR_EXACT);
				found = findCirclesGrid(timg, patternSize, centers, CALIB_CB_SYMMETRIC_GRID | CALIB_CB_CLUSTERING, blobDetector);
				if (found)
				{
					if (scale > 1)
					{
						Mat cornersMat(centers);
						cornersMat *= 1. / scale;
					}
					break;
				}
			}
			if (draw_intersection)
			{
				cout << filename << endl;
				cvtColor(img, cimg, COLOR_GRAY2BGR);
				drawChessboardCorners(cimg, patternSize, centers, found);
				double sf = 640. / MAX(img.rows, img.cols);
				resize(cimg, cimg1, Size(), sf, sf, INTER_LINEAR_EXACT);
				imshow("corners", cimg1);
				bool writed = imwrite(cal_debug_folder + "/corners/" + filename.substr(0, filename.size() - 4) + "_circulars.jpg", cimg);
				if (!writed) {
					cout << filename << "圆点检测图像存储失败\n";
				}
				char c = (char)waitKey(100);
				if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
					exit(-1);
			}
			else
				putchar('.');
			if (!found)
				break;
			// 亚像素角点检测优化
			cornerSubPix(img, centers, Size(11, 11), Size(-1, -1),
				TermCriteria(TermCriteria::COUNT + TermCriteria::EPS,
					30, 0.01));
		}
		if (k == 2)
		{
			goodImageList.push_back(imagelist[i * 2]);
			goodImageList.push_back(imagelist[i * 2 + 1]);
			j++;
		}
	}
	cout << j << "对图像成功被检测到！\n";
	nimages = j;
	if (nimages < 2)
	{
		cout << "错误: 图片对数过少不足以完成标定\n";
		return;
	}

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);

	for (i = 0; i < nimages; i++)
	{
		for (j = 0; j < patternSize.height; j++)
			for (k = 0; k < patternSize.width; k++)
				objectPoints[i].push_back(Point3f(k*cal_target_spacing_size, j*cal_target_spacing_size, 0));
	}
	clock_t end_t = clock();
	endtime += (double)(end_t - start_t) / CLOCKS_PER_SEC;
	return;
}


// 获取特殊标记点的像素坐标
void calib_circular_new::get_dot_markers(IN Mat img,
	OUT vector<KeyPoint> & keypoints,
	IN int thresh,
	IN bool invert,
	IN const int min_size) {

	// 设置提取算法参数
	int block_size = block_size_default; // The old method had default set to 75
	if (block_size % 2 == 0) block_size++; // block size has to be odd
	int threshold_mode = 0;

	// 设置用于检测的BLOB
	SimpleBlobDetector::Params params;
	params.filterByArea = true;
	params.maxArea = 10e4;
	params.minArea = min_size;
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);

	// 清除关键点结果的缓存
	keypoints.clear();

	// 创建图像副本
	Mat timg;
	img.copyTo(timg);

	// 设置一个空的位图
	Mat bi_src(timg.size(), timg.type());

	// 二值化
	if (use_adaptive) {
		adaptiveThreshold(img, img, 255, filter_mode, threshold_mode, block_size, thresh);
		img.copyTo(bi_src);
	}
	else {
		threshold(timg, bi_src, thresh, 255, threshold_mode);
	}

	// 创建反色图像
	Mat not_src(bi_src.size(), bi_src.type());
	bitwise_not(bi_src, not_src);

	// detect dots on the appropriately inverted image
	Mat labelImage(img.size(), CV_32S);
	Mat stats, centroids;
	int nLabels = 0;

	// 在正确的背景图像上搜寻特征点
	if (invert) {
		detector->detect(not_src, keypoints);
		// 联通区域分析，返回值：区域数目
		nLabels = connectedComponentsWithStats(bi_src, labelImage, stats, centroids, 8, CV_32S);
	}
	else {
		detector->detect(bi_src, keypoints);
		nLabels = connectedComponentsWithStats(not_src, labelImage, stats, centroids, 8, CV_32S);
	}
	// 没有搜寻到特征点
	if (keypoints.size() == 0) return;

	// 当关键点数目为3，但是联通区域大于等于3时需要进行过滤
	if (keypoints.size() == 3 && nLabels >= 3) {
		float avg_diameter = 0.0f;
		for (size_t i = 0; i<keypoints.size(); ++i) {
			int x = keypoints[i].pt.x;
			int y = keypoints[i].pt.y;
			int label = labelImage.at<int>(y, x);
			int psize = stats.at<int>(label, CC_STAT_AREA);
			DEBUG_MSG("new size " << psize);
			keypoints[i].size = psize;
			DEBUG_MSG("Keypoint: " << i << " updated size " << keypoints[i].size);
			avg_diameter += keypoints[i].size;
		}
		avg_diameter /= keypoints.size();
		DEBUG_MSG("get_dot_markers(): avg keypoint diameter " << avg_diameter);
		size_t i = keypoints.size();
		while (i--) {
			// remove the keypoint from the vector
			if (keypoints[i].size <= 0.0) {
				keypoints.erase(keypoints.begin() + i);
			}
			if (abs(keypoints[i].size - avg_diameter) / avg_diameter>0.30) {
				keypoints.erase(keypoints.begin() + i);
			}
		}
		DEBUG_MSG("get_dot_markers(): num keypoints " << keypoints.size());
	}
}

// 将关键点重新排序为原点、x、y 顺序
void calib_circular_new::reorder_keypoints(vector<KeyPoint> & keypoints) {
	vector<float> dist(3, 0.0); //holds the distances between the points
	vector<KeyPoint> temp_points;
	vector<int> dist_order(3, 0); //index order of the distances max to min
	float cross; //cross product and indicies
	temp_points.clear();
	//save the distances between the points (note if dist(1,2) is max point 0 is the origin)
	dist[0] = dist2(keypoints[1], keypoints[2]);
	dist[1] = dist2(keypoints[0], keypoints[2]);
	dist[2] = dist2(keypoints[0], keypoints[1]);
	//将三点之间的距离进行排序
	order_dist3(dist, dist_order);

	// 计算叉积以确定 x 和 y 轴
	int io = dist_order[0];
	int i1 = dist_order[1];
	int i2 = dist_order[2];

	//由于图像是倒置，所以叉乘为正是i1对应的值为Y轴， i2对应的值为X轴
	cross = ((keypoints[i1].pt.x - keypoints[io].pt.x) * (keypoints[i2].pt.y - keypoints[io].pt.y)) -
		((keypoints[i1].pt.y - keypoints[io].pt.y) * (keypoints[i2].pt.x - keypoints[io].pt.x));
	if (cross > 0.0) { //i2 is the x axis
		i2 = dist_order[1];
		i1 = dist_order[2];
	}
	//reorder the points and return
	temp_points.push_back(keypoints[io]);
	temp_points.push_back(keypoints[i1]);
	temp_points.push_back(keypoints[i2]);
	keypoints[0] = temp_points[0];
	keypoints[1] = temp_points[1];
	keypoints[2] = temp_points[2];
}

// 计算转换系数
void calib_circular_new::calc_trans_coeff(IN vector<KeyPoint> & imgpoints,
	IN vector<KeyPoint> & grdpoints,
	OUT vector<float> & img_to_grdx,
	OUT vector<float> & img_to_grdy,
	OUT vector<float> & grd_to_imgx,
	OUT vector<float> & grd_to_imgy) {

	// 常规算法，只有三个关键点时，此时六个参数不会包含梯形形变
	if (imgpoints.size() == 3) {
		Mat A = Mat_<double>(3, 3);
		Mat Ax = Mat_<double>(3, 1);
		Mat Ay = Mat_<double>(3, 1);
		Mat coeff_x = Mat_<double>(3, 1);
		Mat coeff_y = Mat_<double>(3, 1);

		// 图像到网格的变换矩阵 (此时未超定，直接求解线性方程组即可)
		for (int i = 0; i < 3; i++) {
			A.at<double>(i, 0) = 1.0;
			A.at<double>(i, 1) = imgpoints[i].pt.x;
			A.at<double>(i, 2) = imgpoints[i].pt.y;
			Ax.at<double>(i, 0) = grdpoints[i].pt.x;
			Ay.at<double>(i, 0) = grdpoints[i].pt.y;
		}
		//求解系数向量
		coeff_x = A.inv()*Ax;
		coeff_y = A.inv()*Ay;
		//copy over the coefficients
		for (int i_coeff = 0; i_coeff < 3; i_coeff++) {
			img_to_grdx[i_coeff] = coeff_x.at<double>(i_coeff, 0);
			img_to_grdy[i_coeff] = coeff_y.at<double>(i_coeff, 0);
		}
		//高阶系数向量输为0
		img_to_grdx[3] = 0.0;
		img_to_grdy[3] = 0.0;
		img_to_grdx[4] = 0.0;
		img_to_grdy[4] = 0.0;
		img_to_grdx[5] = 0.0;
		img_to_grdy[5] = 0.0;

		//网格到图像的变换 (此时未超定，直接求解线性方程组即可)
		for (int i = 0; i < 3; i++) {
			A.at<double>(i, 0) = 1.0;
			A.at<double>(i, 1) = grdpoints[i].pt.x;
			A.at<double>(i, 2) = grdpoints[i].pt.y;
			Ax.at<double>(i, 0) = imgpoints[i].pt.x;
			Ay.at<double>(i, 0) = imgpoints[i].pt.y;
		}
		//求解系数
		coeff_x = A.inv()*Ax;
		coeff_y = A.inv()*Ay;
		//copy over the coefficients
		for (int i_coeff = 0; i_coeff < 3; i_coeff++) {
			grd_to_imgx[i_coeff] = coeff_x.at<double>(i_coeff, 0);
			grd_to_imgy[i_coeff] = coeff_y.at<double>(i_coeff, 0);
		}
		//高阶系数向量设置为0
		grd_to_imgx[3] = 0.0;
		grd_to_imgy[3] = 0.0;
		grd_to_imgx[4] = 0.0;
		grd_to_imgy[4] = 0.0;
		grd_to_imgx[5] = 0.0;
		grd_to_imgy[5] = 0.0;
	}

	//如果提供了三个以上的点，则使用 12 个参数来反映梯形失真
	if (imgpoints.size() > 3) {

		Mat A = Mat_<double>(imgpoints.size(), 6);
		Mat AtA = Mat_<double>(6, 6);
		Mat bx = Mat_<double>(imgpoints.size(), 1);
		Mat by = Mat_<double>(imgpoints.size(), 1);
		Mat Atb = Mat_<double>(6, 1);
		Mat coeff_x = Mat_<double>(6, 1);
		Mat coeff_y = Mat_<double>(6, 1);

		// 图像到网格变换（此时超定采用最小线性二乘拟合法）
		for (int i = 0; i < (int)imgpoints.size(); i++) {
			A.at<double>(i, 0) = 1.0;
			A.at<double>(i, 1) = imgpoints[i].pt.x;
			A.at<double>(i, 2) = imgpoints[i].pt.y;
			A.at<double>(i, 3) = imgpoints[i].pt.x * imgpoints[i].pt.y;
			A.at<double>(i, 4) = imgpoints[i].pt.x * imgpoints[i].pt.x;
			A.at<double>(i, 5) = imgpoints[i].pt.y * imgpoints[i].pt.y;
			bx.at<double>(i, 0) = grdpoints[i].pt.x;
			by.at<double>(i, 0) = grdpoints[i].pt.y;
		}
		//求解系数
		AtA = A.t()*A;
		Atb = A.t()*bx;
		coeff_x = AtA.inv()*Atb;
		Atb = A.t()*by;
		coeff_y = AtA.inv()*Atb;

		//copy over the coefficients
		for (int i_coeff = 0; i_coeff < 6; i_coeff++) {
			img_to_grdx[i_coeff] = coeff_x.at<double>(i_coeff, 0);
			img_to_grdy[i_coeff] = coeff_y.at<double>(i_coeff, 0);
		}

		// 网格到图像的变换
		for (int i = 0; i < (int)imgpoints.size(); i++) {
			A.at<double>(i, 0) = 1.0;
			A.at<double>(i, 1) = grdpoints[i].pt.x;
			A.at<double>(i, 2) = grdpoints[i].pt.y;
			A.at<double>(i, 3) = grdpoints[i].pt.x * grdpoints[i].pt.y;
			A.at<double>(i, 4) = grdpoints[i].pt.x * grdpoints[i].pt.x;
			A.at<double>(i, 5) = grdpoints[i].pt.y * grdpoints[i].pt.y;
			bx.at<double>(i, 0) = imgpoints[i].pt.x;
			by.at<double>(i, 0) = imgpoints[i].pt.y;
		}
		//solve for the coefficients
		AtA = A.t()*A;
		Atb = A.t()*bx;
		coeff_x = AtA.inv()*Atb;
		Atb = A.t()*by;
		coeff_y = AtA.inv()*Atb;

		//copy over the coefficients
		for (int i_coeff = 0; i_coeff < 6; i_coeff++) {
			grd_to_imgx[i_coeff] = coeff_x.at<double>(i_coeff, 0);
			grd_to_imgy[i_coeff] = coeff_y.at<double>(i_coeff, 0);
		}
	}
}

// 根据平均大小和点是否落在中心框内过滤数据
void calib_circular_new::filter_dot_markers(vector<KeyPoint> dots,
	vector<KeyPoint> & img_points,
	vector<KeyPoint> & grd_points,
	const vector<float> & grd_to_imgx,
	const vector<float> & grd_to_imgy,
	const vector<float> & img_to_grdx,
	const vector<float> & img_to_grdy,
	float dot_tol,
	Mat img,
	bool draw) 
{
	// 设置边界框参数
	vector<float> box_x(5, 0.0);
	vector<float> box_y(5, 0.0);

	// 创建用于返回网格坐标的对象
	float grid_x, grid_y;
	long grid_ix, grid_iy;
	//single point and keypoint for drawing and storage
	Point cvpoint;
	KeyPoint cvkeypoint;

	// 清除之前关键点的网格坐标和图像坐标
	grd_points.clear();
	img_points.clear();

	// 创建特征点的边界框
	create_bounding_box(box_x, box_y, num_fiducials_x, num_fiducials_y,
		grd_to_imgx, grd_to_imgy, img.size().width, img.size().height);

	if (draw) {
		vector<Point> contour;
		for (size_t n = 0; n < box_x.size(); ++n) {
			contour.push_back(Point(box_x[n], box_y[n]));
		}
		// 绘制边界框
		polylines(img, contour, true, Scalar(255, 255, 153), 2);
	}
	// 计算平均圆点大小:
	float avg_dot_size = 0.0;
	assert(dots.size() > 0);
	for (size_t i = 0; i < dots.size(); ++i) {
		avg_dot_size += dots[i].size;
	}
	avg_dot_size /= dots.size();

	//过滤圆点
	for (size_t n = 0; n < dots.size(); ++n) {
		//if requested draw all the found points
		if (draw) {
			// 绘制圆需要给的是圆心坐标
			cvpoint.x = dots[n].pt.x;
			cvpoint.y = dots[n].pt.y;
			// 给找到的特征点绘制红色的圆进行标记
			if (img.size().height>800) {
				circle(img, cvpoint, 20, Scalar(0, 0, 255), 2);
			}
			else {
				circle(img, cvpoint, 10, Scalar(0, 0, 255), 2);
			}
		}

		//如果这个点的大小不在可接受的范围
		if (dots[n].size < 0.8f*avg_dot_size || dots[n].size > 1.4f*avg_dot_size) 
			continue;

		//如果这个点位于边界框外
		if (!is_in_quadrilateral(dots[n].pt.x, dots[n].pt.y, box_x, box_y)) 
			continue;


		//从图片位置获取对应的网格位置
		image_to_grid((float)dots[n].pt.x, (float)dots[n].pt.y, grid_x, grid_y, img_to_grdx, img_to_grdy);
		//获取到四舍五入后对应的整形网格坐标
		grid_ix = lround(grid_x);
		grid_iy = lround(grid_y);
		//提取到的特征点坐标是否在预期位置的可接受距离内和所需的网格区域内
		if (abs(grid_x - round(grid_x)) <= dot_tol && abs(grid_y - round(grid_y)) <= dot_tol &&
			grid_ix >= 0 && grid_ix < num_fiducials_x && grid_iy >= 0 && grid_iy < num_fiducials_y) {
			// 将点保存下来
			cvkeypoint.pt.x = grid_ix;
			cvkeypoint.pt.y = grid_iy;
			img_points.push_back(dots[n]);
			grd_points.push_back(cvkeypoint);
			// 在正确的点上绘制绿色的圆
			if (draw) {
				if (img.size().height>800) {
					circle(img, cvpoint, 12, Scalar(0, 255, 0), 4);
				}
				else {
					circle(img, cvpoint, 6, Scalar(0, 255, 0), 2);
				}
			}
		}
	}//end dots loop

	 // 对每个特征点的圆心以及网格坐标进行标注
	if (draw) {
		float imgx, imgy;
		for (float i_x = 0; i_x < num_fiducials_x; i_x++) {
			for (float i_y = 0; i_y < num_fiducials_y; i_y++) {
				grid_to_image(i_x, i_y, imgx, imgy, grd_to_imgx, grd_to_imgy, img.size().width, img.size().height);
				cvpoint.x = imgx;
				cvpoint.y = imgy;
				stringstream dot_text;
				dot_text << "(" << (int)i_x << "," << (int)i_y << ")";
				if (img.size().height>800) {
					putText(img, dot_text.str(), cvpoint + Point(20, 20),
						FONT_HERSHEY_COMPLEX_SMALL, 1.5, Scalar(255, 0, 255), 1, LINE_AA);
					circle(img, cvpoint, 5, Scalar(255, 0, 255), -1);
				}
				else {
					putText(img, dot_text.str(), cvpoint + Point(8, 20),
						FONT_HERSHEY_COMPLEX_SMALL, 0.75, Scalar(255, 0, 255), 1, LINE_AA);
					circle(img, cvpoint, 3, Scalar(255, 0, 255), -1);
				}
			}
		}
	}
}

// 获取其他圆点的像素坐标
int calib_circular_new::get_dot_targets(Mat & img,
	vector<KeyPoint> & key_points,
	vector<KeyPoint> & img_points,
	vector<KeyPoint> & grd_points,
	int & return_thresh) 
{
	key_points.clear();
	img_points.clear();
	grd_points.clear();


	// 建立图片副本
	Mat img_cpy = img.clone();
	
	int block_size = block_size_default;
	// 设置提取算法默认参数
	if (block_size % 2 == 0) block_size++;
	int min_blob_size = min_blob_size_default;

	// 设置标定板相关参数
	const bool invert = patternType == CALIB_PATTERN_SPECIAL;

	// 创建方位点阵列坐标向量（索引）
	vector<KeyPoint> marker_grid_locs;
	marker_grid_locs.resize(3);
	marker_grid_locs[0].pt.x = origin_loc_x;
	marker_grid_locs[0].pt.y = origin_loc_y;
	marker_grid_locs[1].pt.x = origin_loc_x + num_fiducials_origin_to_x_marker - 1;
	marker_grid_locs[1].pt.y = origin_loc_y;
	marker_grid_locs[2].pt.x = origin_loc_x;
	marker_grid_locs[2].pt.y = origin_loc_y + num_fiducials_origin_to_y_marker - 1;

	// 从图片中提取关键点
	bool keypoints_found = true;
	//在不同阈值条件下搜寻关键点
	int i_thresh_first = 0;
	int i_thresh_last = 0;
	int i_thresh = threshold_start;
	if (threshold_start != threshold_end) {
		for (; i_thresh <= threshold_end; i_thresh += threshold_step) {
			get_dot_markers(img_cpy, key_points, i_thresh, invert, min_blob_size);
			// 判断标志点是否被找到
			if (key_points.size() != 3) {
				keypoints_found = false;
				// 关键点在之前已经被找到了
				if (i_thresh_first != 0) {
					keypoints_found = true; 
					break;
				}
			}
			else
			{
				// 保存当前阈值
				if (i_thresh_first == 0) {
					i_thresh_first = i_thresh;
				}
				i_thresh_last = i_thresh;
				keypoints_found = true;
			}
		}
		  // 计算平均阈值
		i_thresh = (i_thresh_first + i_thresh_last) / 2;
	}

	// 在平均阈值再次搜索标志点（双阈值确定最好的）
	get_dot_markers(img_cpy, key_points, i_thresh, invert, min_blob_size);

	// 可能会出现这种灰度阈值下没有标志点的情况
	// 很可能是由于阈值的开始点存在问题
	if (key_points.size() != 3) {
		cout << "*** warning: unable to identify three keypoints, other points will not be extracted" << endl;
		keypoints_found = false;
	}

	Point cvpoint;
	// 对原始图像进行二值化处理
	if (preview_thresh) {
		if (use_adaptive) {
			adaptiveThreshold(img, img, 255, filter_mode, threshold_mode, block_size, i_thresh);
		}
		else {
			// apply thresholding
			threshold(img, img, i_thresh, 255, threshold_mode);
		}
	}
	// 若关键点未被找到，返回错误码1
	if (!keypoints_found) return 1;

	// 获取了关键点坐标以后开始获取其他点的坐标

	// 将方位点根据圆点，X轴，Y轴进行重新排序
	reorder_keypoints(key_points);

	// 打印当前关键点的信息情况
	cout << "get_dot_targets():     using threshold: " << i_thresh << endl;
	return_thresh = i_thresh;
	DEBUG_MSG("    ordered keypoints: ");
	for (size_t i = 0; i < key_points.size(); ++i) //save and display the keypoints
		DEBUG_MSG("      keypoint: " << key_points[i].pt.x << " " << key_points[i].pt.y);

	// 复制图像并在关键点上绘制圆
	cvtColor(img, img, COLOR_GRAY2RGB);
	for (int n = 0; n < 3; n++) {
		cvpoint.x = key_points[n].pt.x;
		cvpoint.y = key_points[n].pt.y;
		if (img.size().height>800) {
			circle(img, cvpoint, 20, Scalar(0, 255, 255), 4);
		}
		else {
			circle(img, cvpoint, 10, Scalar(0, 255, 255), 4);
		}
	}

	vector<float> img_to_grdx(6, 0.0);
	vector<float> img_to_grdy(6, 0.0);
	vector<float> grd_to_imgx(6, 0.0);
	vector<float> grd_to_imgy(6, 0.0);

	// 通过方位点计算像素坐标和圆点阵列坐标相互转换关系（无梯形失真）
	calc_trans_coeff(key_points, marker_grid_locs, img_to_grdx, img_to_grdy, grd_to_imgx, grd_to_imgy);

	// 从关键点之间的灰阶确定阈值
	int xstart, xend, ystart, yend;
	float maxgray, mingray;
	xstart = key_points[0].pt.x;
	xend = key_points[1].pt.x;
	if (xend < xstart) {
		xstart = key_points[1].pt.x;
		xend = key_points[0].pt.x;
	}
	ystart = key_points[0].pt.y;
	yend = key_points[1].pt.y;
	if (yend < ystart) {
		ystart = key_points[1].pt.y;
		yend = key_points[0].pt.y;
	}

	maxgray = img_cpy.at<uchar>(ystart, xstart);
	mingray = maxgray;
	int curgray;
	for (int ix = xstart; ix <= xend; ix++) {
		for (int iy = ystart; iy <= yend; iy++) {
			curgray = img_cpy.at<uchar>(iy, ix);
			if (maxgray < curgray) maxgray = curgray;
			if (mingray > curgray) mingray = curgray;
		}
	}
	// 阈值等于关键点之间的（灰度最大值 + 灰度最小值）/ 2
	i_thresh = (maxgray + mingray) / 2;
	cout << "  min gray value (inside target keypoints): " << mingray << " max gray value: " << maxgray << endl;
	cout << "  getting the rest of the dots using average gray intensity value as threshold" << endl;
	cout << "    threshold to get dots: " << i_thresh << endl;

	// 获取余下的特征点坐标
	vector<KeyPoint> dots;
	get_dot_markers(img_cpy, dots, i_thresh, !invert, min_blob_size);
	DEBUG_MSG("    prospective grid points found: " << dots.size());

	// 根据平均大小和点是否落在中心框内过滤数据
	filter_dot_markers(dots, img_points, grd_points, grd_to_imgx, grd_to_imgy, img_to_grdx, img_to_grdy, dot_tol, img, false);

	// 初始化计算过程变量，迭代过滤圆点，避免出现杂噪点干扰
	int filter_passes = 1;
	int old_dot_num = 3;
	int new_dot_num = img_points.size();
	int max_dots = num_fiducials_x * num_fiducials_x - 3;

	// 最少迭代3次，最大迭代20次
	// 如果当前点数等于总特征点数-关键点数或者上一次迭代获取的点数=本次迭代获取的特征点数则跳出
	while ((old_dot_num != new_dot_num && new_dot_num != max_dots && filter_passes < 20) || filter_passes < 3) {
		// 更新上一次迭代得到的点数
		old_dot_num = new_dot_num;
		// xsfrom the good points that were found improve the mapping parameters
		calc_trans_coeff(img_points, grd_points, img_to_grdx, img_to_grdy, grd_to_imgx, grd_to_imgy);
		// filter dots based on avg size and whether the dots fall in the central box with the new parameters
		// the transformation now includes keystoning
		filter_dot_markers(dots, img_points, grd_points, grd_to_imgx, grd_to_imgy, img_to_grdx, img_to_grdy, dot_tol, img, false);
		filter_passes++;
		new_dot_num = img_points.size();
	}

	// 最后执行一次过滤，并将正确的特征点绘制出来
	filter_dot_markers(dots, img_points, grd_points, grd_to_imgx, grd_to_imgy, img_to_grdx, img_to_grdy, dot_tol, img, true);

	// 输出打印信息
	cout << "get_dot_targets():     good dots identified: " << new_dot_num << endl;
	DEBUG_MSG("    filter passes: " << filter_passes);
	// 如果提取的点数过少则返回错误码2
	if (new_dot_num < num_fiducials_x*num_fiducials_y*0.75) { // TODO fix this hard coded tolerance
		cout << "*** warning: not enough (non-keypoint) dots found" << endl;
		return 2; // not an issue with the thresholding (which would have resulted in error code 1)
	}
	else
		return 0;
}


// 带标记点的圆点标定算法
void calib_circular_new::ExtractTarget()
{

	// 参数初始化
	imagePoints[0].clear();
	imagePoints[1].clear();
	objectPoints.clear();
	endtime = 0;

	// 算法参数初始化
	int min_blob_size = min_blob_size_default;

	if (imagelist.size() % 2 != 0)
	{
		cout << "Error: 图片列表中数目为奇数，请重新查验\n";
		return;
	}
	clock_t start_t = clock();
	nimages = (int)imagelist.size() / 2;
	int i, j, k = (int)imagelist.size() / 2;
	goodImage.assign(nimages, true);

	Point2f zero_point;
	zero_point.x = 0;
	zero_point.y = 0;
	image_points_.resize(2);
	for (size_t i_cam = 0; i_cam < 2; i_cam++) {
		image_points_[i_cam].resize(nimages);
		for (size_t i_image = 0; i_image < nimages; i_image++) {
			image_points_[i_cam][i_image].resize(num_fiducials_x);
			for (int i_xpnt = 0; i_xpnt < num_fiducials_x; i_xpnt++) {
				image_points_[i_cam][i_image][i_xpnt].assign(num_fiducials_y, zero_point);
			}
		}
	}
	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);

	// 创建文件目录
	if (draw_intersection)
	{
		// 存放特征点提取图片
		create_directory(cal_debug_folder + "/intersection/");
	}
	// 存放取畸变后的图片
	create_directory(cal_debug_folder + "/undistort/");

	// 记录一组默认二值化参数值
	const int orig_thresh_start = 20;
	const int orig_thresh_end = 250;
	const int orig_thresh_step = 5;

	for (i = j = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			const string& filename = imagelist[i * 2 + k];
			Mat img = imread(image_folder + '/' + filename, 0);
			if (img.empty())
				break;
			if (imageSize == Size())
				imageSize = img.size();
			else if (img.size() != imageSize)
			{
				cout << "图片 " << filename << " 与第一张图片大小不一致. 跳过该组图片\n";
				break;
			}
			vector<KeyPoint> key_points;
			vector<KeyPoint> img_points;
			vector<KeyPoint> grd_points;
			int return_thresh = orig_thresh_start;
			int err_code = get_dot_targets(img, key_points, img_points, grd_points, return_thresh);

			// check if extraction failed
			if (err_code == 0) {
				threshold_start = return_thresh;
				threshold_end = return_thresh;
				threshold_step = return_thresh;
			}
			else if (i != 0 && err_code == 1) {
				threshold_start = orig_thresh_start;
				threshold_end = orig_thresh_end;
				threshold_step = orig_thresh_step;
				err_code = get_dot_targets(img, key_points, img_points, grd_points, return_thresh);
				if (err_code == 0) {
					threshold_start = return_thresh;
					threshold_end = return_thresh;
					threshold_step = return_thresh;
				}
			}
			// 此时报错的原因是由于blob检测异常导致
			else if (i == 0 && err_code == 1) {
				// check to see if the min_blob_size needs to be adjusted, first try 10 (smaller)
				min_blob_size = 10;
				DEBUG_MSG("calib_circular_new::get_dot_targets(): 重置min_blob_size为10，并重新提取");
				err_code = get_dot_targets(img, key_points, img_points, grd_points, return_thresh);
				if (err_code == 0) {
					threshold_start = return_thresh;
					threshold_end = return_thresh;
					threshold_step = return_thresh;
				}
			}
			if (draw_intersection)
			{
				cout << filename << endl;
				double sf = 1020. / MAX(img.rows, img.cols);
				Mat img_c;
				resize(img, img_c, Size(), sf, sf, INTER_LINEAR_EXACT);
				imshow("intersection", img_c);
				bool writed = imwrite(cal_debug_folder + "/intersection/" + filename.substr(0, filename.size() - 4) + "_intersection.jpg", img);
				if (!writed) {
					cout << filename << "带标记点的圆点图像存储失败\n";
				}
				char c = (char)waitKey(100);
				if (c == 27 || c == 'q' || c == 'Q') //Allow ESC to quit
					exit(-1);
			}
			else
				putchar('.');
			if (err_code != 0) {
				WARN_MSG(filename << " 提取圆点错误，错误码: " << err_code);
				goodImage[i] = false;
				break;
			}
			assert(key_points.size() == 3);
			image_points_[k][i][origin_loc_x][origin_loc_y].x = key_points[0].pt.x;
			image_points_[k][i][origin_loc_x][origin_loc_y].y = key_points[0].pt.y;
			image_points_[k][i][origin_loc_x + num_fiducials_origin_to_x_marker - 1][origin_loc_y].x = key_points[1].pt.x;
			image_points_[k][i][origin_loc_x + num_fiducials_origin_to_x_marker - 1][origin_loc_y].y = key_points[1].pt.y;
			image_points_[k][i][origin_loc_x][origin_loc_y + num_fiducials_origin_to_y_marker - 1].x = key_points[2].pt.x;
			image_points_[k][i][origin_loc_x][origin_loc_y + num_fiducials_origin_to_y_marker - 1].y = key_points[2].pt.y;

			for (int n = 0; n < (int)img_points.size(); n++) {
				image_points_[k][i][grd_points[n].pt.x][grd_points[n].pt.y].x = img_points[n].pt.x;
				image_points_[k][i][grd_points[n].pt.x][grd_points[n].pt.y].y = img_points[n].pt.y;
				
			}
			cout << endl;
			cout << "finished" << endl;
		}

		if (k == 2)
		{
			size_t num_common_pts = 0;
			for (int m = 0; m < num_fiducials_x; m++) {
				for (int n = 0; n < num_fiducials_y; n++) {
					bool common_pt = true;
					//如果所有相机的这个位置的值都不为零，就意味有一个共同点
					for (size_t i_cam = 0; i_cam < 2; i_cam++) {
						if (image_points_[i_cam][i][m][n].x <= 0 || image_points_[i_cam][i][m][n].y <= 0)
							common_pt = false;  //测试所有相机是否都存在该共同点?
					}
					if (common_pt) {
						num_common_pts++;
					}
				}
			}
			DEBUG_MSG("numer of common dots: " << num_common_pts);
			if (num_common_pts < (num_fiducials_x*num_fiducials_y*image_set_tol)) {
				//忽略该对图像
				WARN_MSG("excluding this image set due to not enough dots common among all images");
				goodImage[i] = false;
				continue;
			}
			else {
				goodImageList.push_back(imagelist[i * 2]);
				goodImageList.push_back(imagelist[i * 2 + 1]);
				j++;
			}		
		}
	}
	cout << j << "对图像成功被检测到！" << endl;
	nimages = j;

	if (nimages < 2)
	{
		cout << "错误: 图片对数过少不足以完成标定\n";
		return;
	}
	else {
		FileStorage fs(cal_debug_folder + "/GoodImagelist.xml", FileStorage::WRITE);
		fs << "goodImageList" << goodImageList;
		fs.release();
	}

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);
	int length = (int)imagelist.size() / 2;
	int index = 0;

	for (i = 0; i < length; i++)
	{
		if (!goodImage[i])
			continue;
		for (int i_x = 0; i_x < num_fiducials_x; i_x++) {
			for (int i_y = 0; i_y < num_fiducials_y; i_y++) {
				bool common_pt = true;
				for (int i_cam = 0; i_cam < 2; i_cam++) {
					if (image_points_[i_cam][i][i_x][i_y].x <= 0 || image_points_[i_cam][i][i_x][i_y].y <= 0) {
						common_pt = false;
						break;
					}
				}
				if (common_pt) {
					for (int i_cam = 0; i_cam < 2; i_cam++) {
						imagePoints[i_cam][index].push_back(image_points_[i_cam][i][i_x][i_y]);
					}
					objectPoints[index].push_back(Point3f((i_x - origin_loc_x)*cal_target_spacing_size, (i_y - origin_loc_y)*cal_target_spacing_size, 0));
					
				}				
			}
		}
		index++;
	}
	clock_t end_t = clock();
	endtime += (double)(end_t - start_t) / CLOCKS_PER_SEC;
	waitKey(1000);
	destroyAllWindows();
	return;

}
