/* layla/ligand_builder_signals.cpp
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

#include <gtk/gtk.h>
#include "ligand_builder_state.hpp"
#include "ligand_builder_generators.hpp"
#include "ligand_editor_canvas.hpp"
#include "ligand_editor_canvas/core.hpp"

using namespace coot::ligand_editor;
using namespace coot::ligand_editor_canvas;
using BondModifierMode = coot::ligand_editor_canvas::BondModifier::BondModifierMode;
using Element = coot::ligand_editor_canvas::ElementInsertion::Element;
using Structure = coot::ligand_editor_canvas::StructureInsertion::Structure;
using TransformMode = coot::ligand_editor_canvas::TransformManager::Mode;


#if 0
// This depends on `layla_window` being passed in XML as the signal's 'object'
// and on the relevant g_object_set_data call in LigandBuilderState constructor.
// I don't know if it makes sense to use it if `coot::ligand_editor::global_instance` suffices.
#define GET_STATE() (LigandBuilderState*)g_object_get_data(G_OBJECT(user_data), "ligand_builder_instance")
#else
#define GET_STATE() coot::ligand_editor::global_instance
#endif

#define GET_CANVAS() (GET_STATE())->get_canvas()

extern "C" G_MODULE_EXPORT
void
layla_on_close(GtkButton* button, gpointer user_data) {
    LigandBuilderState* state = GET_STATE();

    state->file_exit();
}

extern "C" G_MODULE_EXPORT
void
layla_on_apply(GtkButton* button, gpointer user_data) {
    auto* dialog = gtk_builder_get_object(global_layla_gtk_builder,"layla_apply_dialog");
    gtk_window_present(GTK_WINDOW(dialog));
    auto* monomer_id_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_monomer_id_combobox");
    auto* program_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_program_combobox");
    auto* input_format_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_input_format_combobox");
    auto* molecule_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_molecule_combobox");

    auto set_default_value = [](GtkComboBox* cb){
        if(gtk_combo_box_get_active(cb) == -1) {
            gtk_combo_box_set_active(cb,0);
        }
    };
    
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(molecule_combobox));
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    for(unsigned int i = 0; i != coot_ligand_editor_get_molecule_count(canvas); i++) {
        std::string smiles = coot_ligand_editor_get_smiles_for_molecule(canvas, i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(molecule_combobox), smiles.c_str());
    }

    set_default_value(GTK_COMBO_BOX(monomer_id_combobox));
    set_default_value(GTK_COMBO_BOX(program_combobox));
    set_default_value(GTK_COMBO_BOX(input_format_combobox));
    set_default_value(GTK_COMBO_BOX(molecule_combobox));
}

extern "C" G_MODULE_EXPORT
void
layla_on_generator_monomer_id_combobox_changed(GtkComboBox* self, gpointer user_data) {
    auto* entry = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_monomer_id_entry");
    if(strcmp(gtk_combo_box_get_active_id(self),"Custom") != 0) {
        gtk_widget_set_sensitive(GTK_WIDGET(entry), false);
    } else {
        gtk_widget_set_sensitive(GTK_WIDGET(entry), true);
    }
}

extern "C" G_MODULE_EXPORT
void
on_layla_generator_progress_dialog_cancelled(GtkButton* button, gpointer user_data) {
    auto* progress_dialog = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_progress_dialog");
    if(global_generator_request_task_cancellable) {
        g_cancellable_cancel(global_generator_request_task_cancellable);
    }
}

extern "C" G_MODULE_EXPORT
void
layla_on_apply_dialog_accepted(GtkButton* button, gpointer user_data) {
    if(global_generator_request_task_cancellable != nullptr) {
        return;
    }

    GeneratorRequest request;

    auto* monomer_id_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_monomer_id_combobox");
    if(strcmp(gtk_combo_box_get_active_id(GTK_COMBO_BOX(monomer_id_combobox)),"Custom") != 0) {
        auto monomer_id = std::string(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(monomer_id_combobox)));
        request.monomer_id = std::move(monomer_id);
    } else {
        auto* monomer_id_entry = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_monomer_id_entry");
        auto* buffer = gtk_entry_get_buffer(GTK_ENTRY(monomer_id_entry));
        auto monomer_id = std::string(gtk_entry_buffer_get_text(buffer));
        request.monomer_id = std::move(monomer_id);
    }
    auto* program_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_program_combobox");
    auto program_name = std::string(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(program_combobox)));

    auto* input_format_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_input_format_combobox");
    auto input_format_name = std::string(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(input_format_combobox)));

    auto* molecule_combobox = gtk_builder_get_object(global_layla_gtk_builder,"layla_generator_molecule_combobox");
    const auto* molecule_smiles_cstr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(molecule_combobox));
    std::string molecule_smiles;
    if(molecule_smiles_cstr) {
        molecule_smiles = molecule_smiles_cstr;
    }
    request.molecule_smiles = std::move(molecule_smiles);

    if(program_name == "Grade2") {
        request.generator = GeneratorRequest::Generator::Grade2;
    } else {
        request.generator = GeneratorRequest::Generator::Acedrg;
    }

    if(input_format_name == "SMILES") {
        request.input_format = GeneratorRequest::InputFormat::SMILES;
    } else {
        request.input_format = GeneratorRequest::InputFormat::MolFile;
    }

    auto* dialog = gtk_builder_get_object(global_layla_gtk_builder, "layla_apply_dialog");
    gtk_window_close(GTK_WINDOW(dialog));
    //todo: handle global_generator_request_task_cancellable

    auto* accept_button = gtk_builder_get_object(global_layla_gtk_builder, "layla_apply_dialog_accept_button");
    gtk_widget_set_sensitive(GTK_WIDGET(accept_button), FALSE);
    auto* cancel_button = gtk_builder_get_object(global_layla_gtk_builder, "layla_generator_progress_dialog_cancel_button");
    gtk_widget_set_sensitive(GTK_WIDGET(cancel_button), TRUE);
    auto* progress_dialog = gtk_builder_get_object(global_layla_gtk_builder, "layla_generator_progress_dialog");
    gtk_window_present(GTK_WINDOW(progress_dialog));
    global_generator_request_task_cancellable = run_generator_request(request);
}

extern "C" G_MODULE_EXPORT
void
layla_on_invalid_molecule_toggled(GtkCheckButton* check_button, gpointer user_data) {
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_allow_invalid_molecules(canvas, gtk_check_button_get_active(check_button));
}

extern "C" G_MODULE_EXPORT
void
layla_on_show_alerts_toggled(GtkCheckButton* check_button, gpointer user_data) {
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    g_warning("TODO: Implement 'Show Alerts'");
}

extern "C" G_MODULE_EXPORT
void
layla_on_scale_spinbutton_value_changed(GtkSpinButton* self,gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    double new_scale = gtk_spin_button_get_value(self);
    // This should prevent infinite cascade of signals being emited
    if (coot_ligand_editor_get_scale(canvas) != new_scale) {
        coot_ligand_editor_set_scale(canvas, new_scale);
    }
}

extern "C" G_MODULE_EXPORT
void
layla_on_X_button_clicked(GtkButton* _btn, gpointer user_data){
    LigandBuilderState* state = GET_STATE();
    state->run_choose_element_dialog();
}

extern "C" G_MODULE_EXPORT
void
layla_on_C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::C)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_N_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::N)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_O_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::O)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_S_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::S)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_P_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::P)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_H_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::H)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_F_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::F)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_Cl_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::Cl)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_Br_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::Br)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_I_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
 
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ElementInsertion(Element::I)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_3C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloPropaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_4C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloButaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_5C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloPentaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_6C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloHexaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_6Arom_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::BenzeneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_7C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloHeptaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_8C_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();

    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(StructureInsertion(Structure::CycloOctaneRing)));

}

extern "C" G_MODULE_EXPORT
void
layla_on_move_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(TransformTool(TransformMode::Translation)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_rotate_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(TransformTool(TransformMode::Rotation)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_flip_x_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(FlipTool(FlipMode::Horizontal)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_flip_y_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(FlipTool(FlipMode::Vertical)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_single_bond_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(BondModifier(BondModifierMode::Single)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_double_bond_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(BondModifier(BondModifierMode::Double)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_triple_bond_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(BondModifier(BondModifierMode::Triple)));
}

extern "C" G_MODULE_EXPORT
void
layla_on_geometry_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(GeometryModifier()));
}

extern "C" G_MODULE_EXPORT
void
layla_on_charge_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(ChargeModifier()));
}

extern "C" G_MODULE_EXPORT
void
layla_on_delete_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(DeleteTool()));
}

extern "C" G_MODULE_EXPORT
void
layla_on_format_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(FormatTool()));
}

extern "C" G_MODULE_EXPORT
void
layla_on_delete_hydrogens_button_clicked(GtkButton* _btn, gpointer user_data){
    CootLigandEditorCanvas* canvas = GET_CANVAS();
    coot_ligand_editor_set_active_tool(canvas, std::make_unique<ActiveTool>(RemoveHydrogensTool()));
}