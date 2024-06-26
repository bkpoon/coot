/* layla/ui.cpp
 * 
 * Copyright 2023 by Global Phasing Ltd.
 * Author: Jakub Smulski
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include "ui.hpp"
#include "state.hpp"
#include "ligand_editor_canvas.hpp"
#include "ligand_editor_canvas/core.hpp"
#include "ligand_editor_canvas/model.hpp"
#include "ligand_editor_canvas/tools.hpp"
#include "utils/coot-utils.hh"
#include <functional>

void setup_actions(coot::layla::LaylaState* state, GtkApplicationWindow* win, GtkBuilder* builder) {
    using namespace coot::layla;

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

    using ExportMode = coot::layla::LaylaState::ExportMode;

    // File
    new_action("file_new", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_new();
    }),state);
    new_action("file_open", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_open();
    }),state);
    new_action("import_from_smiles", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->load_from_smiles();
    }),state);
    new_action("import_molecule", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_import_molecule();
    }),state);
    new_action("fetch_molecule", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_fetch_molecule();
    }),state);
    new_action("file_save", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_save();
    }),state);
    new_action("file_save_as", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_save_as();
    }),state);
    new_action("export_pdf", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_export(ExportMode::PDF);
    }),state);
    new_action("export_png", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_export(ExportMode::PNG);
    }),state);
    new_action("export_svg", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_export(ExportMode::SVG);
    }),state);
    new_action("file_exit", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->file_exit();
    }),state);

    // Edit;
    new_action("undo", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->edit_undo();
    }),state);
    new_action("redo", G_CALLBACK(+[](GSimpleAction* self, GVariant* parameter, gpointer user_data){
        ((LaylaState*)user_data)->edit_redo();
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
                ((LaylaState*)user_data)->switch_display_mode(mode.value());
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

GtkApplicationWindow* coot::layla::setup_main_window(GtkApplication* app, GtkBuilder* builder) {

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
        auto smiles_map = coot_ligand_editor_canvas_get_smiles(self);
        std::string smiles;
        for(const auto& [mol_idx, smiles_code] : smiles_map) {
            smiles += std::to_string(mol_idx) + ": " + smiles_code + "\n";
        }
        GtkTextBuffer* buf = gtk_text_view_get_buffer(view);
        gtk_text_buffer_set_text(buf,smiles.c_str(),-1);
    }), smiles_display);

    GtkNotebook* qed_notebook = (GtkNotebook*) gtk_builder_get_object(builder, "layla_qed_notebook");

    auto qed_info_updated_handler = [](CootLigandEditorCanvas* self, unsigned int molecule_id, const ligand_editor_canvas::CanvasMolecule::QEDInfo* qed_info, gpointer user_data){
        GtkNotebook* qed_notebook = GTK_NOTEBOOK(user_data);
        auto find_or_create_tab_for_mol_id = [qed_notebook](unsigned int molecule_id){
            auto no_pages = gtk_notebook_get_n_pages(qed_notebook);
            auto mol_id_as_str = std::to_string(molecule_id);
            for(int i = 0; i != no_pages; i++) {
                GtkWidget* tab = gtk_notebook_get_nth_page(qed_notebook, i);
                const gchar* label = gtk_notebook_get_tab_label_text(qed_notebook, tab);
                if(g_strcmp0(label, mol_id_as_str.c_str()) == 0) {
                    return tab;
                }
            }
            // No tab found. We have to create a new one.
            GtkWidget* n_label = gtk_label_new(mol_id_as_str.c_str());
            GtkWidget* qed_grid = gtk_grid_new();
            /// Setup contents
            gtk_grid_set_column_spacing(GTK_GRID(qed_grid), 15);
            gtk_grid_set_row_spacing(GTK_GRID(qed_grid), 5);
            gtk_widget_set_margin_top(qed_grid, 6);
            gtk_widget_set_margin_bottom(qed_grid, 6);

            std::map<std::string, GtkWidget *> progress_bar_info_map;

            auto build_progressbar_info_box = [] (const std::string &label /* range or anything else ??*/) {
                GtkWidget* ret = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                GtkWidget* gtk_label = gtk_label_new(label.c_str());
                gtk_box_append(GTK_BOX(ret), gtk_label);
                GtkWidget* progress_bar = gtk_progress_bar_new();
                gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
                gtk_box_append(GTK_BOX(ret), progress_bar);
                return ret;
            };

            std::vector<std::string> label_vec = {"QED", "MW", "PSA", "cLogP", "#HBA", "#HBD", "#RotBonds", "#Arom", "#Alerts"};
            for (const auto &label : label_vec)
               progress_bar_info_map[label] = build_progressbar_info_box(label);

            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["QED"], 0, 0, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["MW"],  0, 1, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["PSA"], 1, 1, 1, 1);

            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["cLogP"], 2, 1, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["#HBA"],  3, 1, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["#HBD"],  0, 2, 1, 1);

            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["#RotBonds"], 1, 2, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["#Arom"],     2, 2, 1, 1);
            gtk_grid_attach(GTK_GRID(qed_grid), progress_bar_info_map["#Alerts"],   3, 2, 1, 1);

            gtk_notebook_append_page(qed_notebook, qed_grid, n_label);
            return qed_grid;
        };

        GtkWidget* tab = find_or_create_tab_for_mol_id(molecule_id);

        auto update_progressbar_info_box = [] (GtkWidget* info_box, double value, double progress_bar_value) {
            GtkWidget* label = gtk_widget_get_first_child(info_box);
            GtkWidget* progress_bar = gtk_widget_get_next_sibling(label);
            auto value_as_str = std::to_string(value);
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress_bar), value_as_str.c_str());
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), progress_bar_value);
        };

        // update_progressbar_info_box(progress_bar_info_map["QED"], qed_info->qed_score, qed_info->qed_score);
        // update_progressbar_info_box(progress_bar_info_map[""], qed_info->molecular_weight, qed_info->ads_mw);
    };
    g_signal_connect(canvas, "qed-info-updated", G_CALLBACK(+qed_info_updated_handler), qed_notebook);

    g_signal_connect(canvas, "molecule-deleted", G_CALLBACK(+[](CootLigandEditorCanvas* self, unsigned int deleted_mol_idx, gpointer user_data) {
        GtkNotebook* qed_notebook = GTK_NOTEBOOK(user_data);
        auto no_pages = gtk_notebook_get_n_pages(qed_notebook);
        auto mol_id_as_str = std::to_string(deleted_mol_idx);
        for(int i = 0; i != no_pages; i++) {
            GtkWidget* tab = gtk_notebook_get_nth_page(qed_notebook, i);
            const gchar* label = gtk_notebook_get_tab_label_text(qed_notebook, tab);
            if(g_strcmp0(label, mol_id_as_str.c_str()) == 0) {
                gtk_notebook_remove_page(qed_notebook, i);
            }
        }
    }), qed_notebook);

    gtk_scrolled_window_set_child(viewport, GTK_WIDGET(canvas));
    coot::layla::initialize_global_instance(canvas,GTK_WINDOW(win),GTK_LABEL(status_label));
    setup_actions(coot::layla::global_instance, win, builder);
    return win;
}

GtkBuilder* coot::layla::load_gtk_builder() {

        g_info("Loading Layla's UI...");
        
        std::string dir = coot::package_data_dir();
        // all ui files should live here:
        std::string dir_ui = coot::util::append_dir_dir(dir, "ui");
        std::string ui_file_name = "layla.ui";
        std::string ui_file_full = coot::util::append_dir_file(dir_ui, ui_file_name);
        // allow local override
        if(coot::file_exists(ui_file_name)) {
            ui_file_full = ui_file_name;
        }
        GError* error = NULL;
        GtkBuilder* builder = gtk_builder_new();
        gboolean status = gtk_builder_add_from_file(builder, ui_file_full.c_str(), &error);
        if (status == FALSE) {
            g_error("Failed to read or parse %s: %s", ui_file_full.c_str(), error->message);
        }

        return builder;
}
