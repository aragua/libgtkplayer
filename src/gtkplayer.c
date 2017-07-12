#include "player.h"

/* This function is called when the main window is closed */
static void delete_event_cb(GtkWidget * widget, GdkEvent * event,
			    PlayerData * data)
{
	player_stop(data);
	gtk_main_quit();
}

static gint repeats = 2;
static gboolean fullscreen = FALSE;
static gboolean verbose = FALSE;
static gboolean dontstart = FALSE;
static gchar * uri = "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm";

static GOptionEntry entries[] =
{
  { "repeat", 'r', 0, G_OPTION_ARG_INT, &repeats, "Average over N repetitions", "N" },
  { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "Be verbose", NULL },
  { "fullscreen", 'f', 0, G_OPTION_ARG_NONE, &fullscreen, "Set fullscreen", NULL },
  { "uri", 'u', 0, G_OPTION_ARG_STRING, &uri, "Set uri", NULL },
  { "dontstart", 0, 0, G_OPTION_ARG_NONE, &dontstart, "Don't start video immediately", NULL },
  { NULL }
};

static int init (int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;

    context = g_option_context_new ("- gtk player");
    g_option_context_add_main_entries (context, entries, NULL);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if (!g_option_context_parse (context, &argc, &argv, &error))
    {
        g_print ("option parsing failed: %s\n", error->message);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
	PlayerData data;
	GtkWidget *main_window, *box;

    /* Initialize GTK */
	gtk_init(&argc, &argv);

    if (init(argc,argv) == -1)
        return -1;

    g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	player_init(&data);

    player_change_uri(&data, uri);

	g_signal_connect(G_OBJECT(main_window), "delete-event",
			 G_CALLBACK(delete_event_cb), &data);
	gtk_container_add(GTK_CONTAINER(main_window), data.main_box);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 640, 480);
	gtk_widget_show_all(main_window);

        if (!dontstart)
        player_start(&data,uri);

	/* Start the GTK main loop. We will not regain control until gtk_main_quit is called. */
	gtk_main();

	player_free(&data);
	return 0;
}
