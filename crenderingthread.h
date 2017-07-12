#ifndef CRENDERINGTHREAD_H
#define CRENDERINGTHREAD_H

#include <QThread>
#include <handTracker.h>
#include <boost/shared_ptr.hpp>
class CRenderingThread : public QThread
{
	Q_OBJECT

public:
	CRenderingThread(QObject *parent = 0);
	~CRenderingThread();

private:
	boost::shared_ptr<handTracker> sptrk;
	
signals:
	void UpdateSingal(int num);
protected:
	void run();
};

#endif // CRENDERINGTHREAD_H
