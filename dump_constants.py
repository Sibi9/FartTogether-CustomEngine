import dnfile, struct

dn = dnfile.dnPE(r'Fart Together_Data\Managed\Assembly-CSharp.dll')

constants = {}
if dn.net.mdtables.Constant:
    for c in dn.net.mdtables.Constant:
        parent = c.Parent
        if parent and parent.table.name == 'Field':
            blob = c.Value.value_bytes
            ctype = c.Type
            val = None
            try:
                if ctype == 0x0C: # R4 (float)
                    val = struct.unpack('<f', blob)[0]
                elif ctype == 0x0D: # R8 (double)
                    val = struct.unpack('<d', blob)[0]
                elif ctype == 0x08: # I4
                    val = struct.unpack('<i', blob)[0]
                elif ctype == 0x02: # Boolean
                    val = bool(blob[0])
                elif ctype == 0x0E: # String
                    val = blob.decode('utf-16le')
                else:
                    val = list(blob)
            except Exception as e:
                val = f"err: {e}"
            constants[(str(parent.row.Name), parent.row_index)] = val

classes = [
    'PirateButtBlendJiggle', 'PirateButtBlendJiggleConfig',
    'PirateGas', 'PirateGasConfig',
    'BoatPhysicsController', 'BoatPhysicsConfig',
    'BumperObstacle', 'Crocodile', 'Shark', 'BoatDefinition'
]

with open("constants_out.txt", "w", encoding="utf-8") as out:
    for r in dn.net.mdtables.TypeDef:
        if r.TypeName in classes:
            out.write(f"\n=== {r.TypeName} ({r.TypeNamespace}) ===\n")
            for f in r.FieldList:
                fname = str(f.row.Name)
                cval = constants.get((fname, f.row_index), None)
                if cval is not None:
                    out.write(f"  const {fname} = {cval}\n")
                else:
                    out.write(f"  field {fname}\n")
print("Done writing constants_out.txt")
