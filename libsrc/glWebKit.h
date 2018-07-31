
#pragma once

#include <EAWebKit\EAWebKit.h>
#include <EAWebKit\EAWebKitView.h>

#include <string>

extern EA::WebKit::EAWebKitLib* wk;
extern EA::WebKit::View* v;

bool init();
void update();
void updateTexture(unsigned int id);
void resize(int width, int height);
void mousemove(int x, int y);
void mousebutton(int x, int y, int btn, bool depressed);
void mousewheel(int x, int y, int keys, int delta);
void keyboard(int id, bool ischar, bool depressed);
void reload();
void shutdown();
