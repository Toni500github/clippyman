#define _POSIX_C_SOURCE 200809L

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-server-core.h>
#include <unistd.h>

#include "protocol/wlr-data-control-unstable-v1-client-protocol.h"
#include "common.h"

struct wl_registry *registry;
int temp;

bool running = true;

// ---------------- zwlr v1 -------------------------
void
data_source_send_zwlr(void *data, struct zwlr_data_control_source_v1 *source, const char *mime_type, int32_t fd)
{
	lseek(temp, 0, SEEK_SET);

	copyfd(fd, temp);
	close(fd);
}

void
data_source_cancelled_zwlr(void *data, struct zwlr_data_control_source_v1 *source)
{
	running = 0;
        zwlr_data_control_source_v1_destroy(source);
        close(temp);
}

static const struct zwlr_data_control_source_v1_listener data_source_listener_zwlr = {
	.send = data_source_send_zwlr,
	.cancelled = data_source_cancelled_zwlr,
};

// ---------------- wayland -------------------------
void
data_source_send_wl(void *data, struct wl_data_source *source, const char *mime_type, int32_t fd)
{
	lseek(temp, 0, SEEK_SET);

	copyfd(fd, temp);
	close(fd);
}

void
data_source_cancelled_wl(void *data, struct wl_data_source *source)
{
	running = 0;
        wl_data_source_destroy(source);
        close(temp);
}

static const struct wl_data_source_listener data_source_listener_wl = {
	.send = data_source_send_wl,
	.cancelled = data_source_cancelled_wl,
};

const char *const tempname = "/waycopy-buffer-XXXXXX";

int
main_waycopy(struct wl_display *display, struct wc_options options, const int fd)
{
	char path[PATH_MAX] = {0};
	char *ptr = getenv("TMPDIR");
	if (ptr == NULL)
		strcpy(path, "/tmp");
	else {
		if (strlen(ptr) > PATH_MAX - strlen(tempname))
			wc_die("TMPDIR has too long of a path");

		strcpy(path, ptr);
	}

	strncat(path, tempname, PATH_MAX - 1);
	temp = mkstemp(path);
	if (temp == -1)
		wc_die("failed to create temporary file for copy buffer");

	if (unlink(path) == -1)
		wc_die("failed to remove temporary file");
	copyfd(fd, temp);
	close(fd);

        registry = wl_display_get_registry(display);
	if (registry == NULL)
		wc_die("failed to get registry");

	wl_registry_add_listener(registry, &registry_listener, NULL);

	wl_display_roundtrip(display);
	if (options.seat)
		wl_display_roundtrip(display);

	if (seat == NULL)
		wc_die("failed to bind to seat interface");

	if (data_control_manager->zwlr == NULL || data_control_manager->wl == NULL)
		wc_die("failed to bind to data_control_manager interface");

        if (data_control_manager->zwlr)
        {
                struct zwlr_data_control_device_v1 *device = zwlr_data_control_manager_v1_get_data_device(data_control_manager->zwlr, seat);
                if (device == NULL)
		        wc_die("data device is null");

                struct zwlr_data_control_source_v1 *source = zwlr_data_control_manager_v1_create_data_source(data_control_manager->zwlr);
                if (source == NULL)
		        wc_die("source device is null");

                zwlr_data_control_source_v1_offer(source, options.type);
	        zwlr_data_control_source_v1_add_listener(source, &data_source_listener_zwlr, NULL);
	        if (options.primary)
		        zwlr_data_control_device_v1_set_primary_selection(device, source);
	        else
		        zwlr_data_control_device_v1_set_selection(device, source);
        }
        else
        {
                struct wl_data_device *device = wl_data_device_manager_get_data_device(data_control_manager->wl, seat);
                if (device == NULL)
		        wc_die("data device is null");

                struct wl_data_source *source = wl_data_device_manager_create_data_source(data_control_manager->wl);
                if (source == NULL)
		        wc_die("source device is null");

                wl_data_source_offer(source, options.type);
	        wl_data_source_add_listener(source, &data_source_listener_wl, NULL);
                wl_data_device_set_selection(device, source,  wl_display_get_serial(display));
        }

	return running;
}

// vim:shiftwidth=8
