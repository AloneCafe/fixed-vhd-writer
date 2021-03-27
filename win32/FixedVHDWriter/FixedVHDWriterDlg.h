
// FixedVHDWriterDlg.h: 头文件
//

#pragma once

extern int main(int argc, char ** argv);

// CFixedVHDWriterDlg 对话框
class CFixedVHDWriterDlg : public CDialogEx
{
// 构造
public:
	CFixedVHDWriterDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FIXEDVHDWRITER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonOpensrc();
	afx_msg void OnBnClickedButtonOpendst();
	afx_msg void OnBnClickedButtonWrite();
};
