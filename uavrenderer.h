#ifndef UAVRENDERER_H
#define UAVRENDERER_H

#include <QtWidgets/QMainWindow>
#include "ui_uavrenderer.h"

#include <boost/log/trivial.hpp>
#include <boost/shared_ptr.hpp>
#include "crenderingthread.h"
QT_BEGIN_NAMESPACE
class QTimer;
class QAction;
class QMenu;
class QPushButton;
class QMenuBar;
class QVTKWidget;
class vtkPolyData;
class vtkSTLReader;
class vtkPolyDataMapper;
class vtkActor;
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkEventQtSlotConnect;
class vtkObject;
class vtkCommand;
class vtkJPEGReader;
class vtkTexture;
QT_END_NAMESPACE

class UavRenderer : public QMainWindow
{
	Q_OBJECT


public:
	UavRenderer(QWidget *parent = 0);
	~UavRenderer();


const static int ndegree;
public:
	void contextMenuEvent(QContextMenuEvent *event);
	void mouseDoubleClickEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent *);
public slots:
	void popup(vtkObject * obj, unsigned long,
		void * client_data, void *,
		vtkCommand * command);
	void vtkMouseMove();
private:
	Ui::UavRendererClass ui;

private slots:
	void openFile();
	void vtInformation();
	void UpdateSlot(int pose);
private:
	void createMenus();
	QMenu* fileMenus;
	QMenu* aboutMenus;
	QAction* openAct;
	QAction* openAbout;
	QAction* exitAct;
	boost::shared_ptr<CRenderingThread> pThread;
	bool b_fullScreen;
private:
	vtkSTLReader* m_reader;
	vtkPolyDataMapper* m_mapper;
	vtkActor* m_actor;
	vtkRenderer* m_renderer;
	vtkRenderWindow* m_renderWindow;
	vtkEventQtSlotConnect* Connections;
	vtkJPEGReader* m_jpegReader;
	vtkTexture* m_vtkTexture;
	bool b_loadvtk;
	void deleteVtkResource();
private:
	void ElevateCamera(double angle);
};

#endif // UAVRENDERER_H
