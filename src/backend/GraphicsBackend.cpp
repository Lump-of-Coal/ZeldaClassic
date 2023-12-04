#include "GraphicsBackend.h"
#include "MouseBackend.h"
#include "PaletteBackend.h"
#include "Backend.h"
#include "../zc_alleg.h"
#include <cassert>

void Z_message(const char *format, ...);

void (*GraphicsBackend::switch_in_func_)() = NULL;
void (*GraphicsBackend::switch_out_func_)() = NULL;
int GraphicsBackend::fps_ = 60;

volatile int GraphicsBackend::frame_counter = 0;
volatile int GraphicsBackend::frames_this_second = 0;
volatile int GraphicsBackend::prev_frames_this_second = 0;

bool GraphicsBackend::windowsFullscreenFix_ = false;

void update_frame_counter()
{
	static int cnt;
	GraphicsBackend::frame_counter++;
	cnt++;
	if (cnt == GraphicsBackend::fps_)
	{
		GraphicsBackend::prev_frames_this_second = GraphicsBackend::frames_this_second;
		GraphicsBackend::frames_this_second = 0;
		cnt = 0;
	}
}
END_OF_FUNCTION(update_frame_counter)

void onSwitchOut()
{
	if (GraphicsBackend::switch_out_func_)
		GraphicsBackend::switch_out_func_();
}
END_OF_FUNCTION(onSwitchOut);

void onSwitchIn()
{
#ifdef _WINDOWS
	GraphicsBackend::windowsFullscreenFix_ = true;
#endif
	if (GraphicsBackend::switch_in_func_)
		GraphicsBackend::switch_in_func_();
}
END_OF_FUNCTION(onSwitchIn);


GraphicsBackend::GraphicsBackend() :
	hw_screen_(NULL),
	backbuffer_(NULL),
	nativebuffer_(NULL),
	initialized_(false),
	screenw_(320),
	screenh_(240),
	fullscreen_(false),
	native_(true),
	curmode_(-1),
	virtualw_(1),
	virtualh_(1),
	switchdelay_(1)
{	
}

GraphicsBackend::~GraphicsBackend()
{
	if (initialized_)
	{
		remove_int(update_frame_counter);
		screen = hw_screen_;
		destroy_bitmap(backbuffer_);		
		clear_to_color(screen, 0);
	}
	if(nativebuffer_)
		destroy_bitmap(nativebuffer_);
}

void GraphicsBackend::readConfigurationOptions(const std::string &prefix)
{
	std::string section = prefix + "-graphics";
	const char *secname = section.c_str();

	screenw_ = get_config_int(secname, "resx", 640);
	screenh_ = get_config_int(secname, "resy", 480);
	fullscreen_ = get_config_int(secname, "fullscreen", 0) != 0;
	native_ = get_config_int(secname, "native", 1) != 0;
	fps_ = get_config_int(secname, "fps", 60);
}

void GraphicsBackend::writeConfigurationOptions(const std::string &prefix)
{
	std::string section = prefix + "-graphics";
	const char *secname = section.c_str();

	set_config_int(secname, "resx", screenw_);
	set_config_int(secname, "resy", screenh_);
	set_config_int(secname,"fullscreen", isFullscreen() ? 1 : 0 );
	set_config_int(secname, "native", native_ ? 1 : 0);
	set_config_int(secname, "fps", fps_);
}

bool GraphicsBackend::initialize()
{
	if (initialized_)
		return true;

	LOCK_VARIABLE(frame_counter);
	LOCK_FUNCTION(update_frame_counter);
	install_int_ex(update_frame_counter, BPS_TO_TIMER(fps_));
	
	backbuffer_ = create_bitmap_ex(8, virtualScreenW(), virtualScreenH());
	initialized_ = true;

	bool ok = trySettingVideoMode();

	return ok;
}

void GraphicsBackend::waitTick()
{
	if (!initialized_)
		return;

	while (frame_counter == 0)
		rest(0);
	frame_counter = 0;
}

bool GraphicsBackend::showBackBuffer()
{
#ifdef _WINDOWS
	if (windowsFullscreenFix_)
	{
		windowsFullscreenFix_ = false;
		if (fullscreen_)
			if (!trySettingVideoMode())
				return false;
	}
#endif
	Backend::mouse->renderCursor(backbuffer_);

	// Allegro crashes if you call set_palette and screen does not point to the hardware buffer
	screen = hw_screen_;
	Backend::palette->applyPaletteToScreen();
	screen = backbuffer_;

	if (native_)
	{
		stretch_blit(backbuffer_, nativebuffer_, 0, 0, virtualScreenW(), virtualScreenH(), 0, 0, SCREEN_W, SCREEN_H);
		set_color_conversion(COLORCONV_TOTAL);
		blit(nativebuffer_, hw_screen_, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	}
	else
	{
		stretch_blit(backbuffer_, hw_screen_, 0, 0, virtualScreenW(), virtualScreenH(), 0, 0, SCREEN_W, SCREEN_H);
	}
	Backend::mouse->unrenderCursor(backbuffer_);
	frames_this_second++;
	return true;
}

bool GraphicsBackend::setScreenResolution(int w, int h)
{
	if (w != screenw_ || h != screenh_)
	{
		screenw_ = w;
		screenh_ = h;
		if (initialized_ && !fullscreen_)
		{
			return trySettingVideoMode();
		}
	}
	return true;
}

bool GraphicsBackend::setFullscreen(bool fullscreen)
{
	if (fullscreen != fullscreen_)
	{
		fullscreen_ = fullscreen;
		if (initialized_)
		{
			return trySettingVideoMode();
		}
	}
	return true;
}

bool GraphicsBackend::setUseNativeColorDepth(bool native)
{
	if (native_ != native)
	{
		native_ = native;
		if (initialized_)
		{
			return trySettingVideoMode();
		}
	}
	return true;
}

int GraphicsBackend::getLastFPS()
{
	return prev_frames_this_second;
}

void GraphicsBackend::setVideoModeSwitchDelay(int msec)
{
	switchdelay_ = msec;
}

void GraphicsBackend::registerVirtualModes(const std::vector<std::pair<int, int> > &desiredModes)
{
	virtualmodes_ = desiredModes;
	curmode_ = -1;
	if(initialized_)
		trySettingVideoMode();
}

bool GraphicsBackend::trySettingVideoMode()
{
	if (!initialized_)
		return false;

	Backend::mouse->setCursorVisibility(false);

	screen = hw_screen_;

	int depth = native_ ? desktop_color_depth() : 8;

	bool ok = false;

	if (fullscreen_)
	{
		// Try each registered mode, in decreasing succession
		int nmodes = (int)virtualmodes_.size();
		
		for (int i = 0; i < nmodes; i++)
		{
			int w = virtualmodes_[i].first;
			int h = virtualmodes_[i].second;
			set_color_depth(depth);
			if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, w, h, 0, 0) != 0)
			{
				Z_message("Unable to set gfx mode at -%d %dbpp %d x %d \n", GFX_AUTODETECT_FULLSCREEN, 8, w, h);
				rest(switchdelay_);
			}
			else
			{
				ok = true;
				break;
			}
		}

		if (!ok)
		{
			// Can't do fullscreen
			Z_message("Fullscreen mode not supported by your driver. Using windowed mode.");
			fullscreen_ = false;
		}
	}

	if(!ok)
	{
		// Try the prescribed resolution
		set_color_depth(depth);
		if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, screenw_, screenh_, 0, 0) != 0)
		{
			Z_message("Unable to set gfx mode at -%d %dbpp %d x %d \n", GFX_AUTODETECT_WINDOWED, 8, screenw_, screenh_);
			rest(switchdelay_);
		}
		else
		{
			ok = true;
		}
	}

	// Try each of the registered mode, in succession
	if (!ok)
	{
		int nmodes = (int)virtualmodes_.size();

		for (int i = 0; i < nmodes; i++)
		{
			int w = virtualmodes_[i].first;
			int h = virtualmodes_[i].second;
			if (w == screenw_ && h == screenh_)
			{
				// we already tried this combo, don't waste the user's time
				continue;
			}

			set_color_depth(depth);
			if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, w, h, 0, 0) != 0)
			{
				Z_message("Unable to set gfx mode at -%d %dbpp %d x %d \n", GFX_AUTODETECT_WINDOWED, 8, w, h);
				rest(switchdelay_);
			}
			else
			{
				Z_message("Desired resolution not supported; using %d x %d instead\n", w, h);
				ok = true;
				break;
			}
		}
	}

	if (!ok)
	{
		// give up
		return false;
	}

	screenw_ = SCREEN_W;
	screenh_ = SCREEN_H;

	Backend::palette->applyPaletteToScreen();

	hw_screen_ = screen;
		
	set_display_switch_mode(fullscreen_ ? SWITCH_BACKAMNESIA : SWITCH_BACKGROUND);
	set_display_switch_callback(SWITCH_IN, &onSwitchIn);
	set_display_switch_callback(SWITCH_OUT, &onSwitchOut);

	int nmodes = (int)virtualmodes_.size();
	int newmode = -1;

	// first check for an exact integer match
	for (int i = 0; i < nmodes; i++)
	{
		if (screenw_ % virtualmodes_[i].first == 0)
		{
			int quot = screenw_ / virtualmodes_[i].first;
			if (screenh_ == quot * virtualmodes_[i].second)
			{
				newmode = i;
				break;
			}
		}
	}

	// fall back to the closest resolution that fits
	if (newmode == -1)
	{
		for (int i = 0; i < nmodes; i++)
		{
			if (virtualmodes_[i].first <= screenw_ && virtualmodes_[i].second <= screenh_)
			{
				newmode = i;
				break;
			}
		}
	}

	if (newmode == -1)
		return false;

	curmode_ = newmode;
	if(virtualw_ != virtualmodes_[newmode].first || virtualh_ != virtualmodes_[newmode].second)
	{
		virtualw_ = virtualmodes_[newmode].first;
		virtualh_ = virtualmodes_[newmode].second;
		destroy_bitmap(backbuffer_);
		backbuffer_ = create_bitmap_ex(8, virtualw_, virtualh_);
		clear_to_color(backbuffer_, 0);
	}
	if (native_)
	{
		if (nativebuffer_)
			destroy_bitmap(nativebuffer_);

		nativebuffer_ = create_bitmap_ex(8, SCREEN_W, SCREEN_H);
	}

	screen = backbuffer_;
	showBackBuffer();

	return true;
}

int GraphicsBackend::screenW()
{
	return SCREEN_W;
}

int GraphicsBackend::screenH()
{
	return SCREEN_H;
}

void GraphicsBackend::physicalToVirtual(int &x, int &y)
{
	if (initialized_)
	{
		x = x*virtualScreenW() / screenW();
		y = y*virtualScreenH() / screenH();
	}
}

void GraphicsBackend::virtualToPhysical(int &x, int &y)
{
	if (initialized_)
	{
		x = x*screenW() / virtualScreenW();
		y = y*screenH() / virtualScreenH();
	}
}

void GraphicsBackend::registerSwitchCallbacks(void(*switchin)(), void(*switchout)())
{
	switch_in_func_ = switchin;
	switch_out_func_ = switchout;
	if (initialized_)
	{
		set_display_switch_callback(SWITCH_IN, &onSwitchIn);
		set_display_switch_callback(SWITCH_OUT, &onSwitchOut);
	}
}

const char *GraphicsBackend::videoModeString()
{
	if (!initialized_)
		return "Uninitialized";

	int VidMode = gfx_driver->id;
#ifdef ALLEGRO_DOS

	switch (VidMode)
	{
	case GFX_MODEX:
		return "VGA Mode X";
		break;

	case GFX_VESA1:
		return "VESA 1.x";
		break;

	case GFX_VESA2B:
		return "VESA2 Banked";
		break;

	case GFX_VESA2L:
		return "VESA2 Linear";
		break;

	case GFX_VESA3:
		return "VESA3";
		break;

	default:
		return "Unknown... ?";
		break;
	}

#elif defined(ALLEGRO_WINDOWS)

	switch (VidMode)
	{
	case GFX_DIRECTX:
		return "DirectX Hardware Accelerated";
		break;

	case GFX_DIRECTX_SOFT:
		return "DirectX Software Accelerated";
		break;

	case GFX_DIRECTX_SAFE:
		return "DirectX Safe";
		break;

	case GFX_DIRECTX_WIN:
		return "DirectX Windowed";
		break;

	case GFX_GDI:
		return "GDI";
		break;

	default:
		return "Unknown... ?";
		break;
	}

#elif defined(ALLEGRO_MACOSX)

	switch (VidMode)
	{
	case GFX_SAFE:
		return "MacOS X Safe";
		break;

	case GFX_QUARTZ_FULLSCREEN:
		return "MacOS X Fullscreen Quartz";
		break;

	case GFX_QUARTZ_WINDOW:
		return "MacOS X Windowed Quartz";
		break;

	default:
		return "Unknown... ?";
		break;
	}

#elif defined(ALLEGRO_UNIX)

	switch (VidMode)
	{
	case GFX_AUTODETECT_WINDOWED:
		return "Autodetect Windowed";
		break;

	default:
		return "Unknown... ?";
		break;
	}
#endif
	return "Unknown... ?";
}

int GraphicsBackend::desktopW()
{
	int w, h;
	get_desktop_resolution(&w, &h);
	return w;
}

int GraphicsBackend::desktopH()
{
	int w, h;
	get_desktop_resolution(&w, &h);
	return h;
}