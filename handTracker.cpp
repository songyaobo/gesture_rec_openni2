#include "handTracker.h"


handTracker::handTracker(void)
{
	cnt = 0;
	if (Initialization())
		BOOST_LOG_TRIVIAL(info) << "NiTE initialization successful.";
}

const double handTracker::eps = 1e-14;

handTracker::~handTracker(void)
{
	nite::NiTE::shutdown();
}


void handTracker::clearBuf() {
	vPoints.clear();
	cnt = 0;
}

bool handTracker::Initialization() {
	niteRc = nite::NiTE::initialize();
	if (niteRc != nite::STATUS_OK)
	{
		BOOST_LOG_TRIVIAL(fatal) << "NiTE initialization failed";
		return false;
	}

	niteRc = Tracker.create();
	if (niteRc != nite::STATUS_OK)
	{
		BOOST_LOG_TRIVIAL(fatal) << "Couldn't create user tracker";
		return false;
	}

	Tracker.startGestureDetection(nite::GESTURE_CLICK);
	Tracker.startGestureDetection(nite::GESTURE_HAND_RAISE);
	Tracker.setSmoothingFactor(2.0f);
	return true;
}

int handTracker::handRecognizer() {
	niteRc = Tracker.readFrame(&handTrackerFrame);
	if (niteRc != nite::STATUS_OK)
	{
		BOOST_LOG_TRIVIAL(error) << "Get next frame failed";
	}

	const nite::Array<nite::GestureData>& gestures = handTrackerFrame.getGestures();
	for (int i = 0; i < gestures.getSize(); ++i)
	{
		if(gestures[i].getType()==nite::GESTURE_HAND_RAISE)
		{
			if (gestures[i].isComplete())
			{
				Tracker.startHandTracking(gestures[i].getCurrentPosition(), &newId);
			}		
		}
	}

	const nite::Array<nite::HandData>& hands = handTrackerFrame.getHands();
	for (int i = 0; i < hands.getSize(); ++i)
	{
		const nite::HandData& hand = hands[i];

		if(hand.isLost())
		{
			BOOST_LOG_TRIVIAL(info) << "Lost hand, searching....";
			clearBuf();
			Tracker.stopHandTracking(newId);
			break;
		}

		if (hand.isTracking())
		{
			cnt++;
			
			vPoints.push_back(dbPosition(hand.getPosition().x,
				                         hand.getPosition().y, 
										 hand.getPosition().z));
			
			if (vPoints.size() > 1 && abs(vPoints.at(cnt - 1).x - vPoints.at(cnt - 2).x == 0)
				&& abs(vPoints.at(cnt - 1).y - vPoints.at(cnt - 2).y) == 0)
			{
				clearBuf();
			}

			
			if (vPoints.size() > 1 && abs(vPoints.at(cnt - 1).x - vPoints.at(cnt - 2).x) != 0
				&& abs(vPoints.at(cnt - 1).y - vPoints.at(cnt - 2).y) != 0)
			{
				double k = (vPoints[cnt - 1].y - vPoints[0].y) / ((vPoints[cnt - 1].x - vPoints[0].x) + eps);
				if(k!=0)
				{
					if(abs(k)<1)
					{
						if (vPoints[cnt-1].x- vPoints[0].x >0)
						{							
							BOOST_LOG_TRIVIAL(info) << "++++horizontal++++";
							return FROME_LEFT_2_RIGHT;
						}
						else
						{							
							BOOST_LOG_TRIVIAL(info) << "----horizontal----";
							return FROM_RIGHT_2_LEFT;
						}
					}
					else
					{
						if (vPoints[cnt-1].y- vPoints[0].y >0)
						{							
							BOOST_LOG_TRIVIAL(info) << "++++vertical++++";	
							return FROME_BOTTOM_2_TOP;
						}
						else
						{							
							BOOST_LOG_TRIVIAL(info) <<"----vertical------";		
							return FROM_TOP_2_BOTTOM;
						}
					}				
				}
			}							
		}
	}
	return -1;
}