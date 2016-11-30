/*******************************************************************************
*                                                                              *
*   PrimeSense NiTE 2.0 - Simple Tracker Sample                                *
*   Copyright (C) 2012 PrimeSense Ltd.                                         *
*                                                                              *
*******************************************************************************/

#include "NiTE.h"
#include <NiteSampleUtilities.h>

#include<vector>
#include<iostream>
#include <opencv/cv.h>
#include<opencv/highgui.h>
#include<opencv/cv.hpp>

using namespace std;


vector<double> start_x,start_y,start_z;
static int num=0;   
static double eps=0.000000000001;
nite::HandId newId;

void initial(){
	start_x.clear();
	start_y.clear();
	start_z.clear();
	num=0;
}

int main(int argc, char** argv)
{
	nite::HandTracker handTracker;
	nite::Status niteRc;

	niteRc = nite::NiTE::initialize();
	if (niteRc != nite::STATUS_OK)
	{
		printf("NiTE initialization failed\n");
		return 1;
	}

	niteRc = handTracker.create();
	if (niteRc != nite::STATUS_OK)
	{
		printf("Couldn't create user tracker\n");
		return 3;
	}

	handTracker.startGestureDetection(nite::GESTURE_CLICK);
	handTracker.startGestureDetection(nite::GESTURE_HAND_RAISE);
	handTracker.setSmoothingFactor(2.0f);
	printf("\nWave or click to start tracking your hand...\n");

	nite::HandTrackerFrameRef handTrackerFrame;
	while (!wasKeyboardHit())
	{
		niteRc = handTracker.readFrame(&handTrackerFrame);
		if (niteRc != nite::STATUS_OK)
		{
			printf("Get next frame failed\n");
			continue;
		}
		
		const nite::Array<nite::GestureData>& gestures = handTrackerFrame.getGestures();
		for (int i = 0; i < gestures.getSize(); ++i)
		{
			if(gestures[i].getType()==nite::GESTURE_CLICK)
			{
				//cout<<"click"<<endl;

			}
			if(gestures[i].getType()==nite::GESTURE_HAND_RAISE)
			{
				//cout<<"raise"<<endl;
				if (gestures[i].isComplete())
				{
					//flag=false;
					
					handTracker.startHandTracking(gestures[i].getCurrentPosition(), &newId);
					//handTracker.stopGestureDetection(nite::GESTURE_HAND_RAISE);
				}		
			}
		}

		const nite::Array<nite::HandData>& hands = handTrackerFrame.getHands();
		for (int i = 0; i < hands.getSize(); ++i)
		{
			const nite::HandData& hand = hands[i];

			if(hand.isLost())
			{
				//cout<<"lost"<<endl;
				initial();
				handTracker.stopHandTracking(newId);
				break;
			}

			if (hand.isTracking())
			{
				num++;
				start_x.push_back(hand.getPosition().x);
				start_y.push_back(hand.getPosition().y);
				start_z.push_back(hand.getPosition().z);

				if((start_x.size()>1)&&abs(start_x.at(num-1)-start_x.at(num-2))==0&&abs(start_y.at(num-1)-start_y.at(num-2))==0)
				{
					initial();
					//handTracker.startGestureDetection(nite::GESTURE_HAND_RAISE);
					//handTracker.stopHandTracking(newId);
				}

				if((start_x.size()>1)&&abs(start_x.at(num-1)-start_x.at(num-2))!=0&&abs(start_y.at(num-1)-start_y.at(num-2))!=0)
				{
					double k=(start_y.at(num-1)-start_y.at(0))/((start_x.at(num-1)-start_x.at(0))+eps);

					if(k!=0)
					{
						if(abs(k)<1)
						{
							if((start_x.at(num-1)-start_x.at(0))>0)
							{							
								cout<<"++++horizontal++++"<<endl;							
							}
							else
							{							
								cout<<"----horizontal----"<<endl;	
							}
						}
						else
						{
							if((start_y.at(num-1)-start_y.at(0))>0)
							{							
								cout<<"++++vertical++++"<<endl;								
							}
							else
							{							
								cout<<"----vertical------"<<endl;								
							}
						}				
					}
				}							
			}
		}
	}
	nite::NiTE::shutdown();
}



