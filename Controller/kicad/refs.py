import pcbnew

pcb = pcbnew.GetBoard()
footprints = pcb.GetFootprints()
for f in footprints:
    r = f.Reference()
    if r.IsVisible():
        r.SetVisible(False)
    v = f.Value()
    if v.IsVisible():
        v.SetVisible(False)


