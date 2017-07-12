#include "crenderingthread.h"

CRenderingThread::CRenderingThread(QObject *parent)
	: QThread(parent)
{
	sptrk.reset(new handTracker);
}

CRenderingThread::~CRenderingThread()
{

}


void CRenderingThread::run() {
	while (1)
	{
		emit UpdateSingal(sptrk->handRecognizer());
	}
}