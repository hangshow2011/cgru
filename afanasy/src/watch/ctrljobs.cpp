#include "ctrljobs.h"

#include <QtCore/QEvent>
#include <QtGui/QContextMenuEvent>
#include <QLabel>
#include <QBoxLayout>
#include <QMenu>

#include "../libafqt/qenvironment.h"

#include "button.h"

#define AFOUTPUT
#undef AFOUTPUT
#include "../include/macrooutput.h"
#include "../libafanasy/logger.h"

QStringList CtrlJobs::ms_thumbs_names = {"S","M","L","XL"};
QList<int>  CtrlJobs::ms_thumbs_sizes = { 25, 50,100, 200};

CtrlJobs::CtrlJobs(QWidget * i_parent, ListJobs * i_listjobs, bool i_inworklist):
	QFrame(i_parent),
	m_listjobs(i_listjobs),
	m_inworklist(i_inworklist)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	QHBoxLayout * layout = new QHBoxLayout(this);

	layout->addWidget(new QLabel(tr("Thumbs:"), this));

	ms_thumbs_names[0] = tr("S");
	ms_thumbs_names[1] = tr("M");
	ms_thumbs_names[2] = tr("L");
	ms_thumbs_names[3] = tr("XL");
	for (int i = 0; i < ms_thumbs_names.size(); i++)
	{
		Button * btn = new Button(ms_thumbs_names[i], QString(), QString(), false, true);
		layout->addWidget(btn);
		connect(btn, SIGNAL(sig_Clicked(Button*)), this, SLOT(slot_ThumsButtonClicked(Button*)));

		if (m_inworklist)
		{
			if (ms_thumbs_sizes[i] == afqt::QEnvironment::thumb_work_height.n)
				btn->setActive(true);
		}
		else
		{
			if (ms_thumbs_sizes[i] == afqt::QEnvironment::thumb_jobs_height.n)
				btn->setActive(true);
		}

		m_thumbs_btns.append(btn);
	}

	CtrlJobsViewOptions * viewOpts = new CtrlJobsViewOptions(this, m_listjobs, m_inworklist);
	layout->addWidget(viewOpts);
}

CtrlJobs::~CtrlJobs()
{
}

void CtrlJobs::slot_ThumsButtonClicked(Button * i_btn)
{
	int size = 0;
	for (int i = 0; i < ms_thumbs_names.size(); i++)
		if (i_btn->getName() == ms_thumbs_names[i])
			size = ms_thumbs_sizes[i];

	if (m_inworklist)
	{
		if (size == afqt::QEnvironment::thumb_work_height.n)
			return;
	}
	else
	{
		if (size == afqt::QEnvironment::thumb_jobs_height.n)
			return;
	}

	for (int i = 0; i < m_thumbs_btns.size(); i++)
		if (m_thumbs_btns[i] != i_btn)
			m_thumbs_btns[i]->setActive(false);

	if (m_inworklist)
		afqt::QEnvironment::thumb_work_height.n = size;
	else
		afqt::QEnvironment::thumb_jobs_height.n = size;

	m_listjobs->jobsHeightRecalculate();
}

CtrlJobsViewOptions::CtrlJobsViewOptions(QWidget * i_parent, ListJobs * i_listjobs, bool i_inworklist):
	QLabel(tr("View Options"), i_parent),
	m_listjobs(i_listjobs),
	m_inworklist(i_inworklist)
{
	setFrameShape(QFrame::StyledPanel);
	setFrameShadow(QFrame::Raised);

	setToolTip("Click RMB for options.");
}

CtrlJobsViewOptions::~CtrlJobsViewOptions()
{
}

void CtrlJobsViewOptions::contextMenuEvent(QContextMenuEvent * i_event)
{
	QMenu menu(this);
	QAction * action;
	ActionId * action_id;

	action = new QAction(tr("Hide:"), this);
	action->setEnabled(false);
	menu.addAction(action);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideInvert, tr("Invert"), this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideInvert);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);
	menu.addSeparator();

	action_id = new ActionId(ListNodes::e_HideHidden, tr("Hidden"), this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideHidden);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideDone, tr("Done"), this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideDone);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideError, tr("Error"), this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideError);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	action_id = new ActionId(ListNodes::e_HideOffline, tr("Offline"), this);
	action_id->setCheckable(true);
	action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideOffline);
	connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
	menu.addAction(action_id);

	if (m_inworklist)
	{
		action_id = new ActionId(ListNodes::e_HideEmpty, tr("Empty"), this);
		action_id->setCheckable(true);
		action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideEmpty);
		connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
		menu.addAction(action_id);

		action_id = new ActionId(ListNodes::e_HideSystem, tr("System"), this);
		action_id->setCheckable(true);
		action_id->setChecked(m_listjobs->getFlagsHideShow() & ListNodes::e_HideSystem);
		connect(action_id, SIGNAL(triggeredId(int)), m_listjobs, SLOT(actHideShow(int)));
		menu.addAction(action_id);
	}

	menu.addSeparator();

	action = new QAction(tr("Collapse All Jobs"), this);
	connect(action, SIGNAL(triggered()), m_listjobs, SLOT(slot_CollapseJobs()));
	menu.addAction(action);

	action = new QAction(tr("Expand All Jobs"), this);
	connect(action, SIGNAL(triggered()), m_listjobs, SLOT(slot_ExpandJobs()));
	menu.addAction(action);

	action = new QAction(tr("Collapse New Jobs"), this);
	action->setCheckable(true);
	action->setChecked(afqt::QEnvironment::collapseNewJobs());
	connect(action, SIGNAL(triggered(bool)), this, SLOT(slot_CollapseNewJobs(bool)));
	menu.addAction(action);

	menu.exec(i_event->globalPos());
}

void CtrlJobsViewOptions::slot_CollapseNewJobs(bool i_collapse)
{
	afqt::QEnvironment::setCollapseNewJobs(i_collapse);
}

