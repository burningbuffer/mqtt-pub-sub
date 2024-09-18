#include <gtk/gtk.h>
#include <mosquitto.h>

static GtkWidget *entry_host;
static GtkWidget *entry_port;
static GtkWidget *entry_keepalive;
static GtkWidget *entry_username;
static GtkWidget *entry_password;
static GtkWidget *entry_topic;
static GtkWidget *entry_message;
static struct mosquitto *mosq;

// Function to publish a message to the MQTT topic
static void on_publish_button_clicked(GtkButton *button, gpointer user_data) {
    const gchar *host = gtk_entry_get_text(GTK_ENTRY(entry_host));
    const gchar *port_str = gtk_entry_get_text(GTK_ENTRY(entry_port));
    const gchar *keepalive_str = gtk_entry_get_text(GTK_ENTRY(entry_keepalive));
    const gchar *username = gtk_entry_get_text(GTK_ENTRY(entry_username));
    const gchar *password = gtk_entry_get_text(GTK_ENTRY(entry_password));
    const gchar *topic = gtk_entry_get_text(GTK_ENTRY(entry_topic));
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(entry_message));

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

    // Connect to the MQTT broker
    if (mosquitto_connect(mosq, host, port, keepalive)) {
        g_print("Failed to connect to broker\n");
        return;
    }

    // Publish the message to the topic
    int ret = mosquitto_publish(mosq, NULL, topic, strlen(message), message, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        g_print("Failed to publish message: %s\n", mosquitto_strerror(ret));
    } else {
        g_print("Message published successfully on topic '%s'\n", topic);
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
    GtkWidget *label_message;
    GtkWidget *button_publish;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "MQTT Publisher");
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

    label_message = gtk_label_new("Message:");
    gtk_grid_attach(GTK_GRID(grid), label_message, 0, 6, 1, 1);
    entry_message = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry_message, 1, 6, 1, 1);

    button_publish = gtk_button_new_with_label("Publish");
    gtk_grid_attach(GTK_GRID(grid), button_publish, 0, 7, 2, 1);
    g_signal_connect(button_publish, "clicked", G_CALLBACK(on_publish_button_clicked), NULL);

    gtk_widget_show_all(window);
}

int main (int argc, char **argv)
{
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.example.mqttpublisher", G_APPLICATION_FLAGS_NONE);
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
