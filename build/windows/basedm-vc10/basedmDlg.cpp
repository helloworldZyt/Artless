
// basedmDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "basedm.h"
#include "basedmDlg.h"
#include "afxdialogex.h"
#include <string>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
CbasedmDlg* g_winhand = NULL;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CbasedmDlg 对话框




CbasedmDlg::CbasedmDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CbasedmDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CbasedmDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LOGVIEW, CStatic_Logview);
	DDX_Control(pDX, IDC_LOCALVIEW, CStatic_LocalView);
	DDX_Control(pDX, IDC_INPUTI, CEdit_Input0);
	DDX_Control(pDX, IDC_INPUTII, CEdit_Input1);
}

BEGIN_MESSAGE_MAP(CbasedmDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_START, &CbasedmDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_DO_IT, &CbasedmDlg::OnBnClickedDoIt)
END_MESSAGE_MAP()


// CbasedmDlg 消息处理程序

BOOL CbasedmDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	g_winhand = this;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CbasedmDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CbasedmDlg::OnPaint()
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

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CbasedmDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CbasedmDlg::myLogPrint(int valu, const char* info)
{
	static int outcnt = 0;
	if (6<outcnt++)
	{
		outcnt=0;
		CStatic_Logview.SetWindowText(_T(""));
	}
	CString csOldText;
	CString csNewApp;
	CStatic_Logview.GetWindowText(csOldText);
	csNewApp.Format(_T("%d %s\n\r"), valu, info);
	csOldText += csNewApp;
	CStatic_Logview.SetWindowText(csOldText);
	return;

}

#include "utilmanage.h"

#include "IPConnUtil.h"

char* iplist[10] = {NULL};
int ipcnt = 0;
void CbasedmDlg::OnBnClickedStart()
{
	// TODO: 在此添加控件通知处理程序代码
	
	for (int i = 0; i < 10 && (NULL == iplist[i]); i++)
	{
		iplist[i] = (char*)malloc(128);
		memset(iplist[i], 0, 128);
	}
	ipcnt = utilman_entry(MANN_UTIL_API_GET_IPS, NULL, (void*)iplist);

	myLogPrint(ipcnt, "");

	for (int i = 0; i < ipcnt; i++)
	{
		myLogPrint(i, iplist[i]);
	}
	return;
}


void CbasedmDlg::OnBnClickedDoIt()
{
	// TODO: 在此添加控件通知处理程序代码
	CString csDstIp = _T("");
	CEdit_Input0.GetWindowText(csDstIp);
	string strDstip = csDstIp;

	CString csDstPort;
	CEdit_Input1.GetWindowText(csDstPort);
	string strDstPort = csDstPort;

	for (int i = 0; i < ipcnt; i++)
	{
		int contm = PingHost("103.25.23.99", iplist[i]);
		if (500 <= contm)
		{
			myLogPrint(contm, string(iplist[i]+string("to 103.25.23.99 time out!")).c_str());
		}
	}

	for (int i = 0; i < ipcnt; i++)
	{
		string errstr = iputil_test_ip_connectivity("103.25.23.99", 443, iplist[i]);
		myLogPrint(i, errstr.c_str());
	}
}
