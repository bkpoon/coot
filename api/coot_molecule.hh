#ifndef COOT_MOLECULE_HH
#define COOT_MOLECULE_HH

#include <utility>

#include <clipper/core/xmap.h>
#include "coot-utils/atom-selection-container.hh"
#include "geometry/residue-and-atom-specs.hh"

namespace coot {

   class molecule_t {

   class molecule_save_info_t {
   public:
      std::pair<time_t, unsigned int> last_saved;
      unsigned int modification_index;
      molecule_save_info_t() : last_saved(std::make_pair(0,0)), modification_index(0) {}
      void new_modification() {
         modification_index++;
      }
      void made_a_save() {
         // this is called when the server says that it has saved the file
         last_saved.first = time(nullptr);
         last_saved.second = modification_index;
      }
      bool have_unsaved_changes() const {
         return modification_index > last_saved.second;
      }
   };

   molecule_save_info_t save_info;

   public:
      atom_selection_container_t atom_sel;
      molecule_t() {}
      explicit molecule_t(atom_selection_container_t asc) : atom_sel(asc) {}
      clipper::Xmap<float> xmap; // public because the filling function needs access

      // utils

      bool is_valid_model_molecule();
      bool is_valid_map_molecule();
      std::pair<bool, coot::residue_spec_t> cid_to_residue_spec(const std::string &cid);

      // functions

      int flipPeptide(const coot::residue_spec_t &rs, const std::string &alt_conf);
      bool have_unsaved_changes() const { return save_info.have_unsaved_changes(); }

   };
}


#endif // COOT_MOLECULE_HH
