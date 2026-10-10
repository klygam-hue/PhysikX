# Copyright (c) 2026 Mistrul.
# SPDX-License-Identifier: LicenseRef-PhysikX-Proprietary-1.0
# See the accompanying LICENSE for permitted use.

"""Renderer adapter example: click to spawn circles, right-click to kick nearby bodies."""

import time
import tkinter as tk
import PhysikX as P

world = P.World(2, max_substeps=32)
world.create_body(
    shape=P.BOX, is_static=1, position=(0, -0.5, 0), half_extents=(20, 0.5, 1)
)
world.create_body(
    shape=P.BOX, is_static=1, position=(-8, 4, 0), half_extents=(0.3, 4, 1)
)
world.create_body(
    shape=P.BOX, is_static=1, position=(8, 4, 0), half_extents=(0.3, 4, 1)
)
root = tk.Tk()
root.title("PhysikX — 2D playground")
canvas = tk.Canvas(root, width=1000, height=700, bg="#111827", highlightthickness=0)
canvas.pack()
canvas.create_text(
    20,
    20,
    anchor="nw",
    fill="#e5e7eb",
    text="Left click: spawn | Right click: impulse | Escape: close",
)
bodies = {}


def screen(p):
    return (500 + p.x * 50, 650 - p.y * 50)


def spawn(event):
    b = world.create_body(
        position=((event.x - 500) / 50, (650 - event.y) / 50, 0),
        radius=0.3,
        restitution=0.5,
    )
    bodies[b] = canvas.create_oval(0, 0, 0, 0, fill="#34d399", outline="#a7f3d0")


def kick(event):
    p = ((event.x - 500) / 50, (650 - event.y) / 50, 0)
    for b in world.query_aabb((p[0] - 1, p[1] - 1, 0), (p[0] + 1, p[1] + 1, 0)):
        if b in bodies:
            world.apply_impulse(b, (0, 5, 0))


canvas.bind("<Button-1>", spawn)
canvas.bind("<Button-3>", kick)
canvas.create_rectangle(85, 650, 915, 680, fill="#374151", outline="")
canvas.create_rectangle(85, 250, 115, 650, fill="#374151", outline="")
canvas.create_rectangle(885, 250, 915, 650, fill="#374151", outline="")
previous = time.perf_counter()


def frame():
    global previous
    now = time.perf_counter()
    world.step(min(now - previous, 0.25))
    previous = now
    for b, item in list(bodies.items()):
        state = world.state(b)
        x, y = screen(state.position)
        if state.position.y < -5:
            world.destroy_body(b)
            canvas.delete(item)
            del bodies[b]
        else:
            canvas.coords(item, x - 15, y - 15, x + 15, y + 15)
    root.after(16, frame)


def close():
    world.close()
    root.destroy()


root.protocol("WM_DELETE_WINDOW", close)
root.bind("<Escape>", lambda _: close())
frame()
root.mainloop()
