#include "uavrenderer.h"

#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkActor.h>
#include <vtkRenderer.h>

#include <vtkSTLReader.h>
#include <QVTKWidget.h>
#include <vtkCommand.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkJPEGReader.h>
#include <vtkTexture.h>
#include <vtkTexturingHelper.h>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenuBar>
#include <QLayout>
#include <QSplashScreen>

UavRenderer::UavRenderer(QWidget *parent)
	: QMainWindow(parent), m_reader(NULL), 
	  m_renderer(NULL), 
	  m_renderWindow(NULL), 
	  m_actor(NULL),
	  m_mapper(NULL)
{
	QPixmap pixmap(tr(":/UavRenderer/Resources/vt.png"));
	QSplashScreen splash(pixmap);
	splash.show();
	splash.showMessage(tr("Loading..."));
	ui.setupUi(this);
	
	qApp->processEvents();

	ui.mainToolBar->close();
	createMenus();
	this->setWindowIcon(QIcon(tr(":/UavRenderer/Resources/vt.png")));
	this->showMaximized();
	BOOST_LOG_TRIVIAL(info) << "Server starting...";
	b_fullScreen = false;
	m_reader = nullptr;
	b_loadvtk = false;
	pThread.reset(new CRenderingThread);
	connect(pThread.get(), &CRenderingThread::UpdateSingal, this, &UavRenderer::UpdateSlot);

	Connections = vtkEventQtSlotConnect::New();
	Connections->Connect(ui.qvtkWidget->GetRenderWindow()->GetInteractor(), 
		                 vtkCommand::RightButtonPressEvent, this, 
						 SLOT(popup(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 
						 fileMenus);
	Connections->Connect(ui.qvtkWidget->GetRenderWindow()->GetInteractor(), 
		vtkCommand::MouseMoveEvent, this, SLOT(vtkMouseMove()));
	splash.close();
}

const int UavRenderer::ndegree = 8;

void UavRenderer::deleteVtkResource() {
	m_reader->Delete();
	m_renderer->Delete();
	m_renderWindow->Delete();
	m_actor->Delete();
	m_mapper->Delete();

	m_jpegReader->Delete();
	m_vtkTexture->Delete();

	m_jpegReader = NULL;
	m_vtkTexture = NULL;
	m_reader = NULL;
	m_renderer = NULL;
	m_renderWindow = NULL;
	m_actor = NULL;
	m_mapper = NULL;
}

UavRenderer::~UavRenderer()
{
	Connections->Delete();
	Connections = NULL;
}

void UavRenderer::createMenus() {
	fileMenus = menuBar()->addMenu(tr("&File"));
	aboutMenus = menuBar()->addMenu(tr("&About"));

	openAct = new QAction(tr("&Open..."), this);
	fileMenus->addAction(openAct);
	connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

	fileMenus->addSeparator();
	exitAct = new QAction(tr("E&xit"), this);
	fileMenus->addAction(exitAct);
	connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
	
	openAbout = new QAction(tr("&About..."), this);
	aboutMenus->addAction(openAbout);
	connect(openAbout, SIGNAL(triggered()), this, SLOT(vtInformation()));
}

void UavRenderer::popup(vtkObject * obj, unsigned long, void * client_data, void *, vtkCommand * command) {
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	command->AbortFlagOn();
	QMenu* popupMenu = static_cast<QMenu*>(client_data);
	int* sz = iren->GetSize();
	int* position = iren->GetEventPosition();
	QPoint pt = QPoint(position[0], sz[1]-position[1]);
	QPoint global_pt = popupMenu->parentWidget()->mapToGlobal(pt);
	popupMenu->popup(global_pt);
}

void UavRenderer::vtkMouseMove() {
	setCursor(Qt::ArrowCursor);
}

void UavRenderer::openFile() {
	pThread->terminate();
	if (b_loadvtk) 
		deleteVtkResource();

	QString fileName = QFileDialog::getOpenFileName(this,
		                                            tr("Open STL model"),
													tr("."),
													tr("Model Files(*.stl *STL)"));
	if (fileName.isEmpty()) {
		QMessageBox::information(this, tr("information"), tr("Stl file not found"), QMessageBox::Ok);
		return;
	}

	QString qstatus;
	try {
		qstatus = tr("Loading....");
		this->ui.statusBar->showMessage(qstatus);
		m_reader = vtkSTLReader::New();
		m_reader->SetFileName(fileName.toStdString().c_str());
		m_reader->Update();

		m_jpegReader = vtkJPEGReader::New();
		m_jpegReader->SetFileName("E:\\myproject\\UavRenderer\\UavRenderer\\Resources\\texture_3.jpg");
		m_jpegReader->Update();
	} catch (...) {
		qstatus = tr("Loading error...");
		this->ui.statusBar->showMessage(qstatus);
		return;
	}
	b_loadvtk = true;
	BOOST_LOG_TRIVIAL(info) << "Wave to start tracking your hand...";
	qstatus = tr("Successful");
	this->ui.statusBar->showMessage(qstatus);


	m_mapper = vtkPolyDataMapper::New();
	m_mapper->SetInputConnection(m_reader->GetOutputPort());

	m_actor = vtkActor::New();
	m_actor->SetMapper(m_mapper);

	m_vtkTexture = vtkTexture::New();
	m_vtkTexture->SetInputConnection(m_jpegReader->GetOutputPort());
	m_vtkTexture->InterpolateOn();
	m_vtkTexture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_REPLACE);
	m_actor->SetTexture(m_vtkTexture);
	m_renderer = vtkRenderer::New();
	m_renderWindow = vtkRenderWindow::New();
	m_renderWindow->AddRenderer(m_renderer);

	m_renderer->AddActor(m_actor);
	m_renderer->SetBackground(0, 0, .0);

	ui.qvtkWidget->SetRenderWindow(m_renderWindow);
	Connections->Connect(ui.qvtkWidget->GetRenderWindow()->GetInteractor(), 
		                 vtkCommand::RightButtonPressEvent, 
						 this, 
						 SLOT(popup(vtkObject*, unsigned long, void*, void*, vtkCommand*)), 
						 fileMenus);
	Connections->Connect(ui.qvtkWidget->GetRenderWindow()->GetInteractor(), 
		vtkCommand::MouseMoveEvent, this, SLOT(vtkMouseMove()));
	pThread->start();
}

void UavRenderer::vtInformation() {
	QMessageBox msg(QMessageBox::NoIcon, tr("About"),
		           tr("Welcome to VisualTouring. inc."));
	msg.setIconPixmap(QPixmap(tr(":/UavRenderer/Resources/vt.png")));
	msg.exec();
}

void UavRenderer::contextMenuEvent(QContextMenuEvent *event) {
	fileMenus->move(cursor().pos());
	fileMenus->show();
}

void UavRenderer::mouseDoubleClickEvent(QMouseEvent * event) {
	if (!b_fullScreen) {
		this->showFullScreen();
		b_fullScreen = true;
	} else {
		this->showMaximized();
		b_fullScreen = false;
	}
}

void UavRenderer::mouseMoveEvent(QMouseEvent *) {
	setCursor(Qt::ArrowCursor);
}
void UavRenderer::ElevateCamera(double angle) {
	double elevationPartialShift = angle / 5; 
	for (int i = 0; i < 5; ++i) 
	{ 
		m_renderer->GetActiveCamera()->Elevation(elevationPartialShift); 
		m_renderer->GetActiveCamera()->OrthogonalizeViewUp(); 
	} 
}

void UavRenderer::UpdateSlot(int pose) {
	if (m_reader) {
		switch(pose) {
		case FROM_RIGHT_2_LEFT:
			m_renderWindow->Render();
			m_renderer->GetActiveCamera()->Azimuth(ndegree);
			m_renderer->GetActiveCamera()->OrthogonalizeViewUp();
			qApp->processEvents();
			break;

		case  FROME_LEFT_2_RIGHT:
			m_renderWindow->Render();
			m_renderer->GetActiveCamera()->Azimuth(-ndegree);
			m_renderer->GetActiveCamera()->OrthogonalizeViewUp();
			qApp->processEvents();
			break;

		case FROME_BOTTOM_2_TOP:
			m_renderWindow->Render();
			ElevateCamera(-ndegree);
			qApp->processEvents();
			break;

		case  FROM_TOP_2_BOTTOM:
			m_renderWindow->Render();
			ElevateCamera(ndegree);
			qApp->processEvents();
			break;
		default:;
		}
	}
}