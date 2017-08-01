#include "gui.h"

GtkWidget *main_window;
GtkWidget *dialed_number;
GtkWidget *status_bar;
guint status_bar_context_id;
GtkWidget *dial_button;
GtkWidget *hangup_button;

static GtkWidget *
gui_init_tab(GtkWidget *notebook, char *title)
{
	GtkWidget *tab = gtk_vbox_new(FALSE, BORDER_WIDTH);

	gtk_container_set_border_width(
		GTK_CONTAINER(tab), 3 * BORDER_WIDTH);
	gtk_widget_show(tab);
	gtk_container_add(GTK_CONTAINER(notebook), tab);
	gtk_notebook_set_tab_label_text(
		GTK_NOTEBOOK(notebook), tab, title);

	return tab;
}

static GtkWidget *
gui_init_table(GtkWidget *parent, int rows, int columns)
{
	GtkWidget *table = gtk_table_new(rows, columns, TRUE);
	gtk_container_set_border_width(
		GTK_CONTAINER(table), BORDER_WIDTH);
	gtk_widget_show(table);
	gtk_container_add(GTK_CONTAINER(parent), table);
	return table;
}

static GtkWidget *
gui_init_dialpad_button(GtkWidget *table, char *label,
			int cstart, int cend, int rstart, int rend)
{
	GtkWidget *button = gtk_button_new_with_label(label);
	gtk_widget_show(button);
	gtk_table_attach(GTK_TABLE(table), button,
			 cstart, cend, rstart, rend,
			 (GtkAttachOptions) GTK_EXPAND | GTK_FILL,
			 (GtkAttachOptions) GTK_EXPAND | GTK_FILL,
			 BORDER_WIDTH, BORDER_WIDTH);
        g_signal_connect(
		button, "button-press-event",
		(GCallback) callback_dialpad_button_event, label);
	return button;
}

static void
gui_init_dialpad_tab(GtkWidget *notebook)
{
	GtkWidget *tab, *vbox, *align, *table;

	tab = gui_init_tab(notebook, "Dialpad");

        vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
        gtk_container_set_border_width(
		GTK_CONTAINER(vbox), BORDER_WIDTH);
        gtk_widget_show(vbox);
        gtk_container_add(GTK_CONTAINER(tab), vbox);

	align = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_container_set_border_width(
		GTK_CONTAINER(align), 3 * BORDER_WIDTH);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(vbox), align);

	dialed_number = gtk_entry_new();
	gtk_widget_show(dialed_number);
	gtk_container_add(GTK_CONTAINER(align), dialed_number);

	align = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_container_set_border_width(
		GTK_CONTAINER(align), BORDER_WIDTH);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(vbox), align);

	table = gui_init_table(align, 4, 3);

	gui_init_dialpad_button(table, "7", 0, 1, 0, 1);
	gui_init_dialpad_button(table, "8", 1, 2, 0, 1);
	gui_init_dialpad_button(table, "9", 2, 3, 0, 1);

	gui_init_dialpad_button(table, "4", 0, 1, 1, 2);
	gui_init_dialpad_button(table, "5", 1, 2, 1, 2);
	gui_init_dialpad_button(table, "6", 2, 3, 1, 2);

	gui_init_dialpad_button(table, "1", 0, 1, 2, 3);
	gui_init_dialpad_button(table, "2", 1, 2, 2, 3);
	gui_init_dialpad_button(table, "3", 2, 3, 2, 3);

	gui_init_dialpad_button(table, "*", 0, 1, 3, 4);
	gui_init_dialpad_button(table, "0", 1, 2, 3, 4);
	gui_init_dialpad_button(table, "#", 2, 3, 3, 4);

	align = gtk_alignment_new(0.5, 0.5, 1, 1);
	gtk_container_set_border_width(
		GTK_CONTAINER(align), BORDER_WIDTH);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(vbox), align);

	table = gui_init_table(align, 2, 2);

	dial_button =
		gui_init_dialpad_button(table, "Dial", 0, 1, 0, 1);
	hangup_button =
		gui_init_dialpad_button(table, "Hangup", 1, 2, 0, 1);
	gui_init_dialpad_button(table, "Clear", 0, 1, 1, 2);
	gui_init_dialpad_button(table, "Settings", 1, 2, 1, 2);

}

static void
gui_init_about_tab(GtkWidget *notebook)
{
	GtkWidget *tab, *align, *vbox, *label;
        char version[BUFSIZE];

	tab = gui_init_tab(notebook, "About");

	align = gtk_alignment_new(0.5, 0.5, 0, 0);
	gtk_container_set_border_width(GTK_CONTAINER(align), BORDER_WIDTH);
	gtk_widget_show(align);
	gtk_container_add(GTK_CONTAINER(tab), align);

	vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), BORDER_WIDTH);
	gtk_widget_show(vbox);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	/* Title label */
	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), "<b>Cornfed SIP User Agent</b>");
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox), label);

	/* Version label */
	memset(version, 0, BUFSIZE);
	sprintf(version, "<i>Version %d.%d.%d</i>",
		CORNFEDSIPUA_VERSION, MAJOR_RELEASE, MINOR_RELEASE);
	label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), version);
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox), label);

	/* Copyright data label */
	label = gtk_label_new("Copyright (C) 2004-2008");
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox), label);

	/* Copyright label */
	label = gtk_label_new("Cornfed Systems LLC");
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox), label);

	/* Author label */
	label = gtk_label_new("Written by Frank W. Miller");
	gtk_widget_show(label);
	gtk_container_add(GTK_CONTAINER(vbox), label);
}

void
gui_init(int window_width, int window_height)
{
	GdkPixbuf *pixbuf;
	GError *gerror = NULL;
	GtkWidget *main_window_vbox;
	GtkWidget *main_window_notebook;
	GtkWidget *status_hbox;

	/* main_window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(
		GTK_WINDOW(main_window), window_width, window_height);
	gtk_window_set_title(
		GTK_WINDOW(main_window), "Cornfed SIP User Agent");

	pixbuf = gdk_pixbuf_new_from_file(ICON_SMALL, &gerror);
	if (pixbuf != NULL)
		gtk_window_set_icon(GTK_WINDOW(main_window), pixbuf);

	gtk_widget_show(main_window);
	gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
			   GTK_SIGNAL_FUNC(callback_delete_event), NULL);

	/* main_window_vbox */
	main_window_vbox = gtk_vbox_new(FALSE, BORDER_WIDTH);
	gtk_container_set_border_width(
		GTK_CONTAINER(main_window_vbox), BORDER_WIDTH);
	gtk_widget_show(main_window_vbox);
	gtk_container_add(GTK_CONTAINER(main_window), main_window_vbox);

	/* main_window_notebook */
	main_window_notebook = gtk_notebook_new();
	gtk_widget_show(main_window_notebook);
	gtk_container_add(
		GTK_CONTAINER(main_window_vbox), main_window_notebook);

	/* status_hbox */
	status_hbox = gtk_hbox_new(FALSE, BORDER_WIDTH);
	gtk_widget_show(status_hbox);
	gtk_box_pack_start(
		GTK_BOX(main_window_vbox), status_hbox, FALSE, TRUE,
		BORDER_WIDTH);

	/* status_bar */
	status_bar = gtk_statusbar_new();
	status_bar_context_id = gtk_statusbar_get_context_id(
		GTK_STATUSBAR(status_bar), "Status");
	gtk_statusbar_set_has_resize_grip(
		GTK_STATUSBAR(status_bar), FALSE);
	gtk_statusbar_push(
		GTK_STATUSBAR(status_bar), status_bar_context_id,
		"Cornfed SIP User Agent");
	gtk_widget_set_usize(status_bar, 96, 0);
	gtk_widget_show(status_bar);
	gtk_container_add(GTK_CONTAINER(status_hbox), status_bar);

	gui_init_dialpad_tab(main_window_notebook);
	gui_init_about_tab(main_window_notebook);
}
