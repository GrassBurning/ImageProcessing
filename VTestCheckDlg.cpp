
// VTestCheckDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "VTestCheck.h"
#include "VTestCheckDlg.h"
#include "afxdialogex.h"
#include "Password.h"
#include "CvvImage.h"
#include "cv.h"
#include "highgui.h"
#include "easysize.h"
#include "tisudshl.h"
#include "General.h"
#include<vector>
//#include "CmdHelper.h"//这两个头文件是需要包含的，这样才能进行文件的生成
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// #ifdef _DEBUG
// #pragma comment (lib, "opencv_calib3d231d.lib")
// #pragma comment (lib, "opencv_contrib231d.lib")
// #pragma comment (lib,"opencv_imgproc231d.lib")
// #pragma comment (lib, "opencv_core231d.lib")
// #pragma comment (lib, "opencv_features2d231d.lib")
// #pragma comment (lib, "opencv_flann231d.lib")
// #pragma comment (lib, "opencv_gpu231d.lib")
// #pragma comment (lib, "opencv_highgui231d.lib")
// #pragma comment (lib, "opencv_legacy231d.lib")
// #pragma comment (lib, "opencv_ml231d.lib")
// #pragma comment (lib, "opencv_objdetect231d.lib")
// #pragma comment (lib, "opencv_ts231d.lib")
// #pragma comment (lib, "opencv_video231d.lib")
// //#pragma comment (lib, "opencv_nonfree231d.lib")
// #else
// #pragma comment (lib, "opencv_calib3d231.lib")
// #pragma comment (lib, "opencv_contrib231.lib")
// #pragma comment (lib, "opencv_imgproc231.lib")
// #pragma comment (lib, "opencv_core231.lib")
// #pragma comment (lib, "opencv_features2d231.lib")
// #pragma comment (lib, "opencv_flann231.lib")
// #pragma comment (lib, "opencv_gpu231.lib")
// #pragma comment (lib, "opencv_highgui231.lib")
// #pragma comment (lib, "opencv_legacy231.lib")
// #pragma comment (lib, "opencv_ml231.lib")
// #pragma comment (lib, "opencv_objdetect231.lib")
// #pragma comment (lib, "opencv_ts231.lib")
// #pragma comment (lib, "opencv_video231.lib")
// //#pragma comment (lib, "opencv_nonfree231.lib")
// #endif

// CVTestCheckDlg 对话框
using namespace cv;
using namespace std;
using namespace _DSHOWLIB_NAMESPACE;
using namespace DShowLib;
CPassword password;
CBitmap BitmapPass;
CBitmap BitmapFail;
CBitmap BitmapWait;
CBitmap BitmapStop;
CBitmap BitmapTest;
CVTestCheckDlg * ex = NULL;
Point PrePosBegin(0,0);
Point PrePosEnd(0,0);
Rect SelectRect;
bool SelectFlag;
Mat PShowImg;
BYTE* pBuf[1];
//Grabber grabber;
DShowLib::Grabber* m_pGrabber;
tFrameHandlerSinkPtr pSink;
void onMouse(int Event,int x,int y, int flags,void * param);
UINT TestEntry(LPVOID pParam);

typedef struct PicChangeInfo
{
	int iPosition;
	char cDataR;
	char cDataG;
	char cDataB;
	int state;  /*
				1:背景 2:目标出现 3：目标结束 4：字体出现 5：字体结束

				*/
}sPicChangeInfo,*pPicChangeInfo;
typedef struct TestPicConfig
{
	int iTargetPixMinLength;    //
	int iBackgroundMinLength;     //背景最小长度
	int iBackgroundDeviation;  //多大范围内一直为背景值，则认为背景开始
	int iTargetMinLength;          //目标最小长度
	int iTargetDeviation;       //多大范围内一直为目标值，则认为目标开始


	int iFont1Spec;
	int iFont1MinLength;           //字体最小长度
	int iFont1BandMinWidth;      //字体最小间距
	int iFont1SkipMaxWidth;      //字体跨度最大宽度
	int iFont1Deviation;         //多大范围内一直为字体值，则认为字体开始
	int iFont1MinWidth;

	int iFont2Spec;
	int iFont2MinLength;           //字体最小长度
	int iFont2BandMinWidth;      //字体最小间距
	int iFont2SkipMaxWidth;      //字体跨度最大宽度
	int iFont2Deviation;         //多大范围内一直为字体值，则认为字体开始
	int iFont2MinWidth;

	int iSpotMinLength;            //点最小长度
	int iSpotMinWidth;             //点最小宽度
	int iSpotDeviation;         //多大范围内一直为点值，则认为点开始

	int iBandFirstMinWidth;        //底端第一个白色背景宽度
	int iBandSecondMinWidth ;       //底端第二个白色背景宽度
	int iBandThirdMinWidth;        //底端第三个白色背景宽度



	int iSpotSpec;
	int iTargetSpec1;
	int iTargetSpec2;

	int iBackgroundSpec;

	int iTargetTopBottomMinPlace;         //当目标上下超出该值，则认为目标不存在
	int iTargetLeftRightMinPlace;         //当目标左右超出该值，则认为目标不存在
	
}sTestPicConfig,*pTestPicConfig;

typedef struct TestPicResult
{
	int iEdgeLength;
	int iEdgePosition;
	int iEdgeLeftPosition;
	int iEdgeRightPosition;
	int iSpotFirstLeftPosition;
	int iSpotSecondLeftPosition;
	int iSpotFirstRightPosition;
	int iSpotSecondRightPosition;

	int iBandFirstWidth;
	int iBandSecondWidth;
	int iBandThirdWidth;

	int iFont1BottomPosition;
	int iFont1TopPosition;
	int iFont1BeginPosition;
	int iFont1EndPosition;

	int iFont2BottomPosition;
	int iFont2TopPosition;
	int iFont2BeginPosition;
	int iFont2EndPosition;

	bool bA1CheckResult;
	vector<vector<int>> iA1FontChange;

	bool bRCheckResult;
	vector<vector<int>> iRFontChange;

	bool bUCheckResult;
	vector<vector<int>> iUFontChange;

	bool bBCheckResult;
	vector<vector<int>> iBFontChange;

	bool bA2CheckResult;
	vector<vector<int>> iA2FontChange;

	int iSpotLength;
	int iSpotPosition;
	vector<vector<int>> vFontInfo;

} sTestPicResult,*pTestPicResult;
int fLoadConfig(sTestPicConfig& TestConfig);
int fTargetLocation(Mat * mPic,int iBottom,sTestPicResult& TestResult,sTestPicConfig& TestConfig);
int splitcharacter(uchar**pFont,int iFontSkip,int &k,vector<vector<int>>& iFontChange,sTestPicConfig& TestConfig,int & iEdgeRightPosition);
int checkA1(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig);
int checkR(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig);
int checkU(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig);
int checkB(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig);
int checkA2(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig);



CVTestCheckDlg::CVTestCheckDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVTestCheckDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVTestCheckDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_P_PF, m_Picture_PF);
	DDX_Control(pDX, IDC_ListInfo, m_ListBox);
	DDX_Control(pDX, IDC_STATICVIDEO, m_cStaticVideoWindow);
}

BEGIN_MESSAGE_MAP(CVTestCheckDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CVTestCheckDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVTestCheckDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_B_SetPosition, &CVTestCheckDlg::OnBnClickedBSetposition)
	ON_BN_CLICKED(IDC_Config, &CVTestCheckDlg::OnBnClickedConfig)

END_MESSAGE_MAP()
#ifdef EASYSIZEOPEN	
BEGIN_EASYSIZE_MAP(CVTestCheckDlg)
	//EASYSIZE(Control_ID,left,top,right,bottom,options)
	EASYSIZE(IDC_STATICVIDEO,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_P_PF,IDC_STATICVIDEO,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,0)
	EASYSIZE(IDOK,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDCANCEL,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_Config,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_B_SetPosition,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_S1,ES_KEEPSIZE,IDC_Config,ES_BORDER,ES_KEEPSIZE,0) // test info
	EASYSIZE(IDC_S2,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	EASYSIZE(IDC_Edit_PF,ES_BORDER,ES_KEEPSIZE,ES_KEEPSIZE,ES_BORDER,0)
	EASYSIZE(IDC_ListInfo,IDC_STATICVIDEO,IDC_S1,ES_BORDER,ES_BORDER,0)
END_EASYSIZE_MAP
#endif
// CVTestCheckDlg 消息处理程序

BOOL CVTestCheckDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
#ifdef EASYSIZEOPEN	
	INIT_EASYSIZE;
#endif
	BitmapPass.LoadBitmap(IDB_BITMAP_Pass);
	BitmapFail.LoadBitmap(IDB_BITMAP_Fail);
	BitmapWait.LoadBitmap(IDB_BITMAP_Wait);
	BitmapStop.LoadBitmap(IDB_BITMAP_Stop);
	BitmapTest.LoadBitmap(IDB_BITMAP_Test);
	m_Picture_PF.SetBitmap(BitmapStop);
	DShowLib::InitLibrary();
	atexit( ExitLibrary );
	m_pGrabber = new DShowLib::Grabber();
	ASSERT( m_pGrabber );
	//if( !m_pGrabber->loadDeviceStateFromFile( "lastSelectedDeviceState.xml" ) )
	{
		//std::cout << "Device opened from: <" << "lastSelectedDeviceState.xml" << ">." << std::endl;
			if( !m_pGrabber->showDevicePage() || !m_pGrabber->isDevValid() )
		{
			return false;
		}
		m_pGrabber->saveDeviceStateToFile( "lastSelectedDeviceState.xml", true, true, false );
	
	}

	m_pGrabber->setOverlayBitmapPathPosition( ePP_NONE );
//	pSink = FrameHandlerSink::create( eY800, 1 );
	pSink = FrameHandlerSink::create( eRGB24, 3 );

	// We use snap mode.
	pSink->setSnapMode( false );

	// Set the sink.
	m_pGrabber->setSinkType( pSink );
//	m_cStaticVideoWindow.SetWindowPos(NULL,0,0,640,480,SWP_NOMOVE|SWP_NOZORDER);
	CRect rect;
	m_cStaticVideoWindow.GetWindowRect(&rect);
	m_pGrabber->setDefaultWindowPosition(false);   //取消采用Window自带图像映射显示方式
	m_pGrabber->setWindowSize(rect.Width(),rect.Height());
	m_pGrabber->setHWND( m_cStaticVideoWindow.m_hWnd );
//	m_pGrabber->
	
// Prepare the live mode, to get the output size if the sink.
	if( !m_pGrabber->prepareLive( true ) )
	{
		std::cerr << "Could not render the VideoFormat into a eY800 sink.";
		return -1;
	}
#if 0	
	FrameTypeInfo info;
	pSink->getOutputFrameType( info );
	
	pBuf[0] = new BYTE[info.buffersize];

	tMemBufferCollectionPtr pCollection = MemBufferCollection::create(info,1,pBuf);
	pCollection->getBuffer(0)->getSize().cx;
	if( pCollection == 0 || !pSink->setMemBufferCollection( pCollection ) )
	{
		std::cerr << "Could not set the new MemBufferCollection, because types do not match.";
		return -1;
	}
#endif
	
	m_pGrabber->startLive( true );

#if 0
	m_pGrabber->showDevicePage();  //配置对话框
	if( ! m_pGrabber->isDevValid() )
	{
		// Set the window that should display the live video.
		//pGrabber->setHWND( m_hWnd );
		AfxMessageBox( TEXT("No device was selected."));
		return FALSE;
	}
	m_pGrabber->closeDev();
#endif
	UpdateWindow();
	ex = this;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVTestCheckDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		CRect rect1;
		m_pGrabber->setDefaultWindowPosition(true);
		m_pGrabber->stopLive();
//		m_pGrabber->closeDev();
//		GetDlgItem(IDC_STATICVIDEO)->GetWindowRect(&rect1);
		m_cStaticVideoWindow.GetWindowRect(&rect1);
//		m_pGrabber->openDev();
		m_pGrabber->setDefaultWindowPosition(false);
		m_pGrabber->setWindowSize(rect1.Width(),rect1.Height());
		m_pGrabber->setHWND( GetDlgItem( IDC_STATICVIDEO )->m_hWnd );
		m_pGrabber->startLive();
	
		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);

// 		//定义并创建一个内存设备环境
// 		CDC  m_dcMemory;
// 		GetDlgItem(IDC_Picture)->GetWindowRect(&rect);  
// 		if( !m_dcMemory.CreateCompatibleDC(&dc) )          // 创建兼容性的DC
// 			return;
// 		SetStretchBltMode(dc.m_hDC,STRETCH_HALFTONE);
		
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVTestCheckDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

int ii = 0;

void CVTestCheckDlg::OnBnClickedOk()
{
#if 1
//	Mat srcImage1 = imread("D:\\Work\\opencv\\Desktop\\5.bmp",1);
	Mat srcImage1 = imread("D:\\tt\\12345.bmp",1);
	sTestPicConfig TestConfig;
	sTestPicResult TestResult;
	fLoadConfig(TestConfig);
	fTargetLocation(&srcImage1,2,TestResult,TestConfig);
	//	imshow("Example1",srcImage1);
	//	cvWaitKey();
	return ;
#endif

#if 0
	long height,width;
	Mat *a;
//	width = m_pGrabber->getAcqSizeMaxX();
//	height = m_pGrabber->getAcqSizeMaxY();
//	pSink->setSnapMode( false );
//	m_pGrabber->startLive(false);
//	pSink->snapImages( 1 ); 
	smart_ptr<MemBuffer> pBuffer = pSink->getLastAcqMemBuffer();
	width = pBuffer->getSize().cx;
	height = pBuffer->getSize().cy;
//	
//	m_pGrabber->stopLive();
	a = new Mat(height,width,CV_8UC3,(uchar*)pBuffer->getPtr());
//	pSink->setSnapMode( false );
//	m_pGrabber->startLive(true);
	Mat b;
	a->copyTo(b);
	CString ab;
	ii ++;
	ab.Format("%d.bmp",ii);
	imwrite(ab.GetBuffer(ab.GetLength()),b);
//	m_pGrabber->startLive(true);
//	namedWindow("img");
//	imshow("img", b);
//	waitKey(0);
#endif

}


UINT Test(LPVOID pParam)
{
	long height,width;
	Mat *a;
	Mat b;
	smart_ptr<MemBuffer> pBuffer = pSink->getLastAcqMemBuffer();
	width = pBuffer->getSize().cx;
	height = pBuffer->getSize().cy;
	a = new Mat(height,width,CV_8UC3,(uchar*)pBuffer->getPtr());

	a->copyTo(b);
	imwrite("a.bmp",b);

	namedWindow("img");
	imshow("img", b);
	waitKey(0);
	return 0;
}




void CVTestCheckDlg::OnBnClickedCancel()
{

	
	// Stop the live video.
	m_pGrabber->stopLive();
	
	// Close the device.
	m_pGrabber->closeDev();
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CVTestCheckDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
#ifdef EASYSIZEOPEN	
	UPDATE_EASYSIZE;
#endif

	UpdateWindow();
	// TODO: 在此处添加消息处理程序代码
}


void CVTestCheckDlg::OnBnClickedBSetposition()
{
	// TODO: 在此添加控件通知处理程序代码

	long height,width;
	Mat *a;
	Mat b;
	smart_ptr<MemBuffer> pBuffer = pSink->getLastAcqMemBuffer();
	width = pBuffer->getSize().cx;
	height = pBuffer->getSize().cy;
	a = new Mat(height,width,CV_8UC3,(uchar*)pBuffer->getPtr());
	a->copyTo(PShowImg);
	delete a;
//	PShowImg = imread("D:\\2.bmp",1);
	if(PShowImg.empty() || !PShowImg.data)
	{
		CString tmp;
		tmp.Format("Can not load image !\n");
		AfxMessageBox(tmp);
		return;
	}
	namedWindow("PostionImg");
	imshow("PostionImg",PShowImg);
	setMouseCallback("PostionImg",onMouse,0);
//	waitKey(0);
	return;
}

void onMouse(int Event,int x,int y, int flags,void * param)
{
	char temp[46];
	Mat pShowImgTmp;
	memset(temp,0,sizeof(temp));
	PShowImg.copyTo(pShowImgTmp);
	if(Event==CV_EVENT_LBUTTONDOWN)
	{
		SelectRect.x = x;
		SelectRect.y = y;
		PrePosBegin.x = x;
		PrePosBegin.y = y;
		SelectFlag = true;
	}
#if 1
	else if(SelectFlag &&Event == CV_EVENT_MOUSEMOVE)
	{
		PrePosEnd.x = x;
		PrePosEnd.y = y;
		sprintf(temp, "P(%d,%d)", x, y);
		putText(pShowImgTmp, temp, PrePosEnd, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));//只是实时显示鼠标移动的坐标 
		rectangle(pShowImgTmp,PrePosBegin,PrePosEnd,Scalar(0,255,0),2);
		imshow("PostionImg",pShowImgTmp);
	}
#endif
	else if(SelectFlag && Event == CV_EVENT_LBUTTONUP)
	{
		//显示框出的图
		PrePosEnd.x = x;
		PrePosEnd.y = y;
		SelectRect = Rect(PrePosBegin,PrePosEnd);
		SelectFlag = false;		
	}else if(Event==CV_EVENT_LBUTTONDBLCLK)
	{
		CvScalar s;
		IplImage tmp = PShowImg;
		s = cvGet2D(&tmp,PrePosEnd.y,PrePosEnd.x);
		sprintf(temp, "pixel(%f,%f,%f)",s.val[0],s.val[1],s.val[2]);
		putText(pShowImgTmp, temp, PrePosEnd, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 0, 255));
		imshow("PostionImg",pShowImgTmp);
	}
	return;
}
void AddProcInfo(CString strProcInfo)
{
	int lines;
	lines = ex->m_ListBox.AddString(strProcInfo);
	ex->m_ListBox.SetCurSel(lines);
	return;
}




void CVTestCheckDlg::OnBnClickedConfig()
{
	// Stop the live video.
	m_pGrabber->stopLive();

	// Close the device.
	m_pGrabber->closeDev();
	// TODO: 在此添加控件通知处理程序代码
	if(password.DoModal() == IDOK)
	{

	}
}

typedef struct TestThreadInfo
{
	CWinThread *pTestThreadEntry;
	bool bTestResult;
	bool bTestEnd;
	CTime TestTime;
	Mat * mTestData;
	UINT iTestPosition;
	ULONG iTestItem;

} *pTestThreadInfo,sTestThreadInfo;

vector<sTestThreadInfo> GlobalTestInfo;
UINT TestBegin(LPVOID pParam)
{
	sTestThreadInfo TestInfo;
	long height,width;
	Mat *pData;
	ULONG iItem;

	iItem = 0;
	while (true)
	{
		TestInfo.bTestResult = false;
		TestInfo.bTestEnd = false;
		TestInfo.iTestPosition = -1;
		TestInfo.mTestData = NULL;
		TestInfo.pTestThreadEntry = NULL;
		 
		width = 0;
		height = 0;
		pData = NULL;
		TestInfo.TestTime = CTime::GetCurrentTime();
		TestInfo.iTestItem = iItem;
		smart_ptr<MemBuffer> pBuffer = pSink->getLastAcqMemBuffer();
		width = pBuffer->getSize().cx;
		height = pBuffer->getSize().cy;
		pData = new Mat(height,width,CV_8UC3,(uchar*)pBuffer->getPtr());

		TestInfo.mTestData = pData;
		TestInfo.pTestThreadEntry = AfxBeginThread(TestEntry,(LPVOID) &iItem,0, CREATE_SUSPENDED, NULL);

		
		if (TestInfo.pTestThreadEntry)
		{
			TestInfo.pTestThreadEntry->m_bAutoDelete = FALSE;
			TestInfo.pTestThreadEntry->ResumeThread();
		}
		iItem ++;
		GlobalTestInfo.push_back(TestInfo);
		Sleep(1000);
	}
	
	return 0;
}
typedef struct TestLog
{
	UINT iTestPosition;
} iTestLog;
UINT TestResultHandle(LPVOID pParam)
{

	UINT i,j,length;
	vector<iTestLog> TestLog;
	
	i = 0;
	j = 0;
	length = 0;
	while(TRUE)
	{
		if (GlobalTestInfo.empty())
		{
			Sleep(2000);
			continue;
		}
		length = GlobalTestInfo.size();
		for (i = 0; i < GlobalTestInfo.size(); i ++)
		{
			if (TestLog.empty())
			{
				break;
			}
			for (j = 0;j < TestLog.size(); j ++)
			{
				vector<sTestThreadInfo>::iterator iter;
				iter = GlobalTestInfo.begin();
				if (TestLog[j].iTestPosition == GlobalTestInfo[i].iTestPosition)
				{

					DWORD code;
					bool res = GetExitCodeThread(GlobalTestInfo[i].pTestThreadEntry, &code);
					if (!res && code==STILL_ACTIVE)//线程还活着	如果pWinTh 已经结束了，code的值为线程的退出码或返回值
					{
						TerminateThread(GlobalTestInfo[i].pTestThreadEntry,NULL);
					}
				
					GlobalTestInfo.erase(iter);
					j --;
					
				}
			}
			
		}

		for (i = 0; i < length; i ++)
		{
			if (GlobalTestInfo[i].bTestResult)
			{

			}
		}
	}


	return 0;
}

UINT TestEntry(LPVOID pParam)
{

	return 0;
}


#if 0
/**************************
mPic: 
iColor:
iRStep:行步数
iCStep:列步数
iLineWide:线宽

**************************/
bool ImageSegmentation(Mat* mPic,int iColor,int iRStep,int iCStep,int iLineWide)
{
	if (iRStep > 4 || iCStep >4 || iLineWide < 1)
	{
		return false;
	}

	if (mPic->empty())
	{
		return false;
	}

	
	int rows,cols,channels;
	int iTotal,i,j;
	int icstep,irstep;			

	ptrdiff_t mapstep = NULL;
	
	channels = mPic->channels();
	irstep = iRStep;
	icstep = iCStep * channels;
	rows = mPic->rows;
	cols = mPic->cols;

	if (channels < iColor)
	{
		return false;
	}

	Mat dst(rows,cols, CV_8U, Scalar(0));
	iTotal = (cols - 2) * channels;
	mapstep = cols * channels;

	Mat dx(rows, cols, CV_16SC(1));
	Mat dy(rows, cols, CV_16SC(1));


	for ( i = 0;i < rows;)
	{
		uchar * p = mPic->ptr<uchar>(i);
		for (j = channels ;j < iTotal; )
		{


			j += icstep;
		}
		i += irstep;
	}



	return true;
}
#endif
int fLoadConfig(sTestPicConfig& TestConfig)
{
#define MAX_LEN 200
	char Path[200];
	CString Configfile;
	CString ConfigInfo;
//	HMODULE h = NULL;
//	::GetComputerName(tempcomputer.GetBuffer(1024),&nSize);
//	tempcomputer.ReleaseBuffer();
//	computer.Format("%s",tempcomputer);
//	h=LoadLibrary("SajetConnect.dll");

	GetCurrentDirectory(sizeof(Path),Path);
	Configfile.Format("%s\\Config.ini",Path);

//	WritePrivateProfileString("Basic Info","AutoScanControl",strAutoScanControl,strIniFile);
//	GetPrivateProfileString("Basic Info","PassTimes",NULL,ConfigInfo.GetBuffer(MAX_LEN),MAX_LEN,Configfile);
	TestConfig.iTargetPixMinLength  =GetPrivateProfileInt("Basic Info","TargetPixMinLength",0,Configfile);
	TestConfig.iBackgroundMinLength =GetPrivateProfileInt("Basic Info","BlackgroundMinLength",0,Configfile);
	TestConfig.iBackgroundDeviation =GetPrivateProfileInt("Basic Info","BackgroundDeviation",0,Configfile);
	TestConfig.iTargetMinLength     =GetPrivateProfileInt("Basic Info","TargetMinLength",0,Configfile);
	TestConfig.iTargetDeviation     =GetPrivateProfileInt("Basic Info","TargetDeviation",0,Configfile);

	TestConfig.iFont1MinLength       =GetPrivateProfileInt("Basic Info","Font1MinLength",0,Configfile);
	TestConfig.iFont1BandMinWidth    =GetPrivateProfileInt("Basic Info","Font1BandMinWidth",0,Configfile);
	TestConfig.iFont1Deviation       =GetPrivateProfileInt("Basic Info","Font1Deviation",0,Configfile);
	TestConfig.iFont1SkipMaxWidth    =GetPrivateProfileInt("Basic Info","Font1SkipMaxWidth",0,Configfile);
	TestConfig.iFont1Spec            =GetPrivateProfileInt("Basic Info","Font1Spec",0,Configfile);
	TestConfig.iFont1MinWidth        =GetPrivateProfileInt("Basic Info","Font1MinWidth",0,Configfile);

	TestConfig.iFont2MinLength       =GetPrivateProfileInt("Basic Info","Font2MinLength",0,Configfile);
	TestConfig.iFont2BandMinWidth    =GetPrivateProfileInt("Basic Info","Font2BandMinWidth",0,Configfile);
	TestConfig.iFont2Deviation       =GetPrivateProfileInt("Basic Info","Font2Deviation",0,Configfile);
	TestConfig.iFont2SkipMaxWidth    =GetPrivateProfileInt("Basic Info","Font2SkipMaxWidth",0,Configfile);
	TestConfig.iFont2Spec            =GetPrivateProfileInt("Basic Info","Font2Spec",0,Configfile);
	TestConfig.iFont2MinWidth        =GetPrivateProfileInt("Basic Info","Font2MinWidth",0,Configfile);

	TestConfig.iSpotMinLength       =GetPrivateProfileInt("Basic Info","SpotMinLength",0,Configfile);
	TestConfig.iSpotMinWidth        =GetPrivateProfileInt("Basic Info","SpotMinWidth",0,Configfile);
	TestConfig.iSpotDeviation       =GetPrivateProfileInt("Basic Info","SpotDeviation",0,Configfile);

	TestConfig.iBandFirstMinWidth   =GetPrivateProfileInt("Basic Info","BandFirstMinWidth",0,Configfile);
	TestConfig.iBandSecondMinWidth  =GetPrivateProfileInt("Basic Info","BandSecondMinWidth",0,Configfile);
	TestConfig.iBandThirdMinWidth  =GetPrivateProfileInt("Basic Info","BandThirdMinWidth",0,Configfile);


	TestConfig.iSpotSpec            =GetPrivateProfileInt("Basic Info","SpotSpec",0,Configfile);
	TestConfig.iTargetSpec1         =GetPrivateProfileInt("Basic Info","TargetSpec1",0,Configfile);
	TestConfig.iTargetSpec2         =GetPrivateProfileInt("Basic Info","TargetSpec2",0,Configfile);
	
	TestConfig.iBackgroundSpec      =GetPrivateProfileInt("Basic Info","BackgroundSpec",0,Configfile);
	TestConfig.iTargetTopBottomMinPlace=GetPrivateProfileInt("Basic Info","TargetTopBottomMinPlace",0,Configfile);
	TestConfig.iTargetLeftRightMinPlace=GetPrivateProfileInt("Basic Info","TargetLeftRightMinPlace",0,Configfile);
	return 0;
}

int FindPrepare(Mat * mPic,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
	uchar ** pFont1 = NULL;
}
int FindEdgeLocation(Mat * mPic,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{

}
/*
	0 OK
	1 图片错误
	2 没有指出图片底部
	3 超过一定范围未找到目标
	4 没有完整的目标
*/

int fTargetLocation(Mat * mPic,int iBottom,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{

	if (mPic == NULL || mPic->empty())
	{
		return 1;
	}
	if (iBottom < 1 || iBottom > 4)
	{
		return 2;
	}
	
	
//	CV_Assert( mPic->depth() == CV_8U );
	int depth = mPic->depth();
	int rows,cols,channels;
	int i,j;
	ULONG uSum,uSumSkip;
//	int (* aPosition)[2];
	int span;
	bool bTarget;
	int state,iPixState;
	int iTargetPixMinLength = 300 * 3;
	int iBlackgroundLength = 10,iBlackgroundDeviation = 3;
	int iTargetLength = 100,iTargetDeviation = 5;
	int iFontLength = 30,iFontDeviation = 3;
	int iSpotLength = 10,iSpotDeviation = 3;
	int iTargetPixNum = 0;
	int iBackgroundPixNum = 0;
	int iTargetMaxPlace = 90;
	int iTargetTopBottomMinPlace;
	rows = mPic->rows;
	cols = mPic->cols;
	channels = mPic->channels();
	vector<int> vRowInfo;
	vector<int> vRowChange;
	ULONG uTargetMeasureLength = 0;
	ULONG uTargetMeasureNum = 0;
	ULONG uTargetMeasureDNum = 0;
	bool bTargetFind = false;
	uchar * ptr = NULL;
//	aPosition = new int[rows][2];
//	AutoBuffer<uchar> buffer;

	state = 0;
	uSum = cols * channels;
	uSumSkip = (cols) * channels;
	uTargetMeasureNum = 0;
	iTargetTopBottomMinPlace = rows - TestConfig.iTargetTopBottomMinPlace;
	for(i = rows - 1;i > 0; i--)
	{
		ptr = mPic->ptr<uchar>(i);
		bTarget = false;

		
SwitchRestart:
		switch (state)
		{
			case 0:   //初始状态寻找边界白条
				{
					vRowChange.clear();
					vRowInfo.clear();

					if(i > TestConfig.iTargetTopBottomMinPlace)
					{
						iTargetPixNum = 0;
						iBackgroundPixNum = 0;
						
						for (j = 0;j < uSumSkip; )
						{							 
							if ( ptr[j + 2] > TestConfig.iTargetSpec1 )  //可能目标出现
							{
								iTargetPixNum ++;
								iBackgroundPixNum = 0;
								if(iTargetPixNum > TestConfig.iTargetDeviation)  //连续出现多少的目标点后认为OK
								{
									if (vRowInfo.size() == 0)
									{
										vRowInfo.push_back(1);
										vRowChange.push_back(j/3);
									}else if (vRowInfo.back() != 1)
									{
										vRowInfo.push_back(1);
										vRowChange.push_back(j/3 - iTargetPixNum - 4); //前一个背景坐标
										vRowChange.push_back(j/3 - iTargetPixNum - 1); //目标点坐标
									}
								}
							}else
							{
								iBackgroundPixNum ++;
								iTargetPixNum = 0;
								if(iBackgroundPixNum > TestConfig.iBackgroundDeviation)
								{
									if (vRowInfo.size() == 0)
									{
										vRowInfo.push_back(0);
										vRowChange.push_back(j/3);
									}else if (vRowInfo.back() != 0)
									{
										vRowInfo.push_back(0);
										vRowChange.push_back(j/3 - iBackgroundPixNum - 3);
										vRowChange.push_back(j/3 - iBackgroundPixNum - 1);
									}
								}
							}
							j =j + 3;
						}
						vRowChange.push_back(j/3);
						if (vRowInfo.size() < 2)
						{
							state = 0;
							goto SwitchEnd;
						}
						
						bTargetFind = false;
						uTargetMeasureLength = 0;
						for (j = 0;j < vRowInfo.size();j ++)
						{
							uTargetMeasureLength = vRowChange[j*2 + 1] - vRowChange[j*2];
							if(vRowInfo[j] == 1 && uTargetMeasureLength > TestConfig.iTargetMinLength)
							{
								bTargetFind = true;
								uTargetMeasureNum ++;
								TestResult.iEdgeLeftPosition = vRowChange[j*2];
								TestResult.iEdgeRightPosition = vRowChange[j*2 + 1];
								TestResult.iEdgeLength = uTargetMeasureLength;
								if (uTargetMeasureNum > TestConfig.iBandFirstMinWidth)
								{
									TestResult.iEdgePosition = i + uTargetMeasureNum - 1;
									state = 1;
									uTargetMeasureNum = 0;
									uTargetMeasureDNum = 0;
									goto SwitchEnd;
								}
								
							}
						}
						if (!bTargetFind) //当出现有一行不是目标点则重新开始
						{
							uTargetMeasureNum = 0;
						}
						state = 0;
						goto SwitchEnd;
						
					}else
					{
						return 3;
					}
					
				}
			case 1:   //寻找点
				{
					vRowChange.clear();
					vRowInfo.clear();
#if 1            //因为右边缘点在确认完点后已经足够大。所以无需再次扩展确认
					for(j = TestResult.iEdgeRightPosition * 3; j < uSum;) //寻找右边缘点
					{
						if(ptr[j + 2] < TestConfig.iBackgroundSpec)
						{
							break;
						}
						j = j + 3;
					}
					TestResult.iEdgeRightPosition = j/3 - 10;
#endif
					for(j = TestResult.iEdgeLeftPosition * 3;j > 0;)      //寻找左边缘点
					{
						if(ptr[j + 2] < TestConfig.iBackgroundSpec)
						{
							break;
						}
						j = j - 3;
					}
					TestResult.iEdgeLeftPosition = j/3 + 10;
					
					uSumSkip = TestResult.iEdgeRightPosition * 3;
					iTargetPixNum = 0;
					iBackgroundPixNum = 0;
					for (j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
					{							 
						if ( ptr[j + 2] < TestConfig.iSpotSpec )    //点出现
						{
							iTargetPixNum ++;
							iBackgroundPixNum = 0;
							if(iTargetPixNum > TestConfig.iSpotDeviation)
							{
								if (vRowInfo.size() == 0)
								{
									vRowInfo.push_back(1);
									vRowChange.push_back(j/3);
								}else if (vRowInfo.back() != 1)
								{
									vRowInfo.push_back(1);
									vRowChange.push_back(j/3 - iTargetPixNum - 3);
									vRowChange.push_back(j/3 - iTargetPixNum - 1);
								}
							}
						}else
						{
							iBackgroundPixNum ++;
							iTargetPixNum = 0;
							if(iBackgroundPixNum > TestConfig.iSpotDeviation)
							{
								if (vRowInfo.size() == 0)
								{
									vRowInfo.push_back(0);
									vRowChange.push_back(j/3);
								}else if (vRowInfo.back() != 0)
								{
									vRowInfo.push_back(0);
									vRowChange.push_back(j/3 - iBackgroundPixNum - 3);
									vRowChange.push_back(j/3 - iBackgroundPixNum - 1);
								}
							}
						}
						j =j + 3;
					}
					vRowChange.push_back(TestResult.iEdgeRightPosition);

					if (vRowInfo.size() < 2)
					{
						state = 1;
						goto SwitchEnd;
					}

					uTargetMeasureLength = 0;
					bTargetFind = false;
					bool bTargetFind2 = false;
					for (j = 0;j < vRowInfo.size();j ++)
					{
						uTargetMeasureLength = vRowChange[j*2 + 1] - vRowChange[j*2];
						if(!bTargetFind && vRowInfo[j] == 1 && uTargetMeasureLength > TestConfig.iSpotMinLength)
						{
							bTargetFind = true;
							
							TestResult.iSpotFirstLeftPosition = TestResult.iSpotFirstLeftPosition < vRowChange[j*2]? TestResult.iSpotFirstLeftPosition:vRowChange[j*2];
							TestResult.iSpotFirstRightPosition = TestResult.iSpotFirstRightPosition > vRowChange[j*2 + 1]? TestResult.iSpotFirstRightPosition:vRowChange[j*2+1];
							continue;
						}
						if (bTargetFind&&!bTargetFind2 && vRowInfo[j] == 1 && uTargetMeasureLength > TestConfig.iSpotMinLength)
						{
							 bTargetFind2 = true;
							 TestResult.iSpotSecondLeftPosition = TestResult.iSpotSecondLeftPosition < vRowChange[j*2]? TestResult.iSpotSecondLeftPosition:vRowChange[j*2];
							 TestResult.iSpotSecondRightPosition = TestResult.iSpotSecondRightPosition > vRowChange[j*2 + 1]? TestResult.iSpotSecondRightPosition:vRowChange[j*2+1];
							 continue;
						}
		
					}
					if (bTargetFind&&bTargetFind2)
					{
						uTargetMeasureNum ++;
						uTargetMeasureDNum = 0;
						if (uTargetMeasureNum > TestConfig.iSpotMinWidth)
						{
							bTargetFind =false;
							TestResult.iSpotPosition = i + uTargetMeasureNum;
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 2;
							goto SwitchEnd;
						}
					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > TestConfig.iSpotDeviation)
						{
							uTargetMeasureNum = 0;
						}
						
					}
					state = 1;
					goto SwitchEnd;
				}
			case 2://寻找字1下边缘
				{

					uTargetMeasureLength = 0;
					
					for(j = TestResult.iEdgeLeftPosition * 3;j > 0;)      //寻找左边缘点
					{
						if(ptr[j + 2] < TestConfig.iBackgroundSpec)
						{
							break;
						}
						j = j - 3;
					}
					TestResult.iEdgeLeftPosition = j/3 + 5;

					uSumSkip = TestResult.iEdgeRightPosition * 3;
					for ( ;j < uSumSkip; )
					{
						if(ptr[j + 2] > TestConfig.iFont1Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}
					if(uTargetMeasureLength > TestResult.iEdgeRightPosition - TestResult.iEdgeLeftPosition - 30)
					{
						uTargetMeasureDNum = 0;
						uTargetMeasureNum ++;
						if (uTargetMeasureNum > TestConfig.iBandSecondMinWidth)
						{
							TestResult.iFont1BottomPosition = i;
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 3;
							goto SwitchEnd;

						}
					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > 3)
						{
							uTargetMeasureNum = 0;
						}
						
					}
					
					state = 2;
					goto SwitchEnd;

				}
			case 3: //寻找字1区域
				{
					uTargetMeasureLength = 0;
					uSumSkip = TestResult.iEdgeRightPosition * 3;
					for ( j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
					{
						if(ptr[j + 2] < TestConfig.iFont1Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}
					if(uTargetMeasureLength  > 10)
					{

						uTargetMeasureNum ++;
						uTargetMeasureDNum = 0;
						if (uTargetMeasureNum > TestConfig.iFont1MinWidth)
						{
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 4;
							goto SwitchEnd;
						}

					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > TestConfig.iFont1Deviation)
						{
							uTargetMeasureNum = 0;
						}
					}
					state = 3;
					goto SwitchEnd;
				}
			case 4:  //定位字1上边缘
				{
					for(j = TestResult.iEdgeLeftPosition * 3;j > 0;)      //寻找左边缘点
					{
						if(ptr[j + 2] < TestConfig.iBackgroundSpec)
						{
							break;
						}
						j = j - 3;
					}
					TestResult.iEdgeLeftPosition = j/3 + 5;

					TestResult.iEdgeRightPosition = TestResult.iSpotSecondRightPosition;

					j = TestResult.iEdgeLeftPosition * 3;
					for ( ;j < uSumSkip; )
					{
						if(ptr[j + 2] < TestConfig.iFont1Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}
					if(uTargetMeasureLength > TestResult.iEdgeRightPosition - TestResult.iEdgeLeftPosition - 30)
					{
						uTargetMeasureDNum = 0;
						uTargetMeasureNum ++;
						if (uTargetMeasureNum > TestConfig.iBandThirdMinWidth)
						{
							TestResult.iFont1TopPosition = i;
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 5;
							goto SwitchEnd;
						}
						
					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > 3)
						{
							uTargetMeasureNum = 0;
						}
					}

					state = 4;
					goto SwitchEnd;
					

				}
			case 5:   //确定字1区间
				{
					int k = 0;
					uchar * ptr1 = NULL;
					bool bT,bB;
					uTargetMeasureLength = 0;
					k = TestResult.iFont1BottomPosition;
					bT = false;
					bB = false;
					for (;k > TestResult.iFont1TopPosition;k --)
					{
						uTargetMeasureLength = 0;
						ptr1 = mPic->ptr<uchar>(k);
						for ( j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
						{
							if(ptr1[j + 2] < TestConfig.iFont1Spec)
							{
								uTargetMeasureLength ++;
							}
							j = j + 3;
						}
						if(uTargetMeasureLength  > 8)
						{
							uTargetMeasureNum ++;
							uTargetMeasureDNum = 0;
							if (uTargetMeasureNum > TestConfig.iFont1SkipMaxWidth/3)
							{
								uTargetMeasureNum = 0;
								bB = true;
								TestResult.iFont1BottomPosition = k + uTargetMeasureNum;
								break;
							}
						}else
						{
							uTargetMeasureDNum ++;
						}
						
						if (uTargetMeasureDNum > TestConfig.iFont1Deviation)
						{
							uTargetMeasureNum = 0;
						}
					}

					if (!bB)
					{
						return 9;
					}
					uTargetMeasureLength = 0;
					k = TestResult.iFont1TopPosition;
					for (;k < TestResult.iFont1BottomPosition;k ++)
					{
						uTargetMeasureLength = 0;
						ptr1 = mPic->ptr<uchar>(k);
						for ( j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
						{
							if(ptr1[j + 2] < TestConfig.iFont1Spec)
							{
								uTargetMeasureLength ++;
							}
							j = j + 3;
						}
						if(uTargetMeasureLength  > 8)
						{
							uTargetMeasureNum ++;
							uTargetMeasureDNum = 0;
							if (uTargetMeasureNum > TestConfig.iFont1SkipMaxWidth/3)
							{
								uTargetMeasureNum = 0;
								bT = true;
								TestResult.iFont1TopPosition = k - uTargetMeasureNum;
								break;
							}
						}else
						{
							uTargetMeasureDNum ++;
						}

						if (uTargetMeasureDNum > TestConfig.iFont1Deviation)
						{
							uTargetMeasureNum = 0;
						}
					}
					
					if (bT)
					{
						state = 6;
						goto SwitchEnd;
					}else
					{
						return 9;
					}
					

				}
			case 6:   //寻找字2下边缘2
				{
					uTargetMeasureLength = 0;
					for(j = TestResult.iEdgeLeftPosition * 3;j > 0;)      //寻找左边缘点
					{
						if(ptr[j + 2] < TestConfig.iBackgroundSpec)
						{
							break;
						}
						j = j - 3;
					}
					TestResult.iEdgeLeftPosition = j/3 + 5;

					j = TestResult.iEdgeLeftPosition * 3;
					uSumSkip = TestResult.iEdgeRightPosition * 3;
					for ( ;j < uSumSkip; )
					{
						if(ptr[j + 2] > TestConfig.iFont2Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}

					if(uTargetMeasureLength >TestResult.iEdgeRightPosition - TestResult.iEdgeLeftPosition - 10)
					{
						uTargetMeasureDNum = 0;
						uTargetMeasureNum ++;
						if (uTargetMeasureNum > TestConfig.iBandThirdMinWidth/3)
						{
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 7;
							goto SwitchEnd;

						}
					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > 3)
						{
							uTargetMeasureNum = 0;
						}
					}

					state = 6;
					goto SwitchEnd;
				}
			case 7:   //寻找字2区间
				{
					uTargetMeasureLength = 0;
					uSumSkip = TestResult.iEdgeRightPosition * 3;
					for ( j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
					{
						if(ptr[j + 2] < TestConfig.iFont2Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}
 					if(uTargetMeasureLength  > 10)
					{
						uTargetMeasureNum ++;
						uTargetMeasureDNum = 0;
						if (uTargetMeasureNum > TestConfig.iFont2MinWidth)
						{
							uTargetMeasureNum = 0;
							uTargetMeasureDNum = 0;
							state = 8;
							goto SwitchEnd;
						}

					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > TestConfig.iFont2Deviation)
						{
							uTargetMeasureNum = 0;
						}
					}
					state = 7;
					goto SwitchEnd;
				}
			case 8:   //寻找字上边缘2
				{
					
					uSumSkip = TestResult.iEdgeRightPosition * 3;
					for ( j = TestResult.iEdgeLeftPosition * 3;j < uSumSkip; )
					{
						if(ptr[j + 2] > TestConfig.iFont2Spec)
						{
							uTargetMeasureLength ++;
						}
						j = j + 3;
					}
					if(uTargetMeasureLength > TestResult.iEdgeRightPosition - TestResult.iEdgeLeftPosition - 10)
					{
						uTargetMeasureNum ++;
						uTargetMeasureDNum = 0;
						if (uTargetMeasureNum > 6)
						{
							TestResult.iFont2TopPosition = i + 2;
							state = 9;
							goto SwitchEnd;
						}
						
					}else
					{
						uTargetMeasureDNum ++;
						if (uTargetMeasureDNum > 3 )
						{
							uTargetMeasureDNum = 0;
						}
						state = 8;
						goto SwitchEnd;
					}

				}
			case 9:   //处理字
				{
					int iCheckNum;
					int iFontSkip;
					uchar ** pFont1 = NULL,**pFont2 = NULL;
					int k,iFontPix;
					int iBandwidth = 5,iBandWidthTmp = 0;
					ULONG uEndPosition;

					iFontSkip = TestResult.iFont1BottomPosition - TestResult.iFont1TopPosition;

					pFont1 = new uchar *[iFontSkip];
					for(j = 0;j < iFontSkip;j++)
					{
						pFont1[j] =  mPic->ptr<uchar>(TestResult.iFont1TopPosition + j);
					}
				
					//查找字前白色区间的位置
					uTargetMeasureLength = 0;
					uTargetMeasureNum = 0;
					uEndPosition = (TestResult.iEdgeRightPosition - 30) * 3;
					for(k = (TestResult.iEdgeLeftPosition + 30)*3;k< uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont1[j][k] < TestConfig.iFont1Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if(uTargetMeasureLength > 4)
						{
							uTargetMeasureNum ++;
							if (uTargetMeasureNum > TestConfig.iFont1MinLength/3)
							{
								TestResult.iFont1BeginPosition = k/3 ;
								break; 
							}
							     //找到开始位置
						}else
						{
							uTargetMeasureNum = 0;
						}
						k = k + 3;
					}

					//查找字的结束位置
					uTargetMeasureLength = 0;
					uTargetMeasureNum = 0;
					uEndPosition = TestResult.iFont1BeginPosition*3;
					for(k = (TestResult.iEdgeRightPosition - 30) * 3;k > uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont1[j][k] < TestConfig.iFont1Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if(uTargetMeasureLength > 4)
						{
							uTargetMeasureNum ++;
							if (uTargetMeasureNum > TestConfig.iFont1MinLength/3)
							{
								TestResult.iFont1EndPosition = k/3 ;
								break; 
							}
							     //找到结束位置
						}else
						{
							uTargetMeasureNum = 0;
						}
						k = k - 3;
					}


					uTargetMeasureNum = 0;
					uTargetMeasureDNum = 0;
					iCheckNum = 0;
					bTargetFind = FALSE;
					uEndPosition = TestResult.iFont1EndPosition * 3;
					for (k = TestResult.iFont1BeginPosition * 3;k < uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont1[j][k] > TestConfig.iFont1Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if (bTargetFind)//uTargetMeasureLength 白色个数
						{
							if (uTargetMeasureLength > iFontSkip - 2)
							{
								uTargetMeasureNum ++;
								uTargetMeasureDNum = 0;
								if (uTargetMeasureNum > TestConfig.iFont1BandMinWidth/2)
								{
									bTargetFind = true;
									uTargetMeasureNum = 0;
									iCheckNum ++;

								}
							}else
							{
								uTargetMeasureDNum ++;
								if (uTargetMeasureDNum > 2)
								{
									uTargetMeasureNum = 0;
								}
							}
						}else
						{
							if (uTargetMeasureLength < iFontSkip - 2)
							{
								uTargetMeasureNum ++;
								uTargetMeasureDNum = 0;
								if (uTargetMeasureNum > TestConfig.iFont1MinLength/3)
								{
									bTargetFind = TRUE;
									uTargetMeasureNum = 0;

								}
							}
							else
							{
								uTargetMeasureDNum ++;
								if (uTargetMeasureDNum > 2)
								{
									uTargetMeasureNum = 0;
								}
							}
						}
						
						k = k + 3;
					}
					if (iCheckNum != 7)
					{
						return 9;
					}
					delete pFont1;
					pFont1 = NULL;


					
					iFontSkip = TestResult.iFont2BottomPosition - TestResult.iFont2TopPosition;
					pFont2 = new uchar *[iFontSkip];
					for(j = 0;j < iFontSkip;j++)
					{
						pFont2[j] =  mPic->ptr<uchar>(TestResult.iFont2TopPosition + j);
					}

					//查找字前白色区间的位置
					uTargetMeasureLength = 0;
					uTargetMeasureNum = 0;
					uEndPosition = (TestResult.iEdgeRightPosition - 30) * 3;
					for(k = (TestResult.iEdgeLeftPosition + 30)*3;k< uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont2[j][k] < TestConfig.iFont2Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if(uTargetMeasureLength > 4)
						{
							uTargetMeasureNum ++;
							if (uTargetMeasureNum > TestConfig.iFont2MinLength/3)
							{
								TestResult.iFont2BeginPosition = k/3 ;
								break; 
							}
							     //找到开始位置
						}else
						{
							uTargetMeasureNum = 0;
						}
						k = k + 3;
					}

					//查找字的结束位置
					uTargetMeasureLength = 0;
					uTargetMeasureNum = 0;
					uEndPosition = TestResult.iFont1BeginPosition*3;
					for(k = (TestResult.iEdgeRightPosition - 30) * 3;k > uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont2[j][k] < TestConfig.iFont2Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if(uTargetMeasureLength > 4)
						{
							uTargetMeasureNum ++;
							if (uTargetMeasureNum > TestConfig.iFont2MinLength/3)
							{
								TestResult.iFont2EndPosition = k/3 ;
								break; 
							}
							     //找到结束位置
						}else
						{
							uTargetMeasureNum = 0;
						}
						k = k - 3;
					}

					uTargetMeasureNum = 0;
					uTargetMeasureDNum = 0;
					iCheckNum = 0;
					bTargetFind = FALSE;
					uEndPosition = TestResult.iFont2EndPosition * 3;
					for (k = TestResult.iFont2BeginPosition * 3;k < uEndPosition;)
					{
						uTargetMeasureLength = 0;
						for(j = 0;j < iFontSkip;j++)
						{
							if(pFont2[j][k] > TestConfig.iFont2Spec)
							{
								uTargetMeasureLength ++;
							}
						}
						if (bTargetFind)//uTargetMeasureLength 白色个数
						{
							if (uTargetMeasureLength > iFontSkip - 2)
							{
								uTargetMeasureNum ++;
								uTargetMeasureDNum = 0;
								if (uTargetMeasureNum > TestConfig.iFont2BandMinWidth/2)
								{
									bTargetFind = true;
									uTargetMeasureNum = 0;
									iCheckNum ++;

								}
							}else
							{
								uTargetMeasureDNum ++;
								if (uTargetMeasureDNum > 2)
								{
									uTargetMeasureNum = 0;
								}
							}
						}else
						{
							if (uTargetMeasureLength < iFontSkip - 2)
							{
								uTargetMeasureNum ++;
								uTargetMeasureDNum = 0;
								if (uTargetMeasureNum > TestConfig.iFont2MinLength/3)
								{
									bTargetFind = TRUE;
									uTargetMeasureNum = 0;

								}
							}
							else
							{
								uTargetMeasureDNum ++;
								if (uTargetMeasureDNum > 2)
								{
									uTargetMeasureNum = 0;
								}
							}
						}
						
						k = k + 3;
					}

					delete pFont2;
					pFont2 = NULL;
					if (iCheckNum != 4)
					{
						return 10;
					}else
					{
						return 0;
					}

#if 0
					k = k - 4 * 3;
					int result = 1;
					//check A1
					result = checkA1(pFont,iFontSkip,k,TestResult,TestConfig);
					if(result != 0)
					{
						return result;
					}
					//check R
					result = checkR(pFont,iFontSkip,k,TestResult,TestConfig);
					if(result != 0)
					{
						return result;
					}
					//check U
					result = checkU(pFont,iFontSkip,k,TestResult,TestConfig);
					if(result != 0)
					{
						return result;
					}
					//check B
					result = checkB(pFont,iFontSkip,k,TestResult,TestConfig);
					if(result != 0)
					{
						return result;
					}
					//check A2
					result = checkA2(pFont,iFontSkip,k,TestResult,TestConfig);
					if(result != 0)
					{
						return result;
					}
#endif
				}
			default:
				{
				}
			}
SwitchEnd:
		continue;
	}

//	imwrite("a.bmp",*mPic);
	return 0;
}
int splitcharacter(uchar**pFont,int iFontSkip,int &k,vector<vector<int>>& iFontChange,sTestPicConfig& TestConfig,int & iEdgeRightPosition)
{
	vector<int> vFontCols;
	vector<int> iFontFind;
	int iFontSkipMid;
	int* pFontDe1; //字像素出现次数
	int* pFontDe2; //背景像素出现次数
	int iBackgroundPix,iBandWidthTmp,j;

	if(pFont == NULL)
		return 7;
	pFontDe1 = new int[iFontSkip];
	if(pFontDe1 == NULL)
	{
		return 8;
	}
	pFontDe2 = new int[iFontSkip];
	if(pFontDe2 == NULL)
	{
		return 8;
	}
	for(j = 0;j < iFontSkip;j++)
	{
		iFontFind.push_back(1);//奇数 -- 背景
		vFontCols.clear();
		vFontCols.push_back(k);
		iFontChange.push_back(vFontCols);
	}
	iFontSkipMid = iFontSkip >> 1;
	iBandWidthTmp = 0;
	
	for(;k < iEdgeRightPosition;)   //遍历并找到一个字母
	{
		for(j = 0;j < iFontSkip; j++)
		{
			
			iBackgroundPix = 0;
			if(pFont[j][k] > TestConfig.iFont2Spec)
			{
				iBackgroundPix ++;
				pFontDe2[j] ++;
				if(pFontDe2[j] > 4)
				{
					pFontDe1[j] = 0;
					if(iFontFind[j]%2 == 0)
					{
						iFontFind[j] ++;
						iFontChange[j].push_back(k - pFontDe2[j]);
					}
				}
			}else
			{
				pFontDe1[j] ++;
				if(pFontDe1[j] > 4)
				{
					pFontDe2[j] = 0;
					if(iFontFind[j]%2 != 0)
					{
						iFontFind[j] ++;
						iFontChange[j].push_back(k - pFontDe1[j]);
					}
				}
			}
		}
		if(iBackgroundPix >= iFontSkip && iFontFind[iFontSkipMid] != 1)
		{
			iBandWidthTmp ++;
			if(iBandWidthTmp > TestConfig.iFont2BandMinWidth)
			{
				iBackgroundPix = k - iBandWidthTmp + 2;
				for(j = 0;j < iFontSkip; j++)
					iFontChange[j].push_back(iBackgroundPix);
				break;      //找到一个字母结束
			}
		}
		k = k + 3;
	}

	return 0;
}
int checkA1(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
	
	int checkstate;
	int checkNum,j;

	checkstate = splitcharacter(pFont,iFontSkip,k,TestResult.iA1FontChange,TestConfig,TestResult.iEdgeRightPosition);
	if(!checkstate)
	{
		return checkstate;
	}
	checkstate = 1;
	checkNum = 0;
	for(j = 0;j< iFontSkip;j++)
	{
		switch (checkstate)
		{
			case 1:
			{
				if(TestResult.iA1FontChange[j].size() == 6 && TestResult.iA1FontChange[j][3] - TestResult.iA1FontChange[j][2] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 2;
					}
				}
			}
			case 2:
			{
				if(TestResult.iA1FontChange[j].size() == 10 && TestResult.iA1FontChange[j][3] - TestResult.iA1FontChange[j][2] > 4 && TestResult.iA1FontChange[j][7] - TestResult.iA1FontChange[j][6] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 3;
					}
				}
			}
			case 3:
			{
				if(TestResult.iA1FontChange[j].size() == 6 && TestResult.iA1FontChange[j][3] - TestResult.iA1FontChange[j][2] > 6)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 4;
					}
				}
			}
			case 4:
				{
					break; //check A1 ok
				}
			default:
			{
				return 7;
			}
		}
	}
	return 0;
}
int checkR(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
	int checkstate;
	int checkNum,j;

	checkstate = splitcharacter(pFont,iFontSkip,k,TestResult.iRFontChange,TestConfig,TestResult.iEdgeRightPosition);
	if(!checkstate)
	{
		return checkstate;
	}

	checkstate = 1;
	checkNum = 0;
	for(j = 0;j< iFontSkip;j++)
	{
		switch (checkstate)
		{
			case 1:
			{
				if(TestResult.iRFontChange[j].size() == 6 && TestResult.iRFontChange[j][3] - TestResult.iRFontChange[j][2] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 2;
					}
				}
			}
			case 2:
			{
				if(TestResult.iRFontChange[j].size() == 10 && TestResult.iRFontChange[j][3] - TestResult.iRFontChange[j][2] > 1 && TestResult.iRFontChange[j][7] - TestResult.iRFontChange[j][6] > 10)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 3;
					}
				}
			}
			case 3:
			{
				if(TestResult.iRFontChange[j].size() == 6 && TestResult.iRFontChange[j][3] - TestResult.iRFontChange[j][2] > 14)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 4;
					}
				}
			}
			case 4:
			{
				if(TestResult.iRFontChange[j].size() == 6 && TestResult.iRFontChange[j][3] - TestResult.iRFontChange[j][2] > 6)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 5;
					}
				}
			}
			case 5:
				{
					break; //check R ok
				}
			default:
			{
				return 7;
			}
		}
	}
	return 0;
}
int checkU(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
	int checkstate;
	int checkNum,j;

	checkstate = splitcharacter(pFont,iFontSkip,k,TestResult.iUFontChange,TestConfig,TestResult.iEdgeRightPosition);
	if(!checkstate)
	{
		return checkstate;
	}
	checkstate = 1;
	checkNum = 0;
	for(j = 0;j< iFontSkip;j++)
	{
		switch (checkstate)
		{
			case 1:
			{
				if(TestResult.iUFontChange[j].size() == 10 && TestResult.iUFontChange[j][3] - TestResult.iUFontChange[j][2] > 4 && TestResult.iUFontChange[j][7] - TestResult.iUFontChange[j][6] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 2;
					}
				}
			}
			case 2:
			{
				if(TestResult.iUFontChange[j].size() == 6 && TestResult.iUFontChange[j][3] - TestResult.iUFontChange[j][2] > 6)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 3;
					}
				}
			}
			case 3:
				{
					break; //check U ok
				}
			default:
			{
				return 7;
			}
		}
	}
	return 0;
}
int checkB(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
		
	int checkstate;
	int checkNum,j;

	checkstate = splitcharacter(pFont,iFontSkip,k,TestResult.iBFontChange,TestConfig,TestResult.iEdgeRightPosition);
	if(!checkstate)
	{
		return checkstate;
	}
	checkstate = 1;
	checkNum = 0;
	for(j = 0;j< iFontSkip;j++)
	{
		switch (checkstate)
		{
			case 1:
			{
				if(TestResult.iBFontChange[j].size() == 6 && TestResult.iBFontChange[j][3] - TestResult.iBFontChange[j][2] > 4)
				{
					checkNum ++;
					if(checkNum > 8)
					{
						checkNum = 0;
						checkstate = 2;
					}
				}
			}
			case 2:
			{
				if(TestResult.iBFontChange[j].size() == 10 && TestResult.iBFontChange[j][3] - TestResult.iBFontChange[j][2] > 4 && TestResult.iBFontChange[j][7] - TestResult.iBFontChange[j][6] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 3;
					}
				}
			}
			case 3:
			{
				if(TestResult.iBFontChange[j].size() == 6 && TestResult.iBFontChange[j][3] - TestResult.iBFontChange[j][2] > 6)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 4;
					}
				}
			}
			case 4:
				{
					break; //check B ok
				}
			default:
			{
				return 7;
			}
		}
	}
	return 0;
}
int checkA2(uchar**pFont,int iFontSkip,int &k,sTestPicResult& TestResult,sTestPicConfig& TestConfig)
{
	int checkstate;
	int checkNum,j;

	checkstate = splitcharacter(pFont,iFontSkip,k,TestResult.iA2FontChange,TestConfig,TestResult.iEdgeRightPosition);
	if(!checkstate)
	{
		return checkstate;
	}
	checkstate = 1;
	checkNum = 0;
	for(j = 0;j< iFontSkip;j++)
	{
		switch (checkstate)
		{
			case 1:
			{
				if(TestResult.iA2FontChange[j].size() == 6 && TestResult.iA2FontChange[j][3] - TestResult.iA2FontChange[j][2] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 2;
					}
				}
			}
			case 2:
			{
				if(TestResult.iA2FontChange[j].size() == 10 && TestResult.iA2FontChange[j][3] - TestResult.iA2FontChange[j][2] > 4 && TestResult.iA2FontChange[j][7] - TestResult.iA2FontChange[j][6] > 4)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 3;
					}
				}
			}
			case 3:
			{
				if(TestResult.iA2FontChange[j].size() == 6 && TestResult.iA2FontChange[j][3] - TestResult.iA2FontChange[j][2] > 6)
				{
					checkNum ++;
					if(checkNum > 4)
					{
						checkNum = 0;
						checkstate = 4;
					}
				}
			}
			case 4:
				{
					break; //check A ok
				}
			default:
			{
				return 7;
			}
		}
	}
	return 0;
}



