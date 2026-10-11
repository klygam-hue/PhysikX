// Copyright (c) 2026 Mistrul.
// SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
// See the accompanying LICENSE for permitted use.

#include <PhysikX/physikx.h>
#include <stdio.h>
int main(void) {
  px_world_desc d = px_world_defaults(2);
  px_world *w = px_world_create(&d);
  if (!w)
    return 1;
  px_body_desc b = px_body_defaults();
  b.position.y = 5;
  px_id id;
  px_body_state s;
  int ok = px_body_create(w, &b, &id) == PX_OK &&
           px_world_step(w, 1. / 120) == PX_OK &&
           px_body_get_state(w, id, &s) == PX_OK && s.position.y < 5;
  px_mass_properties props;
  px_material material;
  px_break_event event, copied;
  ok = ok && px_body_set_mass(w,id,2)==PX_OK &&
       px_body_get_mass_properties(w,id,&props)==PX_OK && props.mass==2 && props.weight>19 &&
       px_body_set_density(w,id,2)==PX_OK && px_body_set_friction(w,id,0.8)==PX_OK &&
       px_body_get_material(w,id,&material)==PX_OK && material.friction==0.8 &&
       px_body_apply_thrust(w,id,(px_vec3){1,0,0},2)==PX_OK &&
       px_body_set_strength(w,id,0)==PX_OK && px_body_fracture(w,id,&event)==PX_OK &&
       event.count==4 && px_world_break_events(w,&copied,1)==1 && copied.source==id;
  px_world_destroy(w);
  puts(ok ? "C ABI passed" : "C ABI failed");
  return ok ? 0 : 1;
}
