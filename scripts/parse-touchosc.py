#!/usr/bin/env python3

from xml.etree import ElementTree

import sys
import base64

def main():
    if len(sys.argv) != 2:
        sys.exit("Usage: " + sys.argv[0] + " <path to decompressed touchosc layout>")

    path = sys.argv[1]
    root = ElementTree.parse(path).getroot()

    for tab_el in root.findall(".//tabpage"):
        tab_name = decode(tab_el.attrib["name"])

        for ctrl_el in tab_el.findall(".//control"):
            attribs = ctrl_el.attrib

            if "osc_cs" in attribs:
                method = decode(attribs["osc_cs"])
            else:
                name = decode(attribs["name"])
                method = "/{}/{}".format(tab_name, name)

            if "number" in attribs:
                num = int(attribs["number"])
                for x in range(num):
                    print("{}/{}".format(method, x + 1))
            elif "number_x" in attribs:
                num_x = int(attribs["number_x"])
                num_y = int(attribs["number_y"])
                for x in range(num_x):
                    for y in range(num_y):
                        print("{}/{}/{}".format(method, x + 1, y + 1))
            else:
                print(method)

def decode(s):
    return base64.b64decode(s).decode('utf-8')

if __name__ == "__main__":
    main()
