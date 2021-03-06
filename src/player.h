#pragma once

#include <gtk/gtk.h>
#include <gst/gst.h>

/* Structure to contain all our information, so we can pass it around */
typedef struct _PlayerData {
	/* main container data */
	GtkWidget *main_box;
	guintptr window_handle;
	GtkWidget *video_window;	/* The drawing area where the video will be shown */
	GtkWidget *slider;	/* Slider widget to keep track of current position */
	GtkWidget *streams_list;	/* Text widget to display info about the streams */
	gulong slider_update_signal_id;	/* Signal ID for the slider update signal */
	gboolean isfullscreen;
	GtkWidget *fullscreen_window;
	GtkWidget *play_button;
	GtkWidget *fullscreen_button;
	/* player data */
	gint64 duration;	/* Duration of the clip, in nanoseconds */
	char *uri;
	GstElement *playbin;	/* Our one and only pipeline */
	GstState state;		/* Current state of the pipeline */
} PlayerData;

gint player_new(PlayerData * data);
gint player_set_uri(PlayerData * data, const char *uri);
gint player_start(PlayerData * data);
void player_stop(PlayerData * data);
void player_free(PlayerData * data);
