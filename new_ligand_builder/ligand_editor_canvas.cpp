#include "ligand_editor_canvas.hpp"
#include "ligand_editor_canvas/core.hpp"
#include "ligand_editor_canvas/model.hpp"
#include <exception>
#include <utility>
#include <algorithm>
#include <vector>
#include <memory>

using namespace coot::ligand_editor_canvas;

/// Because of GObject's amazing macro system, 
/// I can't use a "typedef" to denote that
/// "_CootLigandEditorCanvas" is the same type as "coot::ligand_editor_canvas::impl::CootLigandEditorCanvasPriv".
///
/// A "using" statement doesn't compile too. I don't think there's anything I might be doing wrong.
///
/// An empty struct with inheritance should compile to the exactly same thing though.
struct _CootLigandEditorCanvas:  coot::ligand_editor_canvas::impl::CootLigandEditorCanvasPriv {

};

G_BEGIN_DECLS

G_DEFINE_TYPE(CootLigandEditorCanvas, coot_ligand_editor_canvas, GTK_TYPE_WIDGET)

// not sure what this is for or whether it is going to be needed at all
// struct _CootLigandEditorCanvasClass {
//     GObjectClass parent_class;
// };

void coot_ligand_editor_canvas_snapshot (GtkWidget *widget, GtkSnapshot *snapshot)
{
    CootLigandEditorCanvas* self = COOT_COOT_LIGAND_EDITOR_CANVAS(widget);

    float w = (float) gtk_widget_get_size(widget,GTK_ORIENTATION_HORIZONTAL);
    float h = (float) gtk_widget_get_size(widget,GTK_ORIENTATION_VERTICAL);
    const graphene_rect_t background_rect = graphene_rect_t{{0,0},{w,h}};
    const GdkRGBA background_color = GdkRGBA{1.f,1.f,1.f,1.f};
    gtk_snapshot_append_color(snapshot, &background_color, &background_rect);
    if (self->molecules) {
        if(self->molecules->empty()) {
            g_info("No molecules to be drawn.");
        } else {
            // This does not respect GTK theming
            // PangoLayout* pango_layout = pango_cairo_create_layout(cairo_canvas);
            PangoLayout* pango_layout = pango_layout_new(gtk_widget_get_pango_context(widget));
            for(auto& drawn_molecule: *self->molecules) {
                drawn_molecule.set_offset_from_bounds(&background_rect);
                drawn_molecule.draw(snapshot,pango_layout,&background_rect);
            }
            g_object_unref(pango_layout);
        }
    } else {
        g_error("Molecules vector not initialized!");
    }
   
}

void coot_ligand_editor_canvas_measure
    (GtkWidget      *widget,
    GtkOrientation  orientation,
    int             for_size,
    int            *minimum_size,
    int            *natural_size,
    int            *minimum_baseline,
    int            *natural_baseline)
{
    CootLigandEditorCanvas* self = COOT_COOT_LIGAND_EDITOR_CANVAS(widget);

    switch (orientation)
    {
    case GTK_ORIENTATION_HORIZONTAL:{
         // For now:
        *natural_size = 1200;
        *minimum_size = 1200;
        break;
    }
    case GTK_ORIENTATION_VERTICAL:{
         // For now:
        *natural_size = 500;
        *minimum_size = 500;
        break;
    }
    default:
        break;
    }

   

}

static void on_hover (
  GtkEventControllerMotion* controller,
  gdouble x,
  gdouble y,
  gpointer user_data
) {
    CootLigandEditorCanvas* self = COOT_COOT_LIGAND_EDITOR_CANVAS(user_data);
    for(auto& molecule: *self->molecules) {
        molecule.clear_highlights();
    }
    try {
        auto [bond_or_atom,molecule_idx] = self->resolve_click(x, y).value();
        auto& target = (*self->molecules)[molecule_idx];
        if(std::holds_alternative<CanvasMolecule::Atom>(bond_or_atom)) {
            auto atom = std::get<CanvasMolecule::Atom>(std::move(bond_or_atom));
            target.highlight_atom(atom.idx);
        } else { // a bond
            auto bond = std::get<CanvasMolecule::Bond>(std::move(bond_or_atom));
            target.highlight_bond(bond.first_atom_idx, bond.second_atom_idx);
        }

    } catch (std::exception& e) {
        // Nothing was hovered on
    }
    gtk_widget_queue_draw(GTK_WIDGET(self));
}

static void on_left_click (
  GtkGestureClick* gesture_click,
  gint n_press,
  gdouble x,
  gdouble y,
  gpointer user_data
) {
    CootLigandEditorCanvas* self = COOT_COOT_LIGAND_EDITOR_CANVAS(user_data);
    switch(self->active_tool->get_variant()) {

        case ActiveTool::Variant::None:{
            gtk_gesture_set_state(GTK_GESTURE(gesture_click),GTK_EVENT_SEQUENCE_NONE);
            break;
        }
        case ActiveTool::Variant::BondModifier:{
            
            break;
        }
        case ActiveTool::Variant::StructureInsertion:{
            
            break;
        }
        case ActiveTool::Variant::ElementInsertion:{
            self->active_tool->insert_atom((int)x, (int) y);
            gtk_widget_queue_draw(GTK_WIDGET(self));
            break;
        }
        case ActiveTool::Variant::GeometryModifier:{
            
            break;
        }
        case ActiveTool::Variant::DeleteHydrogens:{
            
            break;
        }
        case ActiveTool::Variant::Delete:{
            
            break;
        }
        case ActiveTool::Variant::Format:{
            
            break;
        }
        case ActiveTool::Variant::ChargeModifier:{
            
            break;
        }
        default:{
            gtk_gesture_set_state(GTK_GESTURE(gesture_click),GTK_EVENT_SEQUENCE_NONE);
            break;
        };
    }
    //gtk_gesture_set_state(GTK_GESTURE(gesture_click),GTK_EVENT_SEQUENCE_CLAIMED);
    
    //
    
}

static void coot_ligand_editor_canvas_init(CootLigandEditorCanvas* self) {
    // This is the primary constructor
    self->active_tool = std::make_unique<ActiveTool>();
    self->active_tool->set_core_widget_data(static_cast<impl::CootLigandEditorCanvasPriv*>(self));
    self->molecules = std::make_unique<std::vector<CanvasMolecule>>();
    self->rdkit_molecules = std::make_unique<std::vector<std::shared_ptr<RDKit::RWMol>>>();

    GtkGesture* click_controller = gtk_gesture_click_new();
    GtkEventController* hover_controller = gtk_event_controller_motion_new();

    // left mouse button
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click_controller),GDK_BUTTON_PRIMARY);
    g_signal_connect(click_controller,"pressed",G_CALLBACK(on_left_click),self);

    g_signal_connect(hover_controller,"motion",G_CALLBACK(on_hover),self);

    gtk_widget_add_controller(GTK_WIDGET(self),GTK_EVENT_CONTROLLER(click_controller));
    gtk_widget_add_controller(GTK_WIDGET(self),GTK_EVENT_CONTROLLER(hover_controller));
}

static void coot_ligand_editor_canvas_dispose(GObject* _self) {
    CootLigandEditorCanvas* self = COOT_COOT_LIGAND_EDITOR_CANVAS(_self);
    self->molecules.reset(nullptr);
    self->active_tool.reset(nullptr);
    self->rdkit_molecules.reset(nullptr);
    G_OBJECT_CLASS(coot_ligand_editor_canvas_parent_class)->dispose(_self);
}

static void coot_ligand_editor_canvas_class_init(CootLigandEditorCanvasClass* klass) {
    // I think that this is a GObject class constructor that sets up the GObject class at runtime.
    GTK_WIDGET_CLASS(klass)->snapshot = coot_ligand_editor_canvas_snapshot;
    GTK_WIDGET_CLASS(klass)->measure = coot_ligand_editor_canvas_measure;
    G_OBJECT_CLASS(klass)->dispose = coot_ligand_editor_canvas_dispose;
    
}

CootLigandEditorCanvas* 
coot_ligand_editor_canvas_new()
{
    return COOT_COOT_LIGAND_EDITOR_CANVAS(g_object_new (COOT_LIGAND_EDITOR_CANVAS_TYPE, NULL));
}

G_END_DECLS

void coot_ligand_editor_set_active_tool(CootLigandEditorCanvas* self, std::unique_ptr<ActiveTool>&& active_tool) {
    self->active_tool = std::move(active_tool);
    self->active_tool->set_core_widget_data(static_cast<impl::CootLigandEditorCanvasPriv*>(self));
}

void coot_ligand_editor_append_molecule(CootLigandEditorCanvas* self, std::shared_ptr<RDKit::RWMol> rdkit_mol) noexcept {
    try {
        g_debug("Appending new molecule to the widget...");
        // Might throw if the constructor fails.
        self->molecules->push_back(CanvasMolecule(rdkit_mol));
        self->rdkit_molecules->push_back(std::move(rdkit_mol));
        // g_debug("Should draw.");
        gtk_widget_queue_draw(GTK_WIDGET(self));
    }catch(std::exception& e) {
        g_warning("coot_ligand_editor_append_molecule: 2D representation could not be created: %s. Ignoring new molecule.",e.what());
    }
}