// ambig-funDlg.h : header file
//

#pragma once

#include <util/common/gui/SimulationDialog.h>

// CAmbigFunDlg dialog
class CAmbigFunDlg : public CDialogEx
{
// Construction
public:
    CAmbigFunDlg(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    enum { IDD = IDD_AMBIGFUN_DIALOG };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
};
