#pragma once

#include <string>
#include <vector>
#include <EAWebKit\EAWebKit.h>

extern unsigned int frame;

#include <EAWebKit\EAWebKit.h>
#include <EAWebKit\EAWebKitView.h>

#include <string>

int getSystemFonts(std::vector<std::string>& fonts);
int add_ttf_font(EA::WebKit::EAWebKitLib* wk, const char* ttfFile);
int init_system_fonts(EA::WebKit::EAWebKitLib* wk);

