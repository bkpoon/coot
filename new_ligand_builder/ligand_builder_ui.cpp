#include "ligand_builder_ui.hpp"
#include "ligand_builder_state.hpp"
#include "ligand_editor_canvas.hpp"
#include "ligand_editor_canvas/core.hpp"
#include "ligand_editor_canvas/model.hpp"
#include "ligand_editor_canvas/tools.hpp"

void setup_actions(coot::ligand_editor::LigandBuilderState* state, GtkApplicationWindow* win, GtkBuilder* builder) {
    using namespace coot::ligand_editor;

    auto new_action = [win](const char* action_name, GCallback func, gpointer userdata = nullptr){
        std::string detailed_action_name = "win.";
        detailed_action_name += action_name;
        GSimpleAction* action = g_simple_action_new(action_name,nullptr);
        g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(action));
        g_signal_connect(action, "activate", func, userdata);
        //return std::make_pair(detailed_action_name,action);
    };

    auto new_stateful_action = [win](const char* action_name,const GVariantType *state_type, GVariant* default_state, GCallback func, gpointer userdata = nullptr){
        std::string detailed_action_name = "win.";
        detailed_action_name += action_name;
        GSimpleAction* action = g_simple_action_new_stateful(action_name, state_type, default_state);
        g_action_map_add_action(G_ACTION_MAP(win), G_ACTION(action));
        g_signal_connect(action, "activate", func, userdata);
        //return std::make_pair(detailed_action_name,action);
    };

    using ExportMode = coot::ligand_editor::LigandBuilderState::ExportMode;

    // File
    new_action("file_new", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_new();
    }),state);
    new_action("file_open", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_open();
    }),state);
    new_action("import_from_smiles", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->load_from_smiles();
    }),state);
    new_action("import_molecule", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_import_molecule();
    }),state);
    new_action("fetch_molecule", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_fetch_molecule();
    }),state);
    new_action("file_save", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_save();
    }),state);
    new_action("file_save_as", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_save_as();
    }),state);
    new_action("export_pdf", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_export(ExportMode::PDF);
    }),state);
    new_action("export_png", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_export(ExportMode::PNG);
    }),state);
    new_action("export_svg", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_export(ExportMode::SVG);
    }),state);
    new_action("file_exit", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->file_exit();
    }),state);

    // Edit;
    new_action("undo", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->edit_undo();
    }),state);
    new_action("redo", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LigandBuilderState*)user_data)->edit_redo();
    }),state);
    // Display

    using coot::ligand_editor_canvas::DisplayMode;
    GVariant* display_mode_action_defstate = g_variant_new("s",coot::ligand_editor_canvas::display_mode_to_string(DisplayMode::Standard));
    new_stateful_action(
        "switch_display_mode", 
        G_VARIANT_TYPE_STRING,
        display_mode_action_defstate, 
        G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
            const gchar* mode_name = g_variant_get_string(parameter,nullptr);
            auto mode = coot::ligand_editor_canvas::display_mode_from_string(mode_name);
            if(mode.has_value()) {
                ((LigandBuilderState*)user_data)->switch_display_mode(mode.value());
                g_simple_action_set_state(self, parameter);
            } else {
                g_error("Could not parse display mode from string!: '%s'",mode_name);
            }
        }
    ),state);

    // Help

    new_action("show_about_dialog", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        auto* about_dialog = GTK_WINDOW(user_data);
        gtk_window_present(GTK_WINDOW(about_dialog));
    }),gtk_builder_get_object(builder, "layla_about_dialog"));

    new_action("show_shortcuts_window", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        auto* window = GTK_WINDOW(user_data);
        gtk_window_present(GTK_WINDOW(window));
    }),gtk_builder_get_object(builder, "layla_shortcuts_window"));

}

GtkApplicationWindow* coot::ligand_editor::setup_main_window(GtkApplication* app, GtkBuilder* builder) {
    GtkApplicationWindow* win = (GtkApplicationWindow*) gtk_builder_get_object(builder, "layla_window");
    gtk_window_set_application(GTK_WINDOW(win),app);
    GtkWidget* status_label = (GtkWidget*) gtk_builder_get_object(builder, "layla_status_label");
    GtkScrolledWindow* viewport = (GtkScrolledWindow*) gtk_builder_get_object(builder, "layla_canvas_viewport");
    auto* canvas = coot_ligand_editor_canvas_new();

    g_signal_connect(canvas, "status-updated", G_CALLBACK(+[](CootLigandEditorCanvas* canvas, const gchar* status_text, gpointer user_data){
        gtk_label_set_text(GTK_LABEL(user_data), status_text);
    }), status_label);

    GtkSpinButton* scale_spinbutton = (GtkSpinButton*) gtk_builder_get_object(builder, "layla_scale_spinbutton");
    g_signal_connect(canvas, "scale-changed", G_CALLBACK(+[](CootLigandEditorCanvas* canvas, float new_scale, gpointer user_data){
        GtkSpinButton* spinbutton = GTK_SPIN_BUTTON(user_data);
        gtk_spin_button_set_value(spinbutton, new_scale);
    }), scale_spinbutton);


    GtkTextView* smiles_display = (GtkTextView*) gtk_builder_get_object(builder, "layla_smiles_textview");
    g_signal_connect(canvas, "smiles-changed", G_CALLBACK(+[](CootLigandEditorCanvas* self, gpointer user_data){
        GtkTextView* view = GTK_TEXT_VIEW(user_data);
        std::string smiles = coot_ligand_editor_get_smiles(self);
        GtkTextBuffer* buf = gtk_text_view_get_buffer(view);
        gtk_text_buffer_set_text(buf,smiles.c_str(),-1);
    }), smiles_display);

    gtk_scrolled_window_set_child(viewport, GTK_WIDGET(canvas));
    coot::ligand_editor::initialize_global_instance(canvas,GTK_WINDOW(win),GTK_LABEL(status_label));
    setup_actions(coot::ligand_editor::global_instance, win, builder);
    return win;
}