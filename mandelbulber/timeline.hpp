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
#include "smartptr.h"


gboolean thumbnail_expose(GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
void PressedKeyframeThumbnail(GtkWidget *widget, GdkEventButton *event);

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
	bool GetImage(int index, sRGB8 *image);
	void DisplayInDrawingArea(int index, GtkWidget *darea);
	int CheckNumberOfKeyframes(char *keyframesPath);
	void CreateInterface(int numberOfKeyframes);
	void RebulidTimelineWindow(void);
	void RecordKeyframe(int index, char *keyframeFile, bool modeInsert);
	void DeleteKeyframe(int index, char *keyframesPath);
	void Resize(int newsize);
	void Reset(void);
	bool isOpened;

private:
	smart_ptr<cDatabase> database;
	int keyframeCount;
	bool isCreated;
};

extern smart_ptr<cTimeline> timeline;



#endif /* TIMELINE_HPP_ */
