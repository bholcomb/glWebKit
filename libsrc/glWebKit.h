
#pragma once

#include <EAWebKit\EAWebKit.h>
#include <EAWebKit\EAWebKitView.h>

#include <string>

extern EA::WebKit::EAWebKitLib* wk;

bool initWebkit();
EA::WebKit::View* createView();


void updateWebkit();
void updateView(EA::WebKit::View* v);

void resize(EA::WebKit::View* v, int width, int height);
void mousemove(EA::WebKit::View* v, int x, int y);
void mousebutton(EA::WebKit::View* v, int x, int y, int btn, bool depressed);
void mousewheel(EA::WebKit::View* v, int x, int y, int keys, int delta);
void keyboard(EA::WebKit::View* v, int id, bool ischar, bool depressed);
void reload(EA::WebKit::View* v);
void destroyView(EA::WebKit::View* v);

double timerCallback();
