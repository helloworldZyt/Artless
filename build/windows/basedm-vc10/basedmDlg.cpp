
// basedmDlg.cpp : ʵ���ļ�
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

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
CbasedmDlg* g_winhand = NULL;

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CbasedmDlg �Ի���




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


// CbasedmDlg ��Ϣ�������

BOOL CbasedmDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	g_winhand = this;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CbasedmDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
