
// FixedVHDWriterDlg.cpp: 实现文件
//

#include "framework.h"
#include "FixedVHDWriter.h"
#include "FixedVHDWriterDlg.h"
#include "afxdialogex.h"

#include "..\..\writer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFixedVHDWriterDlg 对话框



CFixedVHDWriterDlg::CFixedVHDWriterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FIXEDVHDWRITER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFixedVHDWriterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFixedVHDWriterDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPENSRC, &CFixedVHDWriterDlg::OnBnClickedButtonOpensrc)
	ON_BN_CLICKED(IDC_BUTTON_OPENDST, &CFixedVHDWriterDlg::OnBnClickedButtonOpendst)
	ON_BN_CLICKED(IDC_BUTTON_WRITE, &CFixedVHDWriterDlg::OnBnClickedButtonWrite)
END_MESSAGE_MAP()


// CFixedVHDWriterDlg 消息处理程序

BOOL CFixedVHDWriterDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_SHOW);

	// TODO: 在此添加额外的初始化代码
	SetDlgItemText(IDC_EDIT_BEGSEC, TEXT("0"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFixedVHDWriterDlg::OnPaint()
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
HCURSOR CFixedVHDWriterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFixedVHDWriterDlg::OnBnClickedButtonOpensrc() {
	CFileDialog cfd(
		true, nullptr, nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"All Files (*.*)|*.*||");

	if (cfd.DoModal() == IDOK) {
		SetDlgItemText(IDC_EDIT_FILESRC, cfd.GetPathName());
	}
}


void CFixedVHDWriterDlg::OnBnClickedButtonOpendst() {
	CFileDialog cfd(
		true, nullptr, nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		"Fixed VHD File (*.vhd)|*.vhd|All Files (*.*)|*.*||");

	if (cfd.DoModal() == IDOK) {
		SetDlgItemText(IDC_EDIT_FILEDST, cfd.GetPathName());
	}
}

void CFixedVHDWriterDlg::OnBnClickedButtonWrite() {
	// TODO: 在此添加控件通知处理程序代码
	CString csSrcPath, csDstPath, csBeginSector;
	int nBeginSector;
	int result = MessageBox("您确认要执行写入吗？", "写入文件", MB_OKCANCEL | MB_ICONQUESTION);
	if (result == IDOK) {

		GetDlgItemText(IDC_EDIT_FILESRC, csSrcPath);
		GetDlgItemText(IDC_EDIT_FILEDST, csDstPath);
		GetDlgItemText(IDC_EDIT_BEGSEC, csBeginSector);
		long nBeginSector = _tcstol(csBeginSector.GetString(), nullptr, 10);
		
		struct writer_object wo;
		int64_t data_file_size;

		init_writer_object(&wo, csDstPath.GetString());
		if (get_last_error(&wo) == OPEN_FILE_ERROR) {
			MessageBox("Open VHD image file error", nullptr, MB_ICONERROR);
			return;
		}

		if (!vaild_vhd(&wo)) {
			MessageBox("Invaild or broken VHD image file", nullptr, MB_ICONERROR);
			release_writer_object(&wo);
			return;
		}

		if (!fixed_vhd(&wo)) {
			MessageBox("The VHD image is not fixed which is still not support", nullptr, MB_ICONERROR);
			release_writer_object(&wo);
			return;
		}

		size_vhd(&wo);

		/* read data from the data file, then write it to VHD image file */
		data_file_size = get_file_size_by_name(csSrcPath.GetString());
		if (data_file_size == 0) {
			MessageBox("Data file is invaild", nullptr, MB_ICONERROR);
			release_writer_object(&wo);
			return;
		}

		int64_t total_written_bytes = write_hvd_sector_from_data_file(&wo, nBeginSector, csSrcPath.GetString());
		enum writer_error err = get_last_error(&wo);
		if (err == LBA_OUT_OF_RANGE) {
			CString s; s.Format("LBA is out of range (0 - %l)", wo.size / 512 - 1);
			MessageBox(s.GetString(), nullptr, MB_ICONERROR);
			release_writer_object(&wo);
			return;
		} 	else if (err == OPEN_FILE_ERROR) {
			MessageBox("Open data file error", nullptr, MB_ICONERROR);
			release_writer_object(&wo);
			return;
		}

		CString s; 
		s.Format("Data: %s\nVHD: %s (offset LBA: %l)\nTotal bytes to write: %lld\nTotal sectors to write: %lld\nTotal bytes written: %lld\nTotal sectors written: %lld\n",
			csSrcPath.GetString(), csDstPath.GetString(), nBeginSector, 
			data_file_size, data_file_size / 512 + (data_file_size % 512 != 0), total_written_bytes, total_written_bytes / 512 + (total_written_bytes % 512 != 0));
		MessageBox(s.GetString(), "Info", MB_ICONINFORMATION);

		if (total_written_bytes < data_file_size) {
			MessageBox("\n!!! Detected the tail of VHD image file, the writing data has been truncated!\n", "Info", MB_ICONINFORMATION);
		}

		release_writer_object(&wo);
	}
}
