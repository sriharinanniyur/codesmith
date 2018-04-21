/*
 * Main GTK application for CodeSmith
*/

#include "parser.h"
#include <gtk/gtk.h>
#include <gtksourceview/gtksource.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdio>
#include <cstdlib>
#include <cstring>

std::string current_file = "";

GtkCssProvider *provider;
GtkStyleContext *context;

GtkWidget *win;
GtkWidget *view;
GtkWidget *scrollwin;

GtkWidget *output_view;
GtkWidget *output_scrollwin;
GtkTextBuffer *output_buffer;

GtkSourceBuffer *buffer;
GtkWidget *box;
GtkWidget *save_btn;
GtkWidget *open_btn;
GtkWidget *check_btn;
GtkWidget *btn_box;
GtkWidget *entry;

GtkTextTag *err_tag;
GtkTextTag *style_tag;

void open(GtkWidget *widget, gpointer data)
{
    current_file = gtk_entry_get_text(GTK_ENTRY(entry));
    gtk_source_buffer_set_language(buffer, gtk_source_language_manager_guess_language(
        gtk_source_language_manager_get_default(), current_file.c_str(), NULL));
    std::string line;
    std::string total = "";
    std::ifstream in(current_file);
    while (std::getline(in, line))
        total += line + "\n";
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), total.c_str(), -1);
    in.close();
    gtk_window_set_title(GTK_WINDOW(win), (std::string(current_file) + " - CodeSmith").c_str());
}

void save(GtkWidget *widget, gpointer data)
{
    if (! current_file.length())
        current_file = gtk_entry_get_text(GTK_ENTRY(entry));
    gtk_source_buffer_set_language(buffer, gtk_source_language_manager_guess_language(
        gtk_source_language_manager_get_default(), current_file.c_str(), NULL));
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER(buffer), &start, &end);
    std::string text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);
    std::ofstream out(current_file);
    out << text;
    out.close();
    gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(buffer), err_tag, &start, &end);
    gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(buffer), style_tag, &start, &end);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(output_buffer), "", -1);
}

void check(GtkWidget *widget, gpointer data)
{
    if (! current_file.length())
        return;
    GtkTextIter start, end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(buffer), &start, &end);
    std::string total_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(buffer), &start, &end, FALSE);

    std::string results_display = "----STYLE ISSUES----\n";
    Parser p(current_file);
    std::map<int, std::string> errs = p.get_errors();
    std::map<int, std::string> style_errs = p.get_style_errors();
    for (auto it = style_errs.cbegin(); it != style_errs.cend(); it++) {
        int sp, ep;
        int num_lines = 0;
        for (sp = 0; sp < total_text.length(); sp++) {
            if (num_lines == it->first - 1) break;
            if (total_text.at(sp) == '\n') {
                num_lines += 1;
            }
        }
        for (ep = sp; ep < total_text.length(); ep++) {
            if (total_text.at(ep) == '\n') break;
        }
        results_display += "Line " + std::to_string(it->first) + " --- Character" + it->second + "\n";
        gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, sp);
        gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, ep);
        gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), style_tag, &start, &end);
    }
    results_display += "\n----ERRORS----\n";
    for (auto it = errs.cbegin(); it != errs.cend(); it++) {
        int sp, ep;
        int num_lines = 0;
        for (sp = 0; sp < total_text.length(); sp++) {
            if (num_lines == it->first - 1) break;
            if (total_text.at(sp) == '\n') {
                num_lines += 1;
            }
        }
        for (ep = sp; ep < total_text.length(); ep++) {
            if (total_text.at(ep) == '\n') break;
        }
        results_display += "Line " + std::to_string(it->first) + " --- Character" + it->second + "\n";
        gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &start, sp);
        gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(buffer), &end, ep);
        gtk_text_buffer_remove_tag(GTK_TEXT_BUFFER(buffer), style_tag, &start, &end);
        gtk_text_buffer_apply_tag(GTK_TEXT_BUFFER(buffer), err_tag, &start, &end);
    }

    gtk_text_buffer_set_text(output_buffer, results_display.c_str(), -1);
}


int main (int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(win), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(win), 800, 600);
    gtk_window_set_title(GTK_WINDOW(win), "CodeSmith");
    
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    btn_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);

    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Filename...");

    save_btn = gtk_button_new_with_label("Save");
    open_btn = gtk_button_new_with_label("Open");
    check_btn = gtk_button_new_with_label("Check");

    gtk_box_pack_start(GTK_BOX(btn_box), save_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), open_btn, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(btn_box), check_btn, TRUE, TRUE, 0);

    buffer = gtk_source_buffer_new(NULL);

    /* Add the text view */
    view = gtk_source_view_new_with_buffer(buffer);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(view), TRUE);
    gtk_source_view_set_auto_indent(GTK_SOURCE_VIEW(view), TRUE);
    gtk_source_view_set_tab_width(GTK_SOURCE_VIEW(view), 4);
    gtk_source_view_set_insert_spaces_instead_of_tabs(GTK_SOURCE_VIEW(view), TRUE);
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "textview { font: 14 monospace; color: blue; }", -1, NULL);
    context = gtk_widget_get_style_context(view);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    output_buffer = gtk_text_buffer_new(NULL);
    output_view = gtk_text_view_new_with_buffer(output_buffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_view), FALSE);
    output_scrollwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(output_scrollwin, 400, 200);


    scrollwin = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrollwin), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scrollwin, 400, 400);

    gtk_container_add(GTK_CONTAINER(scrollwin), view);
    gtk_container_add(GTK_CONTAINER(output_scrollwin), output_view);
    gtk_container_set_border_width(GTK_CONTAINER(scrollwin), 5);
    gtk_box_pack_start(GTK_BOX(box), btn_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), scrollwin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), output_scrollwin, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(win), box);
    gtk_widget_show_all(win);

    err_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "red-bg", "background", "red", NULL);
    style_tag = gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(buffer), "blue-bg", "background", "blue", NULL);

    g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(save_btn, "clicked", G_CALLBACK(save), NULL);
    g_signal_connect(open_btn, "clicked", G_CALLBACK(open), NULL);
    g_signal_connect(check_btn, "clicked", G_CALLBACK(check), NULL);

    gtk_main();

    return 0;
}
