#include "addtional_functions.h"

extern vector<string> process_with_chinese;//���洰��

string generate_path(string path_base)
{

	static int num = 1;
	while (true) {
		std::fstream file1;
		string path = ((path_base + to_string(num)));
		path = path + ".bmp";
		file1.open(path, ios::in);

		if (!file1)
		{
			file1.close();
			return path;
		}
		else
		{
			num++;
		}
	}


}

Mat hwnd2mat(HWND hwnd)
{

	HDC hwindowDC, hwindowCompatibleDC;
	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;
	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);
	RECT windowsize;
	GetClientRect(hwnd, &windowsize);// get the height and width of the screen
	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
	width = windowsize.right / 1;

	static bool showonce = true;
	if (showonce && height > 10 && width > 10)
	{
		cout << "���ڳߴ�" << width << "*" << height;
		showonce = false;
	}
	//else 
	//{
	//	cout << "Oppps!���ڳߴ�������ʾ����\n";
	//	Mat x;
	//	return x;
	//}
	src.create(height, width, CV_8UC4);
	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow
	// avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);
	return src;
}

void scaleIntervalSampling(const Mat &src, Mat &dst, double xRatio, double yRatio)
{
	//ֻ����uchar�͵�����
	CV_Assert(src.depth() == CV_8U);

	// ������С��ͼ��Ĵ�С
	//û���������룬��ֹ��ԭͼ�����ʱԽ��ͼ��߽�
	int rows = static_cast<int>(src.rows * xRatio);
	int cols = static_cast<int>(src.cols * yRatio);

	dst.create(rows, cols, src.type());

	const int channesl = src.channels();

	switch (channesl)
	{
	case 1: //��ͨ��ͼ��
	{
		uchar *p;
		const uchar *origal;

		for (int i = 0; i < rows; i++) {
			p = dst.ptr<uchar>(i);
			//��������
			//+1 �� -1 ����ΪMat�е������Ǵ�0��ʼ������
			int row = static_cast<int>((i + 1) / xRatio + 0.5) - 1;
			origal = src.ptr<uchar>(row);
			for (int j = 0; j < cols; j++) {
				int col = static_cast<int>((j + 1) / yRatio + 0.5) - 1;
				p[j] = origal[col];  //ȡ�ò�������
			}
		}
		break;
	}

	case 3://��ͨ��ͼ��
	{
		Vec3b *p;
		const Vec3b *origal;

		for (int i = 0; i < rows; i++) {
			p = dst.ptr<Vec3b>(i);
			int row = static_cast<int>((i + 1) / xRatio + 0.5) - 1;
			origal = src.ptr<Vec3b>(row);
			for (int j = 0; j < cols; j++) {
				int col = static_cast<int>((j + 1) / yRatio + 0.5) - 1;
				p[j] = origal[col]; //ȡ�ò�������
			}
		}
		break;
	}
	}
}
void average(const Mat &img, Point_<int> a, Point_<int> b, Vec3b &p);
void scalePartAverage(const Mat &src, Mat &dst, double xRatio, double yRatio)
{
	int rows = static_cast<int>(src.rows * xRatio);
	int cols = static_cast<int>(src.cols * yRatio);

	dst.create(rows, cols, src.type());

	int lastRow = 0;
	int lastCol = 0;

	Vec3b *p;
	for (int i = 0; i < rows; i++) {
		p = dst.ptr<Vec3b>(i);
		int row = static_cast<int>((i + 1) / xRatio + 0.5) - 1;

		for (int j = 0; j < cols; j++) {
			int col = static_cast<int>((j + 1) / yRatio + 0.5) - 1;

			Vec3b pix;
			average(src, Point_<int>(lastRow, lastCol), Point_<int>(row, col), pix);
			p[j] = pix;

			lastCol = col + 1; //��һ���ӿ����Ͻǵ������꣬�����겻��
		}
		lastCol = 0; //�ӿ�����Ͻ������꣬��0��ʼ
		lastRow = row + 1; //�ӿ�����Ͻ�������
	}
}
void average(const Mat &img, Point_<int> a, Point_<int> b, Vec3b &p)
{

	const Vec3b *pix;
	Vec3i temp;
	for (int i = a.x; i <= b.x; i++) {
		pix = img.ptr<Vec3b>(i);
		for (int j = a.y; j <= b.y; j++) {
			temp[0] += pix[j][0];
			temp[1] += pix[j][1];
			temp[2] += pix[j][2];
		}
	}

	int count = (b.x - a.x + 1) * (b.y - a.y + 1);
	p[0] = temp[0] / count;
	p[1] = temp[1] / count;
	p[2] = temp[2] / count;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) // �ص�������������������Բ���minecraft����
{
	TCHAR szTitle[200];
	TCHAR szClass[200];
	GetWindowText(hwnd, szTitle, sizeof(szTitle) / sizeof(TCHAR)); // ��ȡ��������
	GetClassName(hwnd, szClass, sizeof(szClass) / sizeof(TCHAR)); // ������
	string str = TCHAR2STRING(szTitle);
	auto idx = str.find("Minecraft");//����Ҫ��
	if (idx == string::npos);
	else {
		process_with_chinese.push_back(str);
		return TRUE;
	}

	idx = str.find("minecraft");//����Ҫ��
	if (idx == string::npos);
	else {
		process_with_chinese.push_back(str);
		return TRUE;
	}

	if (behas_Chinese(str)) //���������к�������
	{
		process_with_chinese.push_back(str);
		return TRUE;
	}
	return TRUE;
}



string TCHAR2STRING(TCHAR *STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;
}
BOOL behas_Chinese(std::string text)
{
	// �����ַ�����ѡ����뷽ʽ
	BOOL bHasChinese = FALSE;
	for (int i = 0; i < text.length(); i++)
	{
		if (text[i] < 0) // 0xBO 10110000 0xA1 10100000 ��һλ����1����ʾ�Ǹ���
		{
			bHasChinese = TRUE;
			break;
		}
	}
	return bHasChinese;
}
//stringת����wstring
LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length() - 1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

	return wcstring;
}

string LPCWSTR2string(LPCWSTR pwszSrc)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

	if (nLen <= 0) return std::string("");

	char* pszDst = new char[nLen];
	if (NULL == pszDst) return std::string("");

	WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
	pszDst[nLen - 1] = 0;

	std::string strTemp(pszDst);
	delete[] pszDst;

	return strTemp;
}

const char* OpenFile()
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = _T("BITMAP file(*.bmp)\0*.bmp\0");	// Ҫѡ����ļ���׺   
	ofn.lpstrInitialDir = _T("");				// Ĭ�ϵ��ļ�·��   
	ofn.lpstrFile = szBuffer;					// ����ļ��Ļ�����   
	ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	ofn.nFilterIndex = 0;

	//��־����Ƕ�ѡҪ����OFN_ALLOWMULTISELECT  
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	BOOL bSel = GetOpenFileName(&ofn);

	LPWSTR lpwszStrIn = szBuffer;
	LPSTR pszOut = NULL;
	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);

		// Double NULL Termination  
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		pszOut = new char[nOutputStrLen];		//���ܻ�����ڴ�й©

		if (pszOut)
		{
			memset(pszOut, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, pszOut, nOutputStrLen, 0, 0);
		}
	}
	return pszOut;
}


const bool SelectOpenFiles(char path[])
{
	TCHAR szBuffer[MAX_PATH] = { 0 };
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFilter = _T("BITMAP file(*.bmp)\0*.bmp\0");	// Ҫѡ����ļ���׺   
	ofn.lpstrInitialDir = _T("");				// Ĭ�ϵ��ļ�·��   
	ofn.lpstrFile = szBuffer;					// ����ļ��Ļ�����   
	ofn.nMaxFile = sizeof(szBuffer) / sizeof(*szBuffer);
	ofn.nFilterIndex = 0;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ALLOWMULTISELECT;	//��־����Ƕ�ѡҪ����OFN_ALLOWMULTISELECT  
	BOOL bSel = GetOpenFileName(&ofn);
	LPWSTR lpwszStrIn = szBuffer;

	if (lpwszStrIn != NULL)
	{
		int nInputStrLen = wcslen(lpwszStrIn);

		// Double NULL Termination  
		int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, NULL, 0, 0, 0) + 2;
		if (path)
		{
			memset(path, 0x00, nOutputStrLen);
			WideCharToMultiByte(CP_ACP, 0, lpwszStrIn, nInputStrLen, path, nOutputStrLen, 0, 0);

			return true;
		}
	}
	return false;
}




string selectPath() {
	string strInitFolder;
	TCHAR szPath[MAX_PATH] = _T("");
	LPMALLOC lpMalloc = NULL;
	string strFolder;
	BROWSEINFO sInfo;
	LPITEMIDLIST lpidlBrowse = NULL;
	if (::SHGetMalloc(&lpMalloc) != NOERROR)
		return strFolder;
	::ZeroMemory(&sInfo, sizeof(BROWSEINFO));
	sInfo.pidlRoot = 0;
	sInfo.pszDisplayName = szPath;
	sInfo.lpszTitle = _T("��ѡ������Ҫ��Ŀ���ļ��У�");
	sInfo.ulFlags = BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
	sInfo.lpfn = NULL;
	// ��ʾ�ļ���ѡ��Ի���
	lpidlBrowse = ::SHBrowseForFolder(&sInfo);
	if (lpidlBrowse != NULL) {
		// ȡ���ļ�����
		if (::SHGetPathFromIDList(lpidlBrowse, szPath)) {
			strFolder = TCHAR2STRING(szPath);
			cout << "ѡ����ļ���Ϊ:" << endl;
			cout << strFolder << endl;
			return strFolder;
		}
	}
	if (lpidlBrowse != NULL) {
		::CoTaskMemFree(lpidlBrowse);
	}
	lpMalloc->Release();
	return strFolder;
}