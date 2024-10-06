#!/usr/bin/env python3

from AppJsonEncoder import Serializer
from ObjectGenerator import parse_prop_type, parse_func_define, TemplateInfo, TemplateGenerator
from pathlib import Path


draw_template = """\
${return_type} (*${func_name}) (${args});\
"""

def gen_interface_for_func(data):
    funcs = { 
             "set_source_rgba",
             "rectangle",
             "paint",
             "create"
             }

    for line in data.split("\n"):
        line = line\
                .replace("cairo_t ", "FrDrawContext")\
                .replace("cairo_surface_t ", "FrDrawSurface")\
                .replace("double ", "SysDouble ")\
                .replace("int ", "SysInt ")

        match_type = parse_prop_type(line)
        if not match_type:
            continue

        prop = parse_func_define(match_type, line)
        if not prop:
            continue

        # match = False
        # for v in funcs:
        #     if prop.name.endswith(v):
        #         match = True

        # if not match:
        #     continue

        r = draw_template\
                .replace("${func_name}", prop.name.replace("cairo_", "")) \
                .replace("${return_type}", prop.type) \
                .replace("${args}", prop.args) \

        print(r)

def gen_interface_for_cairo():
    # ctags extract command
    # ctags.exe -x --c-kinds=pf --fields=+Sne --_xformat="%{typeref} %{name}%{signature};" ./cairo.h > def.txt
    f = open("def.txt", "r+")

    data = f.read()
    gen_interface_for_func(data)

    f.close()

def gen_struct_result():
    dst = Path("./Framework/Media").as_posix()
    header_path = "Framework/Media"
    common_path = "Framework//FrCommon.h"

    template_struct = """
struct _FrAvPlayer {
  FrPlayer parent;

  /* <private> */
  FrMediaFile *file;
  FrWindow *window;
  FrMediaPipeline pipeline;

  SysBool use_hwaccel;
  SysInt64 seek_position;
  FR_JOB_STATE_ENUM state;
  FrMediaRender *render;
  SysInt64 base_tsp;
  SysInt64 vtsp;
  SysDouble default_delay;
  SysDouble delay;
};
"""
    info = TemplateInfo(template_struct)
    gen = TemplateGenerator(dst, header_path, common_path)
    gen.generate_file(info)
    # result = gen.gen_c_interface_result(info)
    # print(result)

def main():
    gen_struct_result()

if __name__ == '__main__':
    main()
