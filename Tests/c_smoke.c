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
  px_world_destroy(w);
  puts(ok ? "C ABI passed" : "C ABI failed");
  return ok ? 0 : 1;
}
