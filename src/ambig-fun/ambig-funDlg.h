// ambig-funDlg.h : header file
//

#pragma once

#include <util/common/gui/SimulationDialog.h>
#include <util\common\gui\PlotControl.h>

#include "model.h"
#include "afxwin.h"

// CAmbigFunDlg dialog
class CAmbigFunDlg : public CSimulationDialog
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

	CPlotControl m_cSignalsPlot;
	CPlotControl m_cSignalsShiftedPlot;
	CPlotControl m_cCorrelationPlot;
	model::model_data m_data;
public:
    CButton m_cDisplayType;
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    virtual void OnSimulation();
};
