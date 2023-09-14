/* layla/ligand_editor_canvas.hpp
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

#ifndef COOT_LIGAND_EDITOR_CANVAS_HPP
#define COOT_LIGAND_EDITOR_CANVAS_HPP

#include "ligand_editor_canvas/core.hpp"
#include "ligand_editor_canvas/model.hpp"
#include "ligand_editor_canvas/tools.hpp"

/// GObject declaration had to be moved to "core.hpp"

extern "C" {

CootLigandEditorCanvas *coot_ligand_editor_canvas_new();


} // extern "C"


void coot_ligand_editor_set_active_tool(CootLigandEditorCanvas* self, std::unique_ptr<coot::ligand_editor_canvas::ActiveTool>&& active_tool);
void coot_ligand_editor_append_molecule(CootLigandEditorCanvas* self, std::shared_ptr<RDKit::RWMol> rdkit_mol) noexcept;
void coot_ligand_editor_set_scale(CootLigandEditorCanvas* self, float scale) noexcept;
float coot_ligand_editor_get_scale(CootLigandEditorCanvas* self) noexcept;

void coot_ligand_editor_undo_edition(CootLigandEditorCanvas* self) noexcept;
void coot_ligand_editor_redo_edition(CootLigandEditorCanvas* self) noexcept;

/// Returns the pointer to the molecule with the given index or nullptr when index is out of range.
///
/// Canvas owns the returned pointer
const RDKit::ROMol* coot_ligand_editor_get_rdkit_molecule(CootLigandEditorCanvas* self, unsigned int index) noexcept;
unsigned int coot_ligand_editor_get_molecule_count(CootLigandEditorCanvas* self) noexcept;

void coot_ligand_editor_set_allow_invalid_molecules(CootLigandEditorCanvas* self, bool value) noexcept;
bool coot_ligand_editor_get_allow_invalid_molecules(CootLigandEditorCanvas* self) noexcept;

coot::ligand_editor_canvas::DisplayMode coot_ligand_editor_get_display_mode(CootLigandEditorCanvas* self) noexcept;
void coot_ligand_editor_set_display_mode(CootLigandEditorCanvas* self, coot::ligand_editor_canvas::DisplayMode value) noexcept;

std::string coot_ligand_editor_get_smiles(CootLigandEditorCanvas* self) noexcept;
std::string coot_ligand_editor_get_smiles_for_molecule(CootLigandEditorCanvas* self, unsigned int molecule_idx) noexcept;

/// Takes ownership of the pointer
void coot_ligand_editor_draw_on_cairo_surface(CootLigandEditorCanvas* self, cairo_t* cr) noexcept;

void coot_ligand_editor_clear_molecules(CootLigandEditorCanvas* self) noexcept;
#endif // COOT_LIGAND_EDITOR_CANVAS_HPP
