#include "zapp.h"
#include "zc_alleg.h"
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <allegro5/allegro_windows.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef ALLEGRO_SDL
#include <SDL.h>
#endif

#ifdef HAS_SENTRY
#define SENTRY_BUILD_STATIC 1
#include "sentry.h"

void sentry_atexit()
{
    sentry_close();
}
#endif

static App app_id = App::undefined;

bool is_in_osx_application_bundle()
{
#ifdef __APPLE__
    return std::filesystem::current_path().string().find("/ZeldaClassic.app/") != std::string::npos;
#else
    return false;
#endif
}

void common_main_setup(App id, int argc, char **argv)
{
    app_id = id;

#ifdef HAS_SENTRY
    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, "https://133f371c936a4bc4bddec532b1d1304a@o1313474.ingest.sentry.io/6563738");
    sentry_options_set_release(options, "zelda-classic@" SENTRY_RELEASE_TAG);
    sentry_options_set_handler_path(options, "crashpad_handler.exe");
    sentry_init(options);
    sentry_set_tag("app", ZC_APP_NAME);
    atexit(sentry_atexit);
#endif

    // This allows for opening a binary from Finder and having ZC be in its expected
    // working directory (the same as the binary). Only used when not inside an application bundle,
    // and only for testing purposes really. See comment about `SKIP_APP_BUNDLE` in buildpack_osx.sh
#ifdef __APPLE__
    if (!is_in_osx_application_bundle() && argc > 0) {
        chdir(std::filesystem::path(argv[0]).parent_path().c_str());
    }
#endif

#ifdef ALLEGRO_SDL
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);
#endif
}

App get_app_id()
{
    return app_id;
}

// https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
// https://mariusbancila.ro/blog/2021/05/19/how-to-build-high-dpi-aware-native-desktop-applications/
// On windows, Allegro sets DPI awareness at runtime:
//   https://github.com/liballeg/allegro5/blob/0d9271d381c33ab1096b424beadfd82050aaa2d3/src/win/wsystem.c#L142
// But it's better to do it in the application manifest, hence `set_property(TARGET zelda PROPERTY VS_DPI_AWARE "PerMonitor")`
double zc_get_monitor_scale()
{
#ifdef _WIN32
	if (all_get_display())
	{
		// why not use al_get_monitor_dpi? Because allegro uses GetDpiForMonitor,
		// which is an older API.
		HWND hwnd = al_get_win_window_handle(all_get_display());
		int dpi = GetDpiForWindow(hwnd);
		return dpi / 96.0;
	}
	else
	{
		HDC hdc = GetDC(NULL);
		int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
		ReleaseDC(NULL, hdc);
		return dpi / 96.0;
	}
#else
	return al_get_monitor_dpi(0) / 96.0;
#endif
}

extern bool DragAspect;
static void doAspectResize()
{
	if (!DragAspect || all_get_fullscreen_flag())
		return;

	static int prev_width = 0, prev_height = 0;

	if (prev_width == 0 || prev_height == 0)
	{
		prev_width = al_get_display_width(all_get_display());
		prev_height = al_get_display_height(all_get_display());
	}

	if (prev_width != al_get_display_width(all_get_display()) || prev_height != al_get_display_height(all_get_display()))
	{
		bool width_first = true;
		
		if (abs(prev_width - al_get_display_width(all_get_display())) < abs(prev_height - al_get_display_height(all_get_display()))) width_first = false;
		
		if (width_first)
		{
			al_resize_display(all_get_display(), al_get_display_width(all_get_display()), al_get_display_width(all_get_display())*0.75);
		}
		else
		{
			al_resize_display(all_get_display(), al_get_display_height(all_get_display())/0.75, al_get_display_height(all_get_display()));
		}
	}

	prev_width = al_get_display_width(all_get_display());
	prev_height = al_get_display_height(all_get_display());
}

void zc_process_display_events()
{
	all_process_display_events();
	// TODO: should do this only in response to a resize event
	doAspectResize();
}

static ALLEGRO_FONT* builtin_font;

void zc_update_builtin_font()
{
	if (!builtin_font)
		return;
	
	// Only a problem for fullscreen (at least, directx).
	if (!all_get_fullscreen_flag())
		return;
	
	al_destroy_font(builtin_font);
	builtin_font = al_create_builtin_font();
}

ALLEGRO_FONT* zc_get_builtin_font()
{
	if (!builtin_font)
		builtin_font = al_create_builtin_font();
	return builtin_font;
}