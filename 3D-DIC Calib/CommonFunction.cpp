#include "CommonFunction.h"

// floatתstring
string ftos(const float &Num)
{
	ostringstream oss;
	oss << Num;
	string str(oss.str());
	return str;
}

// intתstring
string itos(const int &Num)
{
	ostringstream oss;
	oss << Num;
	string str(oss.str());
	return str;
}

// �����ļ�Ŀ¼
void create_directory(const string &folder)
{
	CreateDirectory(folder.c_str(), NULL);
}

// ���������ľ���
float dist2(KeyPoint pnt1, KeyPoint pnt2) 
{
	return (pnt1.pt.x - pnt2.pt.x)*(pnt1.pt.x - pnt2.pt.x) + (pnt1.pt.y - pnt2.pt.y)*(pnt1.pt.y - pnt2.pt.y);
}

// ��������ֵ����Ƕ�
float angle_2d(const float & x1,
	const float & y1,
	const float & x2,
	const float & y2) 
{

	float dtheta = 0, theta1 = 0, theta2 = 0;
	theta1 = atan2(y1, x1);
	theta2 = atan2(y2, x2);
	dtheta = theta2 - theta1;
	while (dtheta > PI)
		dtheta -= TWOPI;
	while (dtheta < -PI)
		dtheta += TWOPI;
	return(dtheta);
}


//�����������ɴ�С����
void order_dist3(vector<float> & dist,
	vector<int> & dist_order) 
{
	dist_order[0] = 0;
	dist_order[2] = 0;
	for (int i = 1; i < 3; i++) {
		if (dist[dist_order[0]] < dist[i]) dist_order[0] = i;
		if (dist[dist_order[2]] > dist[i]) dist_order[2] = i;
	}
	if (dist_order[0] == dist_order[2]) assert(false);
	dist_order[1] = 3 - (dist_order[0] + dist_order[2]);
}

// �������굽ͼ������ת��
void grid_to_image(IN const float & grid_x,
	IN const float & grid_y,
	OUT float & img_x,
	OUT float & img_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h) 
{
	img_x = grd_to_imgx[0] + grd_to_imgx[1] * grid_x + grd_to_imgx[2] * grid_y + grd_to_imgx[3] * grid_x * grid_y + grd_to_imgx[4] * grid_x * grid_x + grd_to_imgx[5] * grid_y * grid_y;
	img_y = grd_to_imgy[0] + grd_to_imgy[1] * grid_x + grd_to_imgy[2] * grid_y + grd_to_imgy[3] * grid_x * grid_y + grd_to_imgy[4] * grid_x * grid_x + grd_to_imgy[5] * grid_y * grid_y;
	if (img_x<0) img_x = 0;
	if (img_x>img_w - 1) img_x = img_w - 1;
	if (img_y<0) img_y = 0;
	if (img_y>img_h - 1) img_y = img_h - 1;
}

// ͼ�����굽��������ת��
void image_to_grid(IN const float & img_x,
	IN const float & img_y,
	OUT float & grid_x,
	OUT float & grid_y,
	IN const vector<float> & img_to_grdx,
	IN const vector<float> & img_to_grdy) 
{
	grid_x = img_to_grdx[0] + img_to_grdx[1] * img_x + img_to_grdx[2] * img_y + img_to_grdx[3] * img_x * img_y + img_to_grdx[4] * img_x * img_x + img_to_grdx[5] * img_y * img_y;
	grid_y = img_to_grdy[0] + img_to_grdy[1] * img_x + img_to_grdy[2] * img_y + img_to_grdy[3] * img_x * img_y + img_to_grdy[4] * img_x * img_x + img_to_grdy[5] * img_y * img_y;
}

// �����߽��
void create_bounding_box(OUT vector<float> & box_x,
	OUT vector<float> & box_y,
	IN const int num_fiducials_x,
	IN const int num_fiducials_y,
	IN const vector<float> & grd_to_imgx,
	IN const vector<float> & grd_to_imgy,
	IN const int img_w,
	IN const int img_h)
{
	assert(box_x.size() == box_y.size());
	assert(box_x.size() == 5);
	float xgrid, ygrid;
	//xmin, ymin point
	xgrid = -0.5;
	ygrid = -0.5;
	grid_to_image(xgrid, ygrid, box_x[0], box_y[0], grd_to_imgx, grd_to_imgy, img_w, img_h);
	//xmax, ymin point
	xgrid = num_fiducials_x - 1 + 0.5;
	ygrid = -0.5;
	grid_to_image(xgrid, ygrid, box_x[1], box_y[1], grd_to_imgx, grd_to_imgy, img_w, img_h);
	//xmax, ymax point
	xgrid = num_fiducials_x - 1 + 0.5;
	ygrid = num_fiducials_y - 1 + 0.5;
	grid_to_image(xgrid, ygrid, box_x[2], box_y[2], grd_to_imgx, grd_to_imgy, img_w, img_h);
	//xmin, ymax point
	xgrid = -0.5;
	ygrid = num_fiducials_y - 1 + 0.5;
	grid_to_image(xgrid, ygrid, box_x[3], box_y[3], grd_to_imgx, grd_to_imgy, img_w, img_h);
	//�γɷ�յı߽��
	box_x[4] = box_x[0];
	box_y[4] = box_y[0];
	DEBUG_MSG("create_bounding_box(): (" << box_x[0] << "," << box_y[0] << ") (" << box_x[1] << "," << box_y[1] << ") (" <<
		box_x[2] << "," << box_y[2] << ") (" << box_x[3] << "," << box_y[3] << ")");
}

//���ı����ڵ�һ����
bool is_in_quadrilateral(const float & x,
	const float & y,
	const vector<float> & box_x,
	const vector<float> & box_y) 
{
	
	float angle = 0.0;
	//ȷ���Ǹ��ı���
	assert(box_x.size() == 5);
	assert(box_y.size() == 5);

	for (int i = 0; i < 4; i++) {
		// �õ�����αߵ������˵㲢����Ӵ���㵽ÿ���˵����������
		const float dx1 = box_x[i] - x;
		const float dy1 = box_y[i] - y;
		const float dx2 = box_x[i + 1] - x;
		const float dy2 = box_y[i + 1] - y;
		angle += angle_2d(dx1, dy1, dx2, dy2); 
	}
	// ����Ƕȴ��� PI����õ��ڶ������
	if (abs(angle) >= PI) {
		return true;
	}
	return false;
}

// ch:��������ת��ΪOpenCV֧�ֵ�Mat��ʽ
bool Convert2Mat(const MV_FRAME_OUT_INFO_EX* pstImageInfo, 
	unsigned char *pData, 
	Mat &srcImage)
{
	if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else
	{
		printf("unsupported pixel format\n");
		return false;
	}

	if (NULL == srcImage.data)
	{
		return false;
	}
	return true;
}


// ���ļ��ж�ȡ�ڲ���Ϣ
bool readIntrinsic(IN string filename, Mat &M1, Mat &D1, Mat &M2, Mat &D2) 
{
	if (filename.empty()) {
		cout << "�ļ���Ϊ�գ��޷���ȡ" << endl;
		return 0;
	}
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "�޷����ļ�" + filename << endl;
		return 0;
	}
	fs["M1"] >> M1;
	fs["D1"] >> D1;
	fs["M2"] >> M2;
	fs["D2"] >> D2;

	fs.release();
	return 1;
}

// ���ļ��ж�ȡ�����Ϣ
bool readExtrinsic(IN string filename, Mat &R, Mat &T, Mat &R1, Mat &P1, Mat &R2, Mat &P2, Mat &Q)
{
	if (filename.empty()) {
		cout << "�ļ���Ϊ�գ��޷���ȡ" << endl;
		return 0;
	}
	FileStorage fs(filename, FileStorage::READ);
	if (!fs.isOpened())
	{
		cout << "�޷����ļ�" + filename << endl;
		return 0;
	}
	fs["R"] >> R;
	fs["T"] >> T;
	fs["R1"] >> R1;
	fs["P1"] >> P1;
	fs["R2"] >> R2;
	fs["P2"] >> P2;
	fs["Q"] >> R;

	fs.release();
	return 1;
}

// ��opencv�е�MAT�����xls������
bool MatToxls(IN Mat &M, IN string filename)
{
	if (filename.empty() || M.empty()) {
		cout << "��ȷ��������Ϣ��Ϊ��" << endl;
		return 0;
	}
	ofstream Fs(filename);
	if (!Fs.is_open())
	{
		cout << "�ļ��޷�����" << endl;
		return 0;
	}
	int height = M.rows;
	int width = M.cols;
	for (int i = 0; i<height; i++)
	{
		for (int j = 0; j<width; j++)
		{
			Fs << (int)M.ptr<uchar>(i)[j] << '\t';
		}
		Fs << endl;
	}
	Fs.close();
	cout << "����ɹ�" << endl;
	return 1;
}

// ��stringת��Ϊcalib_pattern
calib_pattern string_to_target_type(string & in) {
	// ���ַ���ת��Ϊ��д
	transform(in.begin(), in.end(), in.begin(), ::toupper);
	for (int i = 0; i < MAX_TARGET_TYPE; ++i) {
		if (pattern_to_string(static_cast<calib_pattern>(i)) == in)
			return static_cast<calib_pattern>(i);
	}
	cout << "Error: Ŀ���ʽ " << in << " ������" << endl;
	return NO_SUCH_TARGET_TYPE; // prevent no return errors
}

// ���궨ͼ������ת��Ϊstring
string pattern_to_string(calib_pattern in) {
	assert(in < MAX_TARGET_TYPE);
	const static char * targetStrings[] = {
		"CALIB_PATTERN_CHEESEBOARD",
		"CALIB_PATTERN_CIRCULAR",
		"CALIB_PATTERN_SPECIAL"
	};
	return targetStrings[in];
};