#!/usr/bin/env python3

import argparse
import os

INDENT = " " * 4

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="destination path")
    parser.add_argument("classname", help="class name")
    args = parser.parse_args()

    target_dir = args.path
    classname = args.classname

    namespace = target_dir.replace("common/", "vidrevolt/", 1).split("/")

    cpp_path = "{}/{}.cpp".format(target_dir, classname)
    h_path = "{}/{}.h".format(target_dir, classname)

    if not os.path.exists(target_dir):
        os.makedirs(target_dir)

    with open(h_path, "w") as f:
        f.write(h_str(classname, namespace))

    with open(cpp_path, "w") as f:
        f.write(cpp_str(classname, namespace))

def h_str(classname, namespaces):
    name_uc = classname.upper()
    namespace_uc = "_".join(namespaces).upper()
    macro = "{}_{}_H_".format(namespace_uc, name_uc)
    s = "#ifndef {}\n".format(macro)
    s += "#define {}\n".format(macro)
    s += "\n"
    s += namespace_start_str(namespaces)

    indent = INDENT * len(namespaces)
    s += "{}class {} {{\n".format(indent, classname)
    s += indent + "};\n"

    s += namespace_end_str(namespaces)

    s += "#endif\n"

    return s

def cpp_str(classname, namespaces):
    s = '#include "{}.h"\n'.format(classname)
    s += "\n"
    s += namespace_start_str(namespaces)
    s += namespace_end_str(namespaces)

    return s

def namespace_start_str(namespaces):
    s = ""
    level = 0
    for space in namespaces:
        s += "namespace {} {{\n".format(space)
        level += 1
        s += INDENT * level

    return s.strip() + "\n"

def namespace_end_str(namespaces):
    s = ""
    level = len(namespaces)
    for space in namespaces:
        level -= 1
        s += (INDENT * level) + "}\n"

    return s

if __name__ == "__main__":
    main()
