// ambig-funDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ambig-fun.h"
#include "ambig-funDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAmbigFunDlg dialog

CAmbigFunDlg::CAmbigFunDlg(CWnd* pParent /*=NULL*/)
    : CSimulationDialog(CAmbigFunDlg::IDD, pParent)
    , m_data(model::make_model_data())
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAmbigFunDlg::DoDataExchange(CDataExchange* pDX)
{
    CSimulationDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PLOT1, m_cSignalsPlot);
    DDX_Control(pDX, IDC_PLOT2, m_cSignalsShiftedPlot);
    DDX_Control(pDX, IDC_PLOT5, m_cCorrelationPlot);
    DDX_Control(pDX, IDC_PLOT3, m_cQualityPlot);
    DDX_Control(pDX, IDC_RADIO1, m_cDisplayType);
	DDX_Text(pDX, IDC_EDIT1, m_data.params->carrier);
	DDX_Text(pDX, IDC_EDIT2, m_data.params->sampling_rate);
	DDX_Text(pDX, IDC_EDIT3, m_data.params->bitrate);
	DDX_Text(pDX, IDC_EDIT4, m_data.params->N);
	DDX_Text(pDX, IDC_EDIT5, m_data.params->tau);
	DDX_Text(pDX, IDC_EDIT6, m_data.params->doppler);
	DDX_Text(pDX, IDC_EDIT7, m_data.params->snr);
	DDX_Text(pDX, IDC_EDIT8, m_data.params->dopp_from);
	DDX_Text(pDX, IDC_EDIT9, m_data.params->dopp_to);
	DDX_Text(pDX, IDC_EDIT10, m_data.params->dopp_count);
	DDX_Text(pDX, IDC_EDIT11, m_data.params->tau_from);
	DDX_Text(pDX, IDC_EDIT12, m_data.params->tau_to);
	DDX_Text(pDX, IDC_EDIT13, m_data.params->tau_count);
	DDX_Text(pDX, IDC_EDIT14, m_data.params->num_of_tests);
}

BEGIN_MESSAGE_MAP(CAmbigFunDlg, CSimulationDialog)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CAmbigFunDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2, &CAmbigFunDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON3, &CAmbigFunDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_RADIO1, &CAmbigFunDlg::OnBnClickedRadio1)
    ON_BN_CLICKED(IDC_RADIO2, &CAmbigFunDlg::OnBnClickedRadio2)
    ON_BN_CLICKED(IDC_RADIO3, &CAmbigFunDlg::OnBnClickedRadio3)
END_MESSAGE_MAP()

// CAmbigFunDlg message handlers

BOOL CAmbigFunDlg::OnInitDialog()
{
    CSimulationDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    // TODO: Add extra initialization here

	m_cSignalsPlot.plot_layer.with(
		model::make_root_drawable(m_data.signals, { {
				m_data.signals.am.re.plot,
				m_data.signals.pm.re.plot,
				m_data.signals.fm.re.plot,
				m_data.signals.am.im.plot,
				m_data.signals.pm.im.plot,
				m_data.signals.fm.im.plot
		} })
	);
	m_cSignalsShiftedPlot.plot_layer.with(
		model::make_root_drawable(m_data.signals_shifted, { {
				m_data.signals_shifted.am.re.plot,
				m_data.signals_shifted.pm.re.plot,
				m_data.signals_shifted.fm.re.plot,
				m_data.signals_shifted.am.im.plot,
				m_data.signals_shifted.pm.im.plot,
				m_data.signals_shifted.fm.im.plot
		} })
	);
	m_cCorrelationPlot.plot_layer.with(
		model::make_root_drawable(m_data.correlation, { {
				m_data.correlation.am.plot,
				m_data.correlation.pm.plot,
				m_data.correlation.fm.plot,
		} })
	);
	m_cQualityPlot.plot_layer.with(
		model::make_root_drawable(m_data.quality, { {
				m_data.quality.am.plot,
				m_data.quality.pm.plot,
				m_data.quality.fm.plot,
		} })
	);

	m_cSignalsPlot.triple_buffered = true;
	m_cSignalsShiftedPlot.triple_buffered = true;
	m_cCorrelationPlot.triple_buffered = true;
	m_cQualityPlot.triple_buffered = true;

    m_cDisplayType.SetCheck(1);

    SetupPlots(true, false, false);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAmbigFunDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CSimulationDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAmbigFunDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CAmbigFunDlg::OnBnClickedButton1()
{
    OnBnClickedButton3();

    UpdateData(TRUE);
    m_ct = model::cancellation_token();
    m_mode = MODE_DEMO;
    StartSimulationThread();
}


void CAmbigFunDlg::OnBnClickedButton2()
{
    OnBnClickedButton3();

    UpdateData(TRUE);
    m_ct = model::cancellation_token();
    m_mode = MODE_SIM;
    StartSimulationThread();
}


void CAmbigFunDlg::OnBnClickedButton3()
{
    m_ct.cancel();
    StopSimulationThread();
}


void CAmbigFunDlg::OnSimulation()
{
    switch (m_mode)
    {
    case CAmbigFunDlg::MODE_DEMO: OnDemo(); break;
    case CAmbigFunDlg::MODE_SIM: OnSim(); break;
    default: ASSERT(FALSE); break;
    }
    CSimulationDialog::OnSimulation();
}


void CAmbigFunDlg::OnDemo()
{
    model::signals_pair r;
    model::gen_signals(r, *m_data.params, model::from_params(*m_data.params));
    model::fill_signals(m_data, r);
    
    model::signals_t c;
    model::correlate(r, c, 0, true, m_ct);
    model::fill_corr(m_data, c);

    m_cSignalsPlot.RedrawBuffer();
    m_cSignalsPlot.SwapBuffers();
    m_cSignalsPlot.RedrawWindow();
    m_cSignalsShiftedPlot.RedrawBuffer();
    m_cSignalsShiftedPlot.SwapBuffers();
    m_cSignalsShiftedPlot.RedrawWindow();
    m_cCorrelationPlot.RedrawBuffer();
    m_cCorrelationPlot.SwapBuffers();
    m_cCorrelationPlot.RedrawWindow();

    model::signals_t q;
    model::quality(q, *m_data.params, m_ct, [&] () {
        model::fill_qual(m_data, q);

        m_cQualityPlot.RedrawBuffer();
        m_cQualityPlot.SwapBuffers();
        m_cQualityPlot.RedrawWindow();
    });
}


void CAmbigFunDlg::OnSim()
{
}


void CAmbigFunDlg::SetupPlots(bool am, bool pm, bool fm)
{
    m_data.signals.am.re.plot->visible = am;
    m_data.signals.am.im.plot->visible = am;
    m_data.signals.pm.re.plot->visible = pm;
    m_data.signals.pm.im.plot->visible = pm;
    m_data.signals.fm.re.plot->visible = fm;
    m_data.signals.fm.im.plot->visible = fm;
    m_data.signals_shifted.am.re.plot->visible = am;
    m_data.signals_shifted.am.im.plot->visible = am;
    m_data.signals_shifted.pm.re.plot->visible = pm;
    m_data.signals_shifted.pm.im.plot->visible = pm;
    m_data.signals_shifted.fm.re.plot->visible = fm;
    m_data.signals_shifted.fm.im.plot->visible = fm;
    m_data.correlation.am.plot->visible = am;
    m_data.correlation.pm.plot->visible = pm;
    m_data.correlation.fm.plot->visible = fm;

    m_cSignalsPlot.RedrawBuffer();
    m_cSignalsPlot.SwapBuffers();
    m_cSignalsShiftedPlot.RedrawBuffer();
    m_cSignalsShiftedPlot.SwapBuffers();
    m_cCorrelationPlot.RedrawBuffer();
    m_cCorrelationPlot.SwapBuffers();

    Invoke([this] () {
        m_cSignalsPlot.RedrawWindow();
        m_cSignalsShiftedPlot.RedrawWindow();
        m_cCorrelationPlot.RedrawWindow();
    });
}


void CAmbigFunDlg::OnBnClickedRadio1()
{
    SetupPlots(true, false, false);
}


void CAmbigFunDlg::OnBnClickedRadio2()
{
    SetupPlots(false, true, false);
}


void CAmbigFunDlg::OnBnClickedRadio3()
{
    SetupPlots(false, false, true);
}


BOOL CAmbigFunDlg::DestroyWindow()
{
    m_ct.cancel();
    return CSimulationDialog::DestroyWindow();
}
