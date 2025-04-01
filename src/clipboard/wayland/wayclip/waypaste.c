#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "protocol/wlr-data-control-unstable-v1-client-protocol.h"
#include "common.h"

struct wl_display *g_display;

struct zwlr_data_control_offer_v1 *acceptedoffer_zwlr = NULL;
struct wl_data_offer *acceptedoffer_wl = NULL;
int g_fd;
int pipes[2];

// ---------------- zwlr v1 -------------------------
static void
receive_zwlr(int cond, struct zwlr_data_control_offer_v1 *offer)
{
	if (cond && acceptedoffer_zwlr == offer) {
		zwlr_data_control_offer_v1_receive(offer, options.type, pipes[1]);
		wl_display_roundtrip(g_display);
		close(pipes[1]);
                ftruncate(g_fd, 0);

		copyfd(pipes[0], g_fd);
		close(pipes[0]);

		if (pipe(pipes) == -1)
			wc_die("failed to create pipe");

		// exit(0);
	}

	if (acceptedoffer_zwlr)
		zwlr_data_control_offer_v1_destroy(acceptedoffer_zwlr);

	acceptedoffer_zwlr = NULL;
}

void
offer_offer_zwlr(void *data, struct zwlr_data_control_offer_v1 *offer, const char *mime_type)
{
	if (acceptedoffer_zwlr)
		return;

	if (strcmp(mime_type, options.type) == 0)
		acceptedoffer_zwlr = offer;
}

static const struct zwlr_data_control_offer_v1_listener offer_listener_zwlr = {
	.offer = offer_offer_zwlr,
};

void
control_data_offer_zwlr(void *data, struct zwlr_data_control_device_v1 *device, struct zwlr_data_control_offer_v1 *offer)
{
	zwlr_data_control_offer_v1_add_listener(offer, &offer_listener_zwlr, NULL);
}

void
control_data_selection_zwlr(void *data, struct zwlr_data_control_device_v1 *device, struct zwlr_data_control_offer_v1 *offer)
{
	if (offer)
		receive_zwlr(!options.primary, offer);
}

void
control_data_primary_selection_zwlr(void *data, struct zwlr_data_control_device_v1 *device, struct zwlr_data_control_offer_v1 *offer)
{
	if (offer)
		receive_zwlr(options.primary, offer);
}

static const struct zwlr_data_control_device_v1_listener device_listener_zwlr = {
	.data_offer = control_data_offer_zwlr,
	.selection = control_data_selection_zwlr,
	.primary_selection = control_data_primary_selection_zwlr,
};

// ---------------- wayland -------------------------
static void
receive_wl(int cond, struct wl_data_offer *offer)
{
	if (cond && acceptedoffer_wl == offer) {
		wl_data_offer_receive(offer, options.type, pipes[1]);
		wl_display_roundtrip(g_display);
		close(pipes[1]);
                ftruncate(g_fd, 0);

		copyfd(pipes[0], g_fd);
		close(pipes[0]);

		if (pipe(pipes) == -1)
			wc_die("failed to create pipe");

		// exit(0);
	}

	if (acceptedoffer_wl)
		wl_data_offer_destroy(acceptedoffer_wl);

	acceptedoffer_wl = NULL;
}

void
offer_offer_wl(void *data, struct wl_data_offer *offer, const char *mime_type)
{
	if (acceptedoffer_wl)
		return;

	if (strcmp(mime_type, options.type) == 0)
		acceptedoffer_wl = offer;
}

static const struct wl_data_offer_listener offer_listener_wl = {
	.offer = offer_offer_wl,
};
void
control_data_offer_wl(void *data, struct wl_data_device *device, struct wl_data_offer *offer)
{
	wl_data_offer_add_listener(offer, &offer_listener_wl, NULL);
}

void
control_data_selection_wl(void *data, struct wl_data_device *device, struct wl_data_offer *offer)
{
	if (offer)
		receive_wl(!options.primary, offer);
}

static const struct wl_data_device_listener device_listener_wl = {
        .data_offer = control_data_offer_wl,
	.selection = control_data_selection_wl,
};

void
main_waypaste(struct wl_display *display, const int fd)
{
        g_display = display;
        g_fd = fd;

        struct wl_registry *const registry = wl_display_get_registry(display);
	if (registry == NULL)
		wc_die("failed to get registry");

	wl_registry_add_listener(registry, &registry_listener, NULL);

	wl_display_roundtrip(display);
	if (options.seat)
		wl_display_roundtrip(display);

	if (seat == NULL)
		wc_die("failed to bind to seat interface");

	if (data_control_manager.zwlr == NULL || data_control_manager.wl)
		wc_die("failed to bind to data_control_manager interface");

	if (pipe(pipes) == -1)
		wc_die("failed to create pipe");

        if (data_control_manager.zwlr)
        {
                struct zwlr_data_control_device_v1 *device = zwlr_data_control_manager_v1_get_data_device(data_control_manager.zwlr, seat);
                if (device == NULL)
		        wc_die("data device is null");

                zwlr_data_control_device_v1_add_listener(device, &device_listener_zwlr, NULL);
        }
        else
        {
                struct wl_data_device *device = wl_data_device_manager_get_data_device(data_control_manager.wl, seat);
                if (device == NULL)
		        wc_die("data device is null");

                wl_data_device_add_listener(device, &device_listener_wl, NULL);
        }
}

// vim:shiftwidth=8
