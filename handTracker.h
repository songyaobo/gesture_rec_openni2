#pragma once
#include <NiTE.h>
#include <boost/log/trivial.hpp>
#include <vector>


enum trackerMsg
{
	FROME_LEFT_2_RIGHT,
	FROM_RIGHT_2_LEFT,
	FROME_BOTTOM_2_TOP,
	FROM_TOP_2_BOTTOM
};

class handTracker
{
public:
	handTracker(void);
	~handTracker(void);

public:
	struct dbPosition
	{
		double x;
		double y;
		double z;
		dbPosition(double _x, double _y, double _z) {
			x = _x;
			y = _y;
			z = _z;
		};
	};
private:
	int cnt;   
	const static double eps;
	std::vector<dbPosition> vPoints;
	void clearBuf();

public:
	bool Initialization();
	int handRecognizer();

private:
	nite::HandTracker Tracker;
	nite::Status niteRc;
	nite::HandTrackerFrameRef handTrackerFrame;
	nite::HandId newId;
};

