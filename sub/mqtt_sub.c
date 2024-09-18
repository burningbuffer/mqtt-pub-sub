#include <gtk/gtk.h>
#include <mosquitto.h>

static GtkWidget *entry_host;
static GtkWidget *entry_port;
static GtkWidget *entry_keepalive;
static GtkWidget *entry_username;
static GtkWidget *entry_password;
static GtkWidget *entry_topic;
static GtkWidget *textview_messages;
static struct mosquitto *mosq;

// Function to append received message to the text view
static void append_message_to_textview(const gchar *message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_messages));
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, message, -1);
    gtk_text_buffer_insert(buffer, &iter, "\n", -1);
}

// Callback function to handle received messages
void on_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    gchar *msg = g_strdup_printf("Received message on topic %s: %s", message->topic, (char *)message->payload);
    g_idle_add((GSourceFunc)append_message_to_textview, msg);
}

// Function to connect and subscribe to the MQTT topic
static void on_subscribe_button_clicked(GtkButton *button, gpointer user_data) {
    const gchar *host = gtk_entry_get_text(GTK_ENTRY(entry_host));
    const gchar *port_str = gtk_entry_get_text(GTK_ENTRY(entry_port));
    const gchar *keepalive_str = gtk_entry_get_text(GTK_ENTRY(entry_keepalive));
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    const gchar *topic = gtk_entry_get_text(GTK_ENTRY(entry_topic));

    int port = atoi(port_str);
    int keepalive = atoi(keepalive_str);

    // Initialize the Mosquitto library
    mosquitto_lib_init();

    // Create a new Mosquitto client instance
    mosq = mosquitto_new(NULL, true, NULL);
    
    if (!mosq) {
        g_print("Failed to create Mosquitto instance\n");
        return;
    }

    // Set username and password if provided
    if (username && password) {
        if (mosquitto_username_pw_set(mosq, username, password)) {
            g_print("Failed to set username/password\n");
            return;
        }
    }

    // Set the message callback
    mosquitto_message_callback_set(mosq, on_message_callback);

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, keepalive)) {
        g_print("Failed to connect to broker\n");
        return;
    }

    // Subscribe to the topic
    if (mosquitto_subscribe(mosq, NULL, topic, 0)) {
        g_print("Failed to subscribe to topic\n");
        return;
    }

    // Start the Mosquitto network loop in a separate thread
    mosquitto_loop_start(mosq);
}

// GUI setup and application activation
static void activate (GtkApplication* app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label_host;
    GtkWidget *label_port;
    GtkWidget *label_keepalive;
    GtkWidget *label_username;
    GtkWidget *label_password;
    GtkWidget *label_topic;
    GtkWidget *button_subscribe;
    GtkWidget *scroll_window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MQTT Subscriber");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    label_host = gtk_label_new("Host:");
    gtk_grid_attach(GTK_GRID(grid), label_host, 0, 0, 1, 1);
    entry_host = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_host, 1, 0, 1, 1);

    label_port = gtk_label_new("Port:");
    gtk_grid_attach(GTK_GRID(grid), label_port, 0, 1, 1, 1);
    entry_port = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_port, 1, 1, 1, 1);

    label_keepalive = gtk_label_new("Keepalive:");
    gtk_grid_attach(GTK_GRID(grid), label_keepalive, 0, 2, 1, 1);
    entry_keepalive = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_keepalive, 1, 2, 1, 1);

    label_username = gtk_label_new("Username:");
    gtk_grid_attach(GTK_GRID(grid), label_username, 0, 3, 1, 1);
    entry_username = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_username, 1, 3, 1, 1);

    label_password = gtk_label_new("Password:");
    gtk_grid_attach(GTK_GRID(grid), label_password, 0, 4, 1, 1);
    entry_password = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry_password), FALSE); // Hide password
    gtk_grid_attach(GTK_GRID(grid), entry_password, 1, 4, 1, 1);

    label_topic = gtk_label_new("Topic:");
    gtk_grid_attach(GTK_GRID(grid), label_topic, 0, 5, 1, 1);
    entry_topic = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_topic, 1, 5, 1, 1);

    button_subscribe = gtk_button_new_with_label("Subscribe");
    gtk_grid_attach(GTK_GRID(grid), button_subscribe, 0, 6, 2, 1);
    g_signal_connect(button_subscribe, "clicked", G_CALLBACK(on_subscribe_button_clicked), NULL);

    // Create a text view to display received messages
    textview_messages = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_messages), FALSE); // Make it read-only

    scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll_window, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll_window), textview_messages);
    gtk_grid_attach(GTK_GRID(grid), scroll_window, 0, 7, 2, 1);

    gtk_widget_show_all(window);
}

int main (int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.example.mqttsubscriber", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    if (mosq) {
        mosquitto_disconnect(mosq);
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
    }

    return status;
}
