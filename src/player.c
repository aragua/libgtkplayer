#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "player.h"

#include <gst/video/videooverlay.h>

#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#elif defined (GDK_WINDOWING_WAYLAND)
#include <gdk/gdkwayland.h>
#endif

int verbose = 0;
int debug = 1;
#define LOG(msg) if (verbose){printf("[%s:%d] %s - %s\n", __FILE__, __LINE__, __func__, msg);}
#define DBG(msg) if (debug){printf("[%s:%d] %s - %s\n", __FILE__, __LINE__, __func__, msg);}
#define FUNC_ENTER if (verbose){printf("%s\n", __func__);}

/* Common function */

static guintptr get_window_handle(GtkWidget * widget)
{
	GdkWindow *window;
	guintptr window_handle = 0;

	FUNC_ENTER;

	window = gtk_widget_get_window(widget);
	if (!gdk_window_ensure_native(window))
		g_error
		    ("Couldn't create native window needed for GstVideoOverlay!");

	/* Retrieve window handler from GDK */
#if defined (GDK_WINDOWING_WIN32)
	window_handle = (guintptr) GDK_WINDOW_HWND(window);
#elif defined (GDK_WINDOWING_QUARTZ)
	window_handle = gdk_quartz_window_get_nsview(window);
#elif defined (GDK_WINDOWING_X11)
	window_handle = GDK_WINDOW_XID(window);
#elif defined (GDK_WINDOWING_WAYLAND)
	window_handle = gdk_wayland_window_get_wl_surface(window);
#else
#error "NO GDK_WINDOWING supported"
#endif
	return window_handle;
}

static gboolean draw_cb(GtkWidget * widget, cairo_t * cr, PlayerData * data)
{
	FUNC_ENTER;
	if (data->state < GST_STATE_PAUSED) {
		GtkAllocation allocation;

		/* Cairo is a 2D graphics library which we use here to clean the video window.
		 * It is used by GStreamer for other reasons, so it will always be available to us. */
		gtk_widget_get_allocation(widget, &allocation);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
		cairo_fill(cr);
	}

	return FALSE;
}

/******************************************************************************/
/*                             keyboard management                            */
/******************************************************************************/

/* this mapping follow the mplayer key-bindings */
static gboolean
key_press_event_cb(GtkWidget * widget, GdkEventKey * event, gpointer data)
{
	PlayerData *pdata = (PlayerData *) data;

	if (event->state != 0 &&
	    ((event->state & GDK_CONTROL_MASK) || (event->state & GDK_MOD1_MASK)
	     || (event->state & GDK_MOD3_MASK)
	     || (event->state & GDK_MOD4_MASK)))
		return FALSE;

	if (event->type != GDK_KEY_PRESS)
		return FALSE;

	switch (event->keyval) {
	case GDK_KEY_f:{
			/* Toggle fullscreen */
			GtkToggleButton *fs =
			    GTK_TOGGLE_BUTTON(pdata->fullscreen_button);
			gboolean active = !gtk_toggle_button_get_active(fs);
			DBG("GDK_KEY_f pressed");
			gtk_toggle_button_set_active(fs, active);
			break;
		}
	default:
		break;
	}

	return FALSE;
}

/******************************************************************************/
/*                           Fullscreen management                            */
/******************************************************************************/

static void full_realize_cb(GtkWidget * widget, PlayerData * data)
{
	FUNC_ENTER;
	gst_element_set_state(data->playbin, GST_STATE_PAUSED);
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(data->playbin),
					    get_window_handle(widget));
	gst_element_set_state(data->playbin, GST_STATE_PLAYING);
}

/* This function is called when the STOP button is clicked */
static void fullscreen_cb(GtkButton * button, PlayerData * data)
{
	FUNC_ENTER;
	if (!data)
		g_error("No player instantiated");

	if (!data->isfullscreen) {
		GtkWidget *newda;
		DBG("create full screen window");
		data->fullscreen_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		newda = gtk_drawing_area_new();
		g_signal_connect(newda, "realize", G_CALLBACK(full_realize_cb),
				 data);
		g_signal_connect(newda, "draw", G_CALLBACK(draw_cb), data);
		gtk_container_add(GTK_CONTAINER(data->fullscreen_window),
				  newda);
		gtk_window_fullscreen(GTK_WINDOW(data->fullscreen_window));
		g_signal_connect(G_OBJECT(data->fullscreen_window),
				 "key-press-event",
				 G_CALLBACK(key_press_event_cb), data);
		gtk_widget_show_all(data->fullscreen_window);
		data->isfullscreen = TRUE;
	} else {
		DBG("quit  full screen mode");
		gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY
						    (data->playbin),
						    data->window_handle);
		gtk_widget_destroy(data->fullscreen_window);
		data->isfullscreen = FALSE;
	}
}

/******************************************************************************/

static void realize_cb(GtkWidget * widget, PlayerData * data)
{
	FUNC_ENTER;
	if (data)
		data->window_handle = get_window_handle(widget);
}

/* This function is called when the PLAY button is clicked */
static void play_cb(GtkButton * button, PlayerData * data)
{
	FUNC_ENTER;
	if (data->initialized == 0) {
		if (data->uri)
			player_start(data);
		else
            g_printerr ("URI unset.\n");
	}

	if (gst_element_set_state(data->playbin, GST_STATE_PLAYING) ==
	    GST_STATE_CHANGE_FAILURE) {
		g_printerr
		    ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data->playbin);
	}
}

/* This function is called when the PAUSE button is clicked */
static void pause_cb(GtkButton * button, PlayerData * data)
{
	FUNC_ENTER;
	gst_element_set_state(data->playbin, GST_STATE_PAUSED);
}

/* This function is called when the STOP button is clicked */
static void stop_cb(GtkButton * button, PlayerData * data)
{
	FUNC_ENTER;
	gst_element_set_state(data->playbin, GST_STATE_READY);
}

/* This function is called when the slider changes its position. We perform a seek to the
 * new position here. */
static void slider_cb(GtkRange * range, PlayerData * data)
{
	gdouble value = gtk_range_get_value(GTK_RANGE(data->slider));
	FUNC_ENTER;
	gst_element_seek_simple(data->playbin, GST_FORMAT_TIME,
				GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT,
				(gint64) (value * GST_SECOND));
}

/* This creates all the GTK+ widgets that compose our application, and registers the callbacks */
static void create_ui(PlayerData * data)
{
	GtkWidget *main_hbox;	/* HBox to hold the video_window and the stream info text widget */
	GtkWidget *controls, *sliderbox;	/* HBox to hold the buttons and the slider */
	GtkWidget *play_button, *pause_button, *stop_button;	/* Buttons */

	FUNC_ENTER;

	LOG("create drawing area");
	data->video_window = gtk_drawing_area_new();
	g_signal_connect(data->video_window, "realize", G_CALLBACK(realize_cb),
			 data);
	g_signal_connect(data->video_window, "draw", G_CALLBACK(draw_cb), data);

	LOG("create button play");
	play_button = gtk_button_new_with_label("Play");
	g_signal_connect(G_OBJECT(play_button), "clicked", G_CALLBACK(play_cb),
			 data);

	LOG("create button pause");
	pause_button = gtk_button_new_with_label("Pause");
	g_signal_connect(G_OBJECT(pause_button), "clicked",
			 G_CALLBACK(pause_cb), data);

	LOG("create stop button");
	stop_button = gtk_button_new_with_label("Stop");
	g_signal_connect(G_OBJECT(stop_button), "clicked", G_CALLBACK(stop_cb),
			 data);

	LOG("create fullscreen button");
	data->fullscreen_button =
	    gtk_toggle_button_new_with_label("view-fullscreen");
	g_signal_connect(G_OBJECT(data->fullscreen_button), "clicked",
			 G_CALLBACK(fullscreen_cb), data);

	LOG("create slider");
	data->slider =
	    gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 1);
	gtk_scale_set_draw_value(GTK_SCALE(data->slider), 0);
	data->slider_update_signal_id =
	    g_signal_connect(G_OBJECT(data->slider), "value-changed",
			     G_CALLBACK(slider_cb), data);

	LOG("create stream list");
	data->streams_list = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(data->streams_list), FALSE);

	LOG("Add controls");
	controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(controls), play_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(controls), pause_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(controls), stop_button, FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(controls), data->fullscreen_button, FALSE,
			   FALSE, 2);

	LOG("Add sliders");
	sliderbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(sliderbox), data->slider, TRUE, TRUE, 2);

	LOG("Add to main_hbox");
	main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(main_hbox), data->video_window, TRUE, TRUE,
			   0);
	gtk_box_pack_start(GTK_BOX(main_hbox), data->streams_list, FALSE, FALSE,
			   2);

	LOG("Add to main_box");
	data->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(data->main_box), main_hbox, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(data->main_box), controls, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(data->main_box), sliderbox, FALSE, FALSE, 0);
}

/* This function is called periodically to refresh the GUI */
static gboolean refresh_ui(PlayerData * data)
{
	gint64 current = -1;
	FUNC_ENTER;
	/* We do not want to update anything unless we are in the PAUSED or PLAYING states */
	if (data->state < GST_STATE_PAUSED)
		return TRUE;

	/* If we didn't know it yet, query the stream duration */
	if (!GST_CLOCK_TIME_IS_VALID(data->duration)) {
		if (!gst_element_query_duration
		    (data->playbin, GST_FORMAT_TIME, &data->duration)) {
			g_printerr("Could not query current duration.\n");
		} else {
			/* Set the range of the slider to the clip duration, in SECONDS */
			gtk_range_set_range(GTK_RANGE(data->slider), 0,
					    (gdouble) data->duration /
					    GST_SECOND);
		}
	}

	if (gst_element_query_position
	    (data->playbin, GST_FORMAT_TIME, &current)) {
		/* Block the "value-changed" signal, so the slider_cb function is not called
		 * (which would trigger a seek the user has not requested) */
		g_signal_handler_block(data->slider,
				       data->slider_update_signal_id);
		/* Set the position of the slider to the current pipeline positoin, in SECONDS */
		gtk_range_set_value(GTK_RANGE(data->slider),
				    (gdouble) current / GST_SECOND);
		/* Re-enable the signal */
		g_signal_handler_unblock(data->slider,
					 data->slider_update_signal_id);
	}
	return TRUE;
}

/* This function is called when new metadata is discovered in the stream */
static void tags_cb(GstElement * playbin, gint stream, PlayerData * data)
{
	FUNC_ENTER;
	/* We are possibly in a GStreamer working thread, so we notify the main
	 * thread of this event through a message in the bus */
	gst_element_post_message(playbin,
				 gst_message_new_application(GST_OBJECT
							     (playbin),
							     gst_structure_new_empty
							     ("tags-changed")));
}

/* This function is called when an error message is posted on the bus */
static void error_cb(GstBus * bus, GstMessage * msg, PlayerData * data)
{
	GError *err;
	gchar *debug_info;
	FUNC_ENTER;

	/* Print error details on the screen */
	gst_message_parse_error(msg, &err, &debug_info);
	g_printerr("Error received from element %s: %s\n",
		   GST_OBJECT_NAME(msg->src), err->message);
	g_printerr("Debugging information: %s\n",
		   debug_info ? debug_info : "none");
	g_clear_error(&err);
	g_free(debug_info);

	/* Set the pipeline to READY (which stops playback) */
	gst_element_set_state(data->playbin, GST_STATE_READY);
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb(GstBus * bus, GstMessage * msg, PlayerData * data)
{
	FUNC_ENTER;
	g_print("End-Of-Stream reached.\n");
	gst_element_set_state(data->playbin, GST_STATE_READY);
}

/* This function is called when the pipeline changes states. We use it to
 * keep track of the current state. */
static void state_changed_cb(GstBus * bus, GstMessage * msg, PlayerData * data)
{
	GstState old_state, new_state, pending_state;
	// FUNC_ENTER;

	gst_message_parse_state_changed(msg, &old_state, &new_state,
					&pending_state);
	if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->playbin)) {
		data->state = new_state;
		g_print("State set to %s\n",
			gst_element_state_get_name(new_state));
		if (old_state == GST_STATE_READY
		    && new_state == GST_STATE_PAUSED) {
			/* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
			refresh_ui(data);
		}
	}
}

/* Extract metadata from all the streams and write it to the text widget in the GUI */
static void analyze_streams(PlayerData * data)
{
	gint i;
	GstTagList *tags;
	gchar *str, *total_str;
	guint rate;
	gint n_video, n_audio, n_text;
	GtkTextBuffer *text;

	FUNC_ENTER;

	/* Clean current contents of the widget */
	text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->streams_list));
	gtk_text_buffer_set_text(text, "", -1);

	/* Read some properties */
	g_object_get(data->playbin, "n-video", &n_video, NULL);
	g_object_get(data->playbin, "n-audio", &n_audio, NULL);
	g_object_get(data->playbin, "n-text", &n_text, NULL);

	for (i = 0; i < n_video; i++) {
		tags = NULL;
		/* Retrieve the stream's video tags */
		g_signal_emit_by_name(data->playbin, "get-video-tags", i,
				      &tags);
		if (tags) {
			total_str = g_strdup_printf("video stream %d:\n", i);
			gtk_text_buffer_insert_at_cursor(text, total_str, -1);
			g_free(total_str);
			gst_tag_list_get_string(tags, GST_TAG_VIDEO_CODEC,
						&str);
			total_str =
			    g_strdup_printf("  codec: %s\n",
					    str ? str : "unknown");
			gtk_text_buffer_insert_at_cursor(text, total_str, -1);
			g_free(total_str);
			g_free(str);
			gst_tag_list_free(tags);
		}
	}

	for (i = 0; i < n_audio; i++) {
		tags = NULL;
		/* Retrieve the stream's audio tags */
		g_signal_emit_by_name(data->playbin, "get-audio-tags", i,
				      &tags);
		if (tags) {
			total_str = g_strdup_printf("\naudio stream %d:\n", i);
			gtk_text_buffer_insert_at_cursor(text, total_str, -1);
			g_free(total_str);
			if (gst_tag_list_get_string
			    (tags, GST_TAG_AUDIO_CODEC, &str)) {
				total_str =
				    g_strdup_printf("  codec: %s\n", str);
				gtk_text_buffer_insert_at_cursor(text,
								 total_str, -1);
				g_free(total_str);
				g_free(str);
			}
			if (gst_tag_list_get_string
			    (tags, GST_TAG_LANGUAGE_CODE, &str)) {
				total_str =
				    g_strdup_printf("  language: %s\n", str);
				gtk_text_buffer_insert_at_cursor(text,
								 total_str, -1);
				g_free(total_str);
				g_free(str);
			}
			if (gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &rate)) {
				total_str =
				    g_strdup_printf("  bitrate: %d\n", rate);
				gtk_text_buffer_insert_at_cursor(text,
								 total_str, -1);
				g_free(total_str);
			}
			gst_tag_list_free(tags);
		}
	}

	for (i = 0; i < n_text; i++) {
		tags = NULL;
		/* Retrieve the stream's subtitle tags */
		g_signal_emit_by_name(data->playbin, "get-text-tags", i, &tags);
		if (tags) {
			total_str =
			    g_strdup_printf("\nsubtitle stream %d:\n", i);
			gtk_text_buffer_insert_at_cursor(text, total_str, -1);
			g_free(total_str);
			if (gst_tag_list_get_string
			    (tags, GST_TAG_LANGUAGE_CODE, &str)) {
				total_str =
				    g_strdup_printf("  language: %s\n", str);
				gtk_text_buffer_insert_at_cursor(text,
								 total_str, -1);
				g_free(total_str);
				g_free(str);
			}
			gst_tag_list_free(tags);
		}
	}
}

/* This function is called when an "application" message is posted on the bus.
 * Here we retrieve the message posted by the tags_cb callback */
static void application_cb(GstBus * bus, GstMessage * msg, PlayerData * data)
{
	FUNC_ENTER;
	if (g_strcmp0
	    (gst_structure_get_name(gst_message_get_structure(msg)),
	     "tags-changed") == 0) {
		/* If the message is the "tags-changed" (only one we are currently issuing), update
		 * the stream info GUI */
		analyze_streams(data);
	}
}

static gint create_playbin(PlayerData * data)
{
	FUNC_ENTER;

	/* Create the elements */
	data->playbin = gst_element_factory_make("playbin", "playbin");

	if (!data->playbin) {
		g_printerr("Not all elements could be created.\n");
		return -1;
	}

	/* Connect to interesting signals in playbin */
	g_signal_connect(G_OBJECT(data->playbin), "video-tags-changed",
			 (GCallback) tags_cb, &data);
	g_signal_connect(G_OBJECT(data->playbin), "audio-tags-changed",
			 (GCallback) tags_cb, &data);
	g_signal_connect(G_OBJECT(data->playbin), "text-tags-changed",
			 (GCallback) tags_cb, &data);
	return 0;
}

static void init_bus(PlayerData * data)
{
	GstBus *bus;

	FUNC_ENTER;

	if (data) {
		/* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
		bus = gst_element_get_bus(data->playbin);
		gst_bus_add_signal_watch(bus);
		g_signal_connect(G_OBJECT(bus), "message::error",
				 (GCallback) error_cb, data);
		g_signal_connect(G_OBJECT(bus), "message::eos",
				 (GCallback) eos_cb, data);
		g_signal_connect(G_OBJECT(bus), "message::state-changed",
				 (GCallback) state_changed_cb, data);
		g_signal_connect(G_OBJECT(bus), "message::application",
				 (GCallback) application_cb, data);
		gst_object_unref(bus);
	}
}

gint player_init(PlayerData * data)
{
	FUNC_ENTER;
	if (!data)
		return -1;

	/* Initialize GStreamer */
	gst_init(NULL, NULL);

	/* Initialize our data structure */
	memset(data, 0, sizeof(PlayerData));
	data->duration = GST_CLOCK_TIME_NONE;

	create_ui(data);

	return 0;
}

gint player_start(PlayerData * data)
{
	FUNC_ENTER;

	create_playbin(data);
	gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(data->playbin),
					    data->window_handle);

	if (!data->uri) {
		g_warning("data->uri is empty. Fix this case\n");
        return -1;
    }

    g_object_set(data->playbin, "uri", data->uri, NULL);

	init_bus(data);

	g_timeout_add_seconds(1, (GSourceFunc) refresh_ui, data);

	data->initialized = 1;

    if (gst_element_set_state(data->playbin, GST_STATE_PLAYING) ==
	    GST_STATE_CHANGE_FAILURE) {
		g_printerr
		    ("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data->playbin);
	}

	return 0;
}

void player_change_uri(PlayerData * data, const char *uri)
{
	FUNC_ENTER;
	if (data) {
		if (data->initialized)
			gst_element_set_state(data->playbin, GST_STATE_READY);
		data->duration = GST_CLOCK_TIME_NONE;
		if (uri) {
			if (data->uri)
				free(data->uri);
			data->uri = strdup(uri);
			if (data->initialized)
			    g_object_set(data->playbin, "uri", data->uri, NULL);
		}
	}
}

void player_stop(PlayerData * data)
{
	FUNC_ENTER;
	stop_cb(NULL, data);
}

void player_free(PlayerData * data)
{
	FUNC_ENTER;
	/* Free resources */
	gst_element_set_state(data->playbin, GST_STATE_NULL);
	gst_object_unref(data->playbin);
}
