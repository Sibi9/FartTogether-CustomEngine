import pathlib

text = pathlib.Path("CustomEngine/native/GameEngine.cpp").read_text(encoding="utf-8")

def check(name, s):
    c = text.count(s)
    print(f"[{'OK' if c == 1 else 'FAIL'}] {name}: count={c}")

s_editor_type = 'AssignEntityModelAndProps(o, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);'
check("editor type switch", s_editor_type)

s_editor_new = 'AssignEntityModelAndProps(newE, &obsModelA1, &obsModelB5, &obsModelB2, &obsModelC2, &obsModelF10, &obsModelF3, &obsModelRock1, &obsModelRock2, &jungleTreeModel, &cp1Model);'
check("editor new entity", s_editor_new)

