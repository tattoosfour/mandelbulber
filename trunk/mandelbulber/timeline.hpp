/*
 * timeline.hpp
 *
 *  Created on: 2010-09-20
 *      Author: krzysztof
 */

#ifndef TIMELINE_HPP_
#define TIMELINE_HPP_

#include "database.hpp"
#include "cimage.hpp"


gboolean thumbnail_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);


struct sTimelineRecord
{
	int index;
	sRGB8 thumbnail[128*128];
};

class cTimeline
{
public:
	cTimeline();
	~cTimeline();
	bool IsCreated(void) {return isCreated;}
	int Initialize(char *keyframesPath); //returns number of keyframes
	void GetImage(int index, sRGB8 *image);
	void DisplayInDrawingArea(int index, GtkWidget *darea);
	int CheckNumberOfKeyframes(char *keyframesPath);
	void CreateInterface(int numberOfKeyframes);
	void RebulidTimelineWindow(void);

private:

	cDatabase *database;
	int keyframeCount;
	bool isCreated;
};

extern cTimeline *timeline;



#endif /* TIMELINE_HPP_ */
