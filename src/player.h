#pragma once

#include <gtk/gtk.h>
#include <gst/gst.h>

/* Structure to contain all our information, so we can pass it around */
typedef struct _PlayerData {
	gint64 duration;	/* Duration of the clip, in nanoseconds */
	gint initialized;
	char *uri;
	GstElement *playbin;	/* Our one and only pipeline */
	guintptr window_handle;
	GtkWidget *video_window;	/* The drawing area where the video will be shown */
	GtkWidget *main_box;
	GtkWidget *slider;	/* Slider widget to keep track of current position */
	GtkWidget *streams_list;	/* Text widget to display info about the streams */
	gulong slider_update_signal_id;	/* Signal ID for the slider update signal */

	GstState state;		/* Current state of the pipeline */
	gboolean isfullscreen;
	GtkWidget *fullscreen_window;
	GtkWidget *fullscreen_button;
} PlayerData;

gint player_init(PlayerData * data);
gint player_start(PlayerData * data);
void player_change_uri(PlayerData * data, const char *uri);
void player_stop(PlayerData * data);
void player_free(PlayerData * data);
