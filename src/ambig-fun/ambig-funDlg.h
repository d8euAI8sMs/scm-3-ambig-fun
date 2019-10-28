// ambig-funDlg.h : header file
//

#pragma once

#include <util/common/gui/SimulationDialog.h>
#include <util\common\gui\PlotControl.h>

#include "model.h"
#include "SurfacePlotControl.h"
#include "afxwin.h"

// CAmbigFunDlg dialog
class CAmbigFunDlg : public CSimulationDialog
{
// Construction
public:
    CAmbigFunDlg(CWnd* pParent = NULL);    // standard constructor

    enum mode_t { MODE_DEMO, MODE_SIM };

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
	CPlotControl m_cQualityPlot;
    CSurfacePlotControl m_cAmbigfunPlot;
	model::model_data m_data;
    mode_t m_mode;
    model::cancellation_token m_ct;
    model::sigtuple_t < CEdit > m_tfTau;
    model::sigtuple_t < CEdit > m_tfDopp;
public:
    CButton m_cDisplayType;
    afx_msg void OnBnClickedButton1();
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    virtual void OnSimulation();
    void OnDemo();
    void OnSim();
    afx_msg void OnBnClickedRadio1();
    afx_msg void OnBnClickedRadio2();
    afx_msg void OnBnClickedRadio3();
    void SetupPlots(bool am, bool pm, bool fm);
    virtual BOOL DestroyWindow();
};
