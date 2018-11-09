#pragma once
#include <windows.h>
#include "CommDlg.h"
#include "tchar.h"
#include <shlobj.h>
#include <io.h>
#include <iostream>
#include <list>
#include <fstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


#define CAMERA_ROWS 60
#define CAMERA_COLS 80

#define _target_width  CAMERA_COLS //80*60�ĻҶ�ͼ
#define _target_hight  CAMERA_ROWS //80*60�ĻҶ�ͼ

using namespace std;
using namespace cv;

Mat hwnd2mat(HWND hwnd);//ʵʱ�������
BOOL behas_Chinese(std::string text);//�жϴ������Ƿ�������
string TCHAR2STRING(TCHAR *STR);//					TCHAR  ->	string
LPCWSTR stringToLPCWSTR(std::string orig);//		string ->	LPCWSTR
string LPCWSTR2string(LPCWSTR pwszSrc);//				LPCWSTR ->  string

int get_string_num(string str);
void scalePartAverage(const Mat &src, Mat &dst, double xRatio, double yRatio);//ת���ֱ���
void scaleIntervalSampling(const Mat &src, Mat &dst, double xRatio, double yRatio);//������ת���ֱ���
string generate_path(string s);//�趨���ʵĴ洢Ŀ¼
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam); // �ص�������������������Բ���minecraft����
void on_Mouse(int event, int x, int y, int flags, void *ustc);
void find_file_in_path(string directoryPath, vector<string> &bmp_filenames);//����ͼƬ�ļ�
void change_console_color(int k);
string selectPath();//ѡ��Ŀ¼
/***************************************************************************/
void imageProcessOnChipAndOnVS(uint8_t(*img)[CAMERA_COLS]);//ͼ�������/***/
/***************************************************************************/


//												char	->	LPCWSTR
/*
	char ch[1024] = "SB c++";
	int num = MultiByteToWideChar(0,0,ch,-1,NULL,0);
	wchar_t *wide = new wchar_t[num];
	MultiByteToWideChar(0,0,ch,-1,wide,num);

	int num = MultiByteToWideChar(0, 0, szFind, -1, NULL, 0);
	wchar_t *szFind_lpcstr = new wchar_t[num];
	MultiByteToWideChar(0, 0, szFind, -1, szFind_lpcstr, num);

													LPCWSTR	->	char
	wchar_t widestr[1024] = L"SB java";
	int num = WideCharToMultiByte(CP_OEMCP,NULL,widestr,-1,NULL,0,NULL,FALSE);
	char *pchar = new char[num];
	WideCharToMultiByte (CP_OEMCP,NULL,widestr,-1,pchar,num,NULL,FALSE);
*/