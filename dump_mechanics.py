import dnfile
from dncil.cil.body.reader import read_method_body_from_bytes
import sys

out_file = open("mechanics_analysis.txt", "w", encoding="utf-8")
sys.stdout = out_file

dn = dnfile.dnPE(r'Fart Together_Data\Managed\Assembly-CSharp.dll')

classes = [
    'BoatPhysicsConfig', 'PirateGasConfig', 'PirateButtBlendJiggleConfig',
    'BumperObstacle', 'BoatDefinition', 'Crocodile', 'Shark',
    'PirateGas', 'PirateButtBlendJiggle', 'BoatPhysicsController'
]

def dump_class(r):
    print(f"\n==================== {r.TypeName} ({r.TypeNamespace}) ====================")
    print("--- Fields ---")
    for f in r.FieldList:
        print(f"  {f.row.Name} (flags: {f.row.Flags})")
    
    print("--- Methods ---")
    for m in r.MethodList:
        mname = m.row.Name
        rva = m.row.Rva
        if rva == 0:
            continue
        try:
            data = dn.get_data(rva)
            if not data:
                continue
            body = read_method_body_from_bytes(data)
            insts = []
            for ins in body.instructions:
                op = str(ins.opcode)
                opnd = str(ins.operand) if ins.operand is not None else ""
                insts.append(f"{op} {opnd}".strip())
            
            # If ctor or small method, print full body
            if mname in ('.ctor', '.cctor') or len(insts) <= 20:
                print(f"  [Method] {mname}:")
                for s in insts:
                    print(f"      {s}")
            else:
                print(f"  [Method] {mname} ({len(insts)} insts)")
        except Exception as e:
            print(f"  [Method] {mname} (error: {e})")

for r in dn.net.mdtables.TypeDef:
    if r.TypeName in classes:
        dump_class(r)
