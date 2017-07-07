#include "player.h"

/* This function is called when the main window is closed */
static void delete_event_cb(GtkWidget * widget, GdkEvent * event,
			    PlayerData * data)
{
	player_stop(data);
	gtk_main_quit();
}

int main(int argc, char *argv[])
{
	PlayerData data;
	GtkWidget *main_window, *box;

	/* Initialize GTK */
	gtk_init(&argc, &argv);

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	player_init(&data);

	if (argc > 1)
		player_change_uri(&data, argv[1]);
	else
		player_change_uri(&data,
				  "https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm");

	g_signal_connect(G_OBJECT(main_window), "delete-event",
			 G_CALLBACK(delete_event_cb), &data);
	gtk_container_add(GTK_CONTAINER(main_window), data.main_box);
	gtk_window_set_default_size(GTK_WINDOW(main_window), 640, 480);
	gtk_widget_show_all(main_window);

	/* Start the GTK main loop. We will not regain control until gtk_main_quit is called. */
	gtk_main();

	player_free(&data);
	return 0;
}
